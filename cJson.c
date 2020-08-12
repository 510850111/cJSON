#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include "cJson.h"


static void *(*cJSON_malloc)(size_t sz) = malloc;
static void(*cJSON_free)(void *ptr) = free;

static const char *ep;

char *cJSON_GetEndPosition(void)
{
	return ep;
}

cJSON *cJSON_New_Item()
{
	cJSON * node = (cJSON *)cJSON_malloc(sizeof(cJSON));
	if (node) { memset(node, 0, sizeof(cJSON)); }
	return node;
}

void cJSON_Delete(cJSON *c)
{
	cJSON *next;
	while (c)
	{
		next = c->next;
		if (c->child) { cJSON_Delete(c->child); }
		if (c->valueString) { cJSON_free(c->valueString); }
		if (c->string) { cJSON_free(c->string); }
		cJSON_free(c);
		c = next;
	}
}

const char *skip(const char *in)
{
	while (in && *in && ((unsigned char)*in<=32))
	{
		in++;
	}
	return in;
}

const char* parse_string(cJSON *item, const char *str)
{
	const char *ptr = str + 1;
	char *ptr2;
	char *out;

	int len = 0;
	if (*str != '\"')
	{
		ep = str;
		return NULL;
	}

	while (*ptr != '\"' && *ptr && ++len)
	{
		if (*ptr++ == '\\') { ptr++; }
	}

	out = (char *)cJSON_malloc(len + 1);
	if (!out) return NULL;

	ptr = str + 1;
	ptr2 = out;

	while (*ptr != '\"' && *ptr)
	{
		if (*ptr != '\\') { *ptr2++ = *ptr++; }
		else
		{
			ptr++;
			switch (*ptr)
			{
			case 'b':*ptr2++ = '\b'; break;
			case 'f':*ptr2++ = '\f'; break;
			case 'n':*ptr2++ = '\n'; break;
			case 'r':*ptr2++ = '\r'; break;
			case 't':*ptr2++ = '\t'; break;
			default:
				*ptr2++ = *ptr; break;
			}
			ptr++;
		}
	}
	*ptr2 = 0;

	if (*ptr == '\"') { ptr++; }
	
	item->valueString = out;
	item->type = cJSON_String;

	return ptr;
}

const char* parse_number(cJSON *item, const char *num)
{
	double n = 0;		//最终计算的数字
	int sign = 1;	//正负号
	int signSubScale = 1;
	int scale = 0;
	int subscale = 0;

	if (*num == '-')
	{
		sign = -1;
		num++;
	}

	if (*num == '0') { num++; }

	if (*num >= '0' && *num <= '9')
	{
		//1920
		do
		{
			n = (n*10.0) + (*num++ - '0');
		} while (*num >= '0' && *num <= '9');
	}

	if (*num == '.' && num[1] >= '0' && num[1] <= '9')
	{
		//12345.6789
		num++;
		do
		{
			n = (n*10.0) + (*num++ - '0');
			scale--;
		} while (*num >= '0' && *num <= '9');
	}

	if (*num == 'e' || *num == 'E')
	{

		num++;
		if (*num == '+') num++;
		else if (*num == '-')
		{
			signSubScale = -1;
			num++;
		}
		do
		{
			subscale = (subscale*10.0) + (*num++ - '0');
		} while (*num >= '0' && *num <= '9');
	}

	//number = +/- number.fraction * 10 ^(+/-)exp
	n = sign * n * pow(10.0, (scale + signSubScale*subscale));

	item->valueDouble = n;
	item->valueInt = (int)n;
	item->type = cJSON_Number;

	return num;
}

const char* parse_Array(cJSON *item, const char *value)
{
	//["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
	//   [],
	cJSON *child;
	if (*value != '[')
	{
		ep = value;
		return NULL;
	}

	item->type = cJSON_Array;
	value = skip(value + 1);
	if (*value == ']') return value + 1;

	item->child = child = cJSON_New_Item();
	if (!item->child) return NULL;

	value = skip(parse_value(child, skip(value)));
	if (!value) return NULL;

	while (*value == ',')
	{
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item())) return NULL;

		child->next = new_item;
		new_item->prev = child;
		child = new_item;

		value = skip(parse_value(child, skip(value + 1)));
		if (!value) return NULL;
	}

	if (*value == ']') return value + 1;
	ep = value;

	return NULL;
}

const char* parse_Object(cJSON *item, const char *value)
{
	cJSON *child;
	if (*value != '{')
	{
		ep = value;
		return NULL;
	}

	item->type = cJSON_Object;
	value = skip(value + 1);
	if (*value == '}') return value + 1; 

	item->child = child = cJSON_New_Item();
	if (!item->child) return NULL;

	value = skip(parse_string(child, skip(value)));
	if (!value) return NULL;

	child->string = child->valueString;
	child->valueString = NULL;
	if (*value != ':')
	{
		ep = value;
		return NULL;
	}

	value = skip(parse_value(child, skip(value + 1)));
	if (!value) return NULL;

	while (*value == ',')
	{
		cJSON *new_item;
		if (!(new_item = cJSON_New_Item())) return NULL;

		child->next = new_item;
		new_item->prev = child;
		child = new_item;

		value = skip(parse_string(child, skip(value + 1)));
		if (!value) return NULL;
		
		child->string = child->valueString;
		child->valueString = NULL;

		if (*value != ':')
		{
			ep = value;
			return NULL;
		}

		value = skip(parse_value(child, skip(value + 1)));
		if (!value) return NULL;
	}

	if (*value == '}') return value + 1;

	ep = value;
	return NULL;


}

const char *parse_value(cJSON *item, const char *value)
{
	if (!value) return NULL;
	if (!strncmp(value, "false", 5))
	{
		item->type = cJSON_False;
		return value + 5;
	}

	if (!strncmp(value, "true", 4))
	{
		item->type = cJSON_True;
		item->valueInt = 1;
		return value + 4;
	}

	if (!strncmp(value, "null", 4))
	{
		item->type = cJSON_NULL;
		return value + 4;
	}

	if (*value == '\"') { return parse_string(item, value); }
	if (*value == '-' || (*value>='0' && *value <= '9')) { return parse_number(item, value); }
	if (*value == '[') { return parse_Array(item, value); }
	if (*value == '{') { return parse_Object(item, value); }

	ep = value;
	return NULL;
}

cJSON *cJSON_Parse(const char *value)
{
	return cJSON_ParseWithOpts(value, 0, 0);
}

cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
	const char *end = NULL;
	cJSON *c = cJSON_New_Item();
	ep = NULL;
	if (!c) return NULL;

	end = parse_value(c, skip(value));
	if (!end)
	{
		cJSON_Delete(c);
		return NULL;
	}
	
	if (require_null_terminated)
	{
		end = skip(end);
		if (*end)
		{
			cJSON_Delete(c);
			ep = end;
			return NULL;
		}
	}

	if (return_parse_end)
	{
		*return_parse_end = end;
	}

	return c;

}

/*==========================================================================*/

char *cJSON_strdup(const char *str)
{
	char *copy = NULL;
	size_t len;

	len = strlen(str);
	copy =(char *) cJSON_malloc(len + 1);
	if (!copy) return NULL;
	memcpy(copy, str, len + 1);

	return copy;
}

char *print_number(cJSON *item)
{
	char *str = NULL;
	double d = item->valueDouble;
	if (d == 0)
	{
		str = (char *)cJSON_malloc(2);
		if (str) strcpy(str, "0");
	}

	//整数
	else if ((fabs((double)(item->valueInt) - d) <= DBL_EPSILON) && d <= INT_MAX && d >= INT_MIN)
	{
		str = (char *)cJSON_malloc(21);
		if (str) sprintf(str, "%d", item->valueInt);
	}
	else
	{
		str = (char *)cJSON_malloc(64);
		if (str)
		{
			//1234xxxx.0
			if ((fabs(floor(d) - d) <= DBL_EPSILON) && fabs(d) < 1.0e60) { sprintf(str, "%.0f", d); }
			else if (fabs(d) < 1.0e-6 || fabs(1.0e9)) { sprintf(str, "%e", d); }
			else { sprintf(str, "%f", d); }
		}
	}

	return str;

}

char *print_string(cJSON *item, int isName)
{
	char *str = NULL;
	if(isName){ str = item->string; }
	else{ str = item->valueString; }
	char *ptr, *ptr2, *out;
	unsigned char token;
	int flag = 0, len = 0;

	for (ptr = str; *ptr; ptr++)
	{
		flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
	}

	if (!flag)
	{
		len = ptr - str;
		out = (char *)cJSON_malloc(len + 2 + 1);
		if (!out) return NULL;

		ptr2 = out;
		*ptr2++ = '\"';
		strcpy(ptr2, str);
		//"abcde"\0
		ptr2[len] = '\"';
		ptr2[len + 1] = '\0';

		return out;
	}

	if (!str)
	{
		out = (char *)cJSON_malloc(3);
		if (!out) return NULL;
		strcpy(out, "\"\"");
		return out;
	}

	ptr = str;
	while ((token = *ptr) && ++len)
	{
		if (strchr("\"\\\b\f\n\r\t", token)) { len++; }
		else if (token < 32) { len += 5; ptr++; }
	}

	out = (char *)cJSON_malloc(len + 3);
	if (!out) return NULL;

	ptr2 = out;
	ptr = str;
	*ptr2++ = '\"';

	while (*ptr)
	{
		if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
		{
			*ptr2++ = *ptr++;
		}
		else
		{
			*ptr2++ = '\\';
			switch (token = *ptr)
			{
			case '\\':*ptr2++ = '\\'; break;
			case '\"':*ptr2++ = '\"'; break;
			case '\b':*ptr2++ = '\b'; break;
			case '\f':*ptr2++ = '\f'; break;
			case '\n':*ptr2++ = '\n'; break;
			case '\r':*ptr2++ = '\r'; break;
			case '\t':*ptr2++ = '\t'; break;
			default:
				sprintf(str, "u%04x", token);
				ptr2++;
				break;
			}
		}
	}

	*ptr2++ = '\"';
	*ptr2++ = '\0';

	return out;
}

char *print_array(cJSON *item, int depth, int fmt)
{
	char **entries; //char *entries[];
	char *out = NULL, *ptr, *ret;
	int len = 5, templen = 0, isFail = 0, i = 0; //"[]"\0

	cJSON *child = item->child;
	int numEntries = 0;

	while (child)
	{
		//广度优先
		numEntries++;
		child = child->next;
	}

	if (!numEntries)
	{
		out = (char *)cJSON_malloc(3);
		if (out) strcpy(out, "[]");
		return out;
	}

	entries = (char **)cJSON_malloc(numEntries * sizeof(char *));
	if (!entries) return NULL;
	memset(entries, 0, numEntries * sizeof(char *));

	child = item->child;
	while (child)
	{
		ret = print_value(child, depth + 1, fmt);
		entries[i++] = ret;
		if (ret) { len += strlen(ret) + 2 + (fmt ? 1 : 0); }
		else { isFail = 1; }

		child = child->next;
	}

	if (!isFail) { out = (char *)cJSON_malloc(len); }
	if (!out) { isFail = 1; }

	if (isFail)
	{
		for (int i = 0; i < numEntries; i++)
		{
			if (entries[i])
			{
				cJSON_free(entries[i]);
			}
		}
		cJSON_free(entries);

		return NULL;
	}

	*out = '[';
	ptr = out + 1;
	*ptr = '\0';
	for (i = 0; i < numEntries; i++)
	{
		templen = strlen(entries[i]);
		memcpy(ptr, entries[i], templen);
		ptr += templen;

		if (i != numEntries - 1)
		{
			*ptr++ = ',';
			if (fmt) { *ptr++ = ' '; }
			*ptr = '\0';
		}

		cJSON_free(entries[i]);
	}
	cJSON_free(entries);

	*ptr++ = ']';
	*ptr++ = '\0';

	return out;
}

char *print_object(cJSON *item, int depth, int fmt)
{
	char **entries = NULL, **names = NULL;
	char *out = NULL, *ptr, *ret, *str;
	int len = 7, i = 0, templen = 0;

	cJSON *child = item->child;
	int numEntries = 0, isFail = 0;

	while (child)
	{
		numEntries++;
		child = child->next;
	}

	if (!numEntries)
	{
		out = (char *)cJSON_malloc(fmt ? depth + 4 : 3);
		if (!out) return NULL;

		ptr = out;
		*ptr++ = '{';
		if (fmt)
		{
			*ptr++ = '\n';
			for (i = 0; i < depth - 1; i++)
			{
				*ptr++ = '\t';
			}
		}

		*ptr++ = '}';
		*ptr++ = '\0';

		return out;

	}

	entries = (char **)cJSON_malloc(numEntries * sizeof(char *));
	if (!entries) return NULL;

	names = (char **)cJSON_malloc(numEntries * sizeof(char *));
	if (!names) { cJSON_free(entries); return NULL; }

	memset(entries, 0, numEntries * sizeof(char *));
	memset(names, 0, numEntries * sizeof(char *));

	child = item->child;
	depth++;
	if (fmt) { len += depth; }

	while (child)
	{
		names[i] = str = print_string(child, 1);
		entries[i++] = ret = print_value(child, depth, fmt);
		if (str && ret) { len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0); }
		else { isFail = 1; }

		child = child->next;
	}

	if (!isFail) { out = (char *)cJSON_malloc(len); }
	if (!out) { isFail = 1; }

	if (isFail)
	{
		for (i = 0; i < numEntries; i++)
		{
			if (names[i]) { cJSON_free(names[i]); }
			if (entries[i]) { cJSON_free(entries[i]); }
		}
		cJSON_free(names);
		cJSON_free(entries);

		return NULL;
	}

	*out = '{';
	ptr = out + 1;
	if (fmt) { *ptr++ = '\n'; }
	*ptr = '\0';

	for (i = 0; i < numEntries; i++)
	{
		if (fmt)
		{
			for (int j = 0; j < depth; j++) { *ptr++ = '\t'; }
		}
		templen = strlen(names[i]);
		memcpy(ptr, names[i], templen);
		ptr += templen;
		*ptr++ = ':';
		if (fmt) { *ptr++ = '\t'; }

		strcpy(ptr, entries[i]);
		ptr += strlen(entries[i]);

		if (i != numEntries - 1) { *ptr++ = ','; }
		if (fmt) { *ptr++ = '\n'; }
		*ptr = '\0';

		cJSON_free(names[i]);
		cJSON_free(entries[i]);
	}
	cJSON_free(names);
	cJSON_free(entries);

	if (fmt)
	{
		for (i = 0; i < depth - 1; i++)
		{
			*ptr++ = '\t';
		}
	}

	*ptr++ = '}';
	*ptr++ = '\0';

	return out;
}

char *print_value(cJSON *item, int depth, int fmt)
{
	char *out = NULL;
	if (!item) return NULL;

	switch ((item->type) & 255)
	{
	case cJSON_NULL: out = cJSON_strdup("null"); break;
	case cJSON_False: out = cJSON_strdup("false"); break;
	case cJSON_True: out = cJSON_strdup("true"); break;
	case cJSON_Number: out = print_number(item); break;
	case cJSON_String: out = print_string(item, 0); break;
	case cJSON_Array: out = print_array(item, depth, fmt); break;
	case cJSON_Object: out = print_object(item, depth, fmt); break;
	}

	return out;
}

char *cJSON_Print(cJSON *item)
{
	return print_value(item, 0, 1);
}


/*==============================================================*/

cJSON *cJSON_CreateNull(void)
{
	cJSON *item = cJSON_New_Item();
	if (item) { item->type = cJSON_NULL; }
	return item;
}

cJSON *cJSON_CreateTrue(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)item->type = cJSON_True;
	return item;
}

cJSON *cJSON_CreateFalse(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)item->type = cJSON_False;
	return item;
}

cJSON *cJSON_CreateBool(int b)
{
	cJSON *item = cJSON_New_Item();
	if (item)item->type = b ? cJSON_True : cJSON_False;
	return item;
}

cJSON *cJSON_CreateNumber(double num)
{
	cJSON *item = cJSON_New_Item();
	if (item)
	{
		item->type = cJSON_Number;
		item->valueDouble = num;
		item->valueInt = (int)num;
	}
	return item;
}

cJSON *cJSON_CreateString(const char *string)
{
	cJSON *item = cJSON_New_Item();
	if (item)
	{
		item->type = cJSON_String;
		item->valueString = cJSON_strdup(string);
	}
	return item;
}

cJSON *cJSON_CreateArray(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)item->type = cJSON_Array;
	return item;
}

cJSON *cJSON_CreateObject(void)
{
	cJSON *item = cJSON_New_Item();
	if (item)item->type = cJSON_Object;
	return item;
}


void suffix_obect(cJSON *prev, cJSON *item)
{
	prev->next = item;
	item->prev = prev;
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
	if (!item) { return; }
	
	if (!item->string) { cJSON_free(item->string); }
	item->string = cJSON_strdup(string);
	cJSON_AddItemToArray(object, item);
}

void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
	cJSON *c = array->child;
	if (!item) return;

	if (!c)
	{
		array->child = item;
	}
	else
	{
		while (c && c->next)
		{
			c = c->next;
		}
		suffix_obect(c, item);
	}
}

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which)
{
	cJSON *c = array->child;
	while (c && which > 0)
	{
		c = c->next;
		which--;
	}
	if (!c) return NULL;
	if (c->prev) c->prev->next = c->next;
	if (c->next) c->next->prev = c->prev;
	if (c == array->child) array->child = c->next;

	c->next = c->prev = NULL;
	return c;
}

void cJSON_DeleteItemFromArray(cJSON *array, int which)
{
	cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

int cJSON_strcasecmp(const char *s1, const char *s2)
{
	int ret;
	if (!s1) return(s1 == s2) ? 0 : 1;
	if (!s2) return 1;

	for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
	{
		if (*s1 == 0) return 0;
	}
	ret = tolower(*(const unsigned char*)s1) - tolower(*(const unsigned char*)s2);
	return ret;
}

cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
	int i = 0;
	cJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->string, string))
	{
		i++;
		c = c->next;
	}

	if (c)
	{
		return cJSON_DetachItemFromArray(object, i);
	}

	return NULL;
}

void cJSON_DeleteItemFromObject(cJSON *object ,int which)
{
	cJSON_Delete(cJSON_DetachItemFromObject(object, which));
}

void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newItem)
{
	cJSON *c = array->child;
	while (c && which > 0)
	{
		c = c->next;
		which--;
	}
	if (!c)
	{
		cJSON_AddItemToArray(array, newItem);
		return;
	}
	newItem->next = c;
	newItem->prev = c->prev;
	c->prev = newItem;

	if (c == array->child) { array->child = newItem; }
	else { newItem->prev->next = newItem; }
}


void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newItem)
{
	cJSON *c = array->child;
	while (c && which > 0)
	{
		c = c->next;
		which--;
	}
	if (!c) return;

	newItem->next = c->next;
	newItem->prev = c->prev;

	if (newItem->next) newItem->next->prev = newItem;
	if (c == array->child) { array->child = newItem; }
	else { newItem->prev->next = newItem; }

	c->next = c->prev = NULL;
	cJSON_Delete(c);
}

void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newItem)
{
	int i = 0;
	cJSON *c = object->child;
	while (c && cJSON_strcasecmp(c->string, string))
	{
		i++;
		c = c->next;
	}

	if (c)
	{
		newItem->string = cJSON_strdup(string);
		cJSON_ReplaceItemInArray(object, i, newItem);
	}
}