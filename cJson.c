#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include "cJson.h"


static void *(*cJSON_malloc)(size_t sz) = malloc;	// ������������������malloc����������ΪcJSON_malloc�����ӿɶ���
static void(*cJSON_free)(void *ptr) = free;	// ������������������free����������ΪcJSON_free�����ӿɶ���

static const char *ep;	//EndPostion��д��ָ��ǰ�����Ľ�βλ��

char *cJSON_GetEndPosition(void)
{
    return ep;
}

cJSON *cJSON_New_Item()
{
    cJSON * node = (cJSON *)cJSON_malloc(sizeof(cJSON));	// �����ڴ�ռ�
    if (node) { memset(node, 0, sizeof(cJSON)); }	// ����ڴ����ɹ������ռ��е�ֵʹ��0��ʼ��
    return node;	// ����cJSON�½ڵ��ָ��
}

void cJSON_Delete(cJSON *c)	
{
    cJSON *next;
    while (c)	// ����cJSON�洢�ṹ
    {
        next = c->next;	// ʹ����ʱ�����洢��ǰ�ڵ����һ���ڵ�
        if (c->child) { cJSON_Delete(c->child); }	// �����ǰ�ڵ���ӽڵ㲻Ϊ�գ�����ɾ���ӽڵ�
        if (c->valueString) { cJSON_free(c->valueString); }	// �����ǰ�ڵ��valueString��Ϊ�գ��ͷ�ֵ�Ŀռ�
        if (c->string) { cJSON_free(c->string); }	// �����ǰ�ڵ��string��Ϊ�գ��ͷ�string�Ŀռ�
        cJSON_free(c);	// �ͷŵ�ǰ�ڵ�Ŀռ�
        c = next;
    }
}

const char *skip(const char *in)
{
    while (in && *in && ((unsigned char)*in<=32))    // ���in��Ϊ����*in��Ϊ����*in�Ĵ�СС��32
    {
        in++;   // in��������
    }
    return in;  // ����in
}

const char* parse_string(cJSON *item, const char *str)
{
    const char *ptr = str + 1;
    char *ptr2;
    char *out;

    int len = 0;
    if (*str != '\"')   // ���str��\���ſ�ͷ
    {
        ep = str;   // ������λ������Ϊstr
        return NULL;    // ����NULL
    }

    while (*ptr != '\"' && *ptr && ++len)   // �����ǰ�ַ���Ϊ\���ţ�����*ptr��Ϊ�գ�����len����������㣬
    {
        if (*ptr++ == '\\') { ptr++; }  // ���*ptr����\\���ţ���ptr������������ptr����һ��������
    }

    out = (char *)cJSON_malloc(len + 1);    // ��out����һ��len+1���ڴ�ռ�
    if (!out) return NULL;  // ����ڴ�ռ����ʧ�ܣ�����NULL

    ptr = str + 1;
    ptr2 = out;

    while (*ptr != '\"' && *ptr)
    {
        if (*ptr != '\\') { *ptr2++ = *ptr++; } // ���ptr������\\����ptr�е����ݴ洢��ptr2��
        else
        {
            // ����ptr�е�ת��������滻
        }
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
    
    item->valueString = out;    // ��out��ֵ����ΪvalueString
    item->type = cJSON_String;  // ��item->type����ΪcJSON_String

    return ptr;
}

const char* parse_number(cJSON *item, const char *num)
{
    double n = 0;		//���ռ��������
    int sign = 1;	//������
    int signSubScale = 1;   // ���ÿ�ѧ������ʱ��С������ƻ���ǰ�ƣ�Ĭ��Ϊ����
    int scale = 0;  // С�����λ��
    int subscale = 0;   // ��ѧ��������λ��

    if (*num == '-')    // �������Ϊ����
    {
        sign = -1;
        num++;
    }

    if (*num == '0') { num++; } // �������ֿ�ͷ��0

    if (*num >= '0' && *num <= '9') // �����ֿ�ͷ��Ϊ0�����Ƚ������ֵ���������
    {
        do
        {
            n = (n*10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == '.' && num[1] >= '0' && num[1] <= '9')  // ���ֵ��������ֽ�����Ϻ󣬿�ʼ�������ֵ�С������
    {
        num++;
        do
        {
            n = (n*10.0) + (*num++ - '0');
            scale--;
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == 'e' || *num == 'E') // ������ֲ��ÿ�ѧ������
    {
        num++;
        if (*num == '+') num++; // С�������
        else if (*num == '-')   // С����ǰ��
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

    item->valueDouble = n;  // �����ֵĸ���ֵ�洢��item�ڵ��valueDouble
    item->valueInt = (int)n;    // �����ֵ�����ֵ�洢��item�ڵ��valueInt������ȡ��
    item->type = cJSON_Number;  // ��item����������ΪcJSON_Number

    return num;
}

const char* parse_Array(cJSON *item, const char *value)
{
    //["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
    //   [],
    cJSON *child;   // ����һ���µ�cJSON�ڵ�
    if (*value != '[')  // ���value����[���ſ�ͷ����value����Ϊ��β��������NULL
    {
        ep = value;
        return NULL;
    }

    item->type = cJSON_Array;   // ��item��type����ΪcJSON_Array
    value = skip(value + 1);    // ����[����
    if (*value == ']') return value + 1;    // ���valueΪ]���ţ�˵����ǰArrayΪ[]������'\0'

    item->child = child = cJSON_New_Item(); // Ϊ�µ�cJSON�ڵ㴴���ռ�
    if (!item->child) return NULL;  // ��������ռ�ʧ�ܣ�����NULL

    value = skip(parse_value(child, skip(value)));  // ��ֵ�洢���ӽڵ��У������ӽڵ����������Ϊvalue
    if (!value) return NULL;    // ���valueΪ�գ�˵���Ѿ������β������NULL

    while (*value == ',')   // ���valueΪ,���ţ�˵����������ֵ�����������µ��ӽڵ㲢�洢
    {
        cJSON *new_item;
        if (!(new_item = cJSON_New_Item())) return NULL;

        child->next = new_item;
        new_item->prev = child;
        child = new_item;   // ��֮ǰ���ӽڵ��´����µ��ӽڵ㣬���ֱ�����ǰ���ӽڵ��ǰ���ڵ�ͺ�̽ڵ�

        value = skip(parse_value(child, skip(value + 1)));  // ��ֵ�洢���ӽڵ��У������ӽڵ����������Ϊvalue
        if (!value) return NULL;    // ���valueΪ�գ�˵���Ѿ������β������NULL
    }

    if (*value == ']') return value + 1;    // ���valueΪ]���ţ�˵����ǰArrayΪ[]������'\0'
    ep = value; // ����β��������Ϊvalue

    return NULL;
}

const char* parse_Object(cJSON *item, const char *value)
{
    cJSON *child;
    if (*value != '{')  // ���value������{���ſ�ͷ��˵�����Ǽ�ֵ�ԣ�����βλ������Ϊvalue��������NULL
    {
        ep = value;
        return NULL;
    }

    item->type = cJSON_Object;  // ��item��type����ΪcJSON_Object�ڵ�
    value = skip(value + 1);    // ����value��ͷ��{����
    if (*value == '}') return value + 1;    // �����ǰvalue��ֵΪ}���ţ�˵���Ѿ�����β��ֱ�ӷ���'\0'��������

    item->child = child = cJSON_New_Item(); // �����µ�cJSON�ڵ���Ϊ�ӽڵ�
    if (!item->child) return NULL;  // ��������ڵ�ʧ�ܣ�����NULL

    value = skip(parse_string(child, skip(value))); // ��value�е�string�洢���ӽڵ��У�����value��������
    if (!value) return NULL;    // �����ǰvalueΪ�գ�����NULL

    child->string = child->valueString; // ���ӽڵ��е�valueString����string
    child->valueString = NULL;  // ���ӽڵ��е�valueString����ΪNULL
    if (*value != ':')  // �����ǰvalue��Ϊ:����
    {
        ep = value; // ����βλ������Ϊvalue
        return NULL;    // ����NULL
    }

    value = skip(parse_value(child, skip(value + 1)));  // ��value�е�ֵ�洢���ӽڵ��У�����value��������
    if (!value) return NULL;    // �����ǰvalueΪ�գ�����NULL

    while (*value == ',')   // ����value�������ǰvalueΪ,���ţ�˵��������һ����ֵ����Ҫ���д洢
    {
        cJSON *new_item;    // ����һ���µ�cJSON�ڵ�
        if (!(new_item = cJSON_New_Item())) return NULL;    // ����½ڵ㴴��ʧ�ܣ�����NULL

        child->next = new_item;
        new_item->prev = child;
        child = new_item;   // ����ǰ���ӽڵ��ǰ���ڵ�ͺ�̽ڵ�

        value = skip(parse_string(child, skip(value + 1))); // ��value�е�stringֵ�����ӽڵ㣬����value��������
        if (!value) return NULL;    // �����ǰvalueΪ�գ�����NULL
        
        child->string = child->valueString; // ������֮ǰ�洢valueֵ��ͬ
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

    ep = value; // ����ǰ�����Ľ�βλ������Ϊvalue
    return NULL;
}

const char *parse_value(cJSON *item, const char *value)
{
    if (!value) return NULL;    // ���ֵ��ָ��Ϊ�գ�����NULL
    // strncmp(string1, string2, count): �Ƚ�string1��string2�����ǰcount���ַ������string1�����ַ���С��string2�����ַ���������ֵ<0�����string1�����ַ�������string2�����ַ���������ֵ=0�����string1�����ַ�������string2�����ַ���������ֵ>0��
    if (!strncmp(value, "false", 5))    // ���value����false
    {
        item->type = cJSON_False;   // ��item��type����ΪcJSON_False
        return value + 5;   // value����5
    }

    if (!strncmp(value, "true", 4)) // ���value����true
    {
        item->type = cJSON_True;    // ��item��type����ΪcJSON_True
        item->valueInt = 1; // ��item��valueInt����Ϊ1
        return value + 4;   // value����4
    }

    if (!strncmp(value, "null", 4)) // ���value����null
    {
        item->type = cJSON_NULL;    // ��item��type����ΪcJSON_NULL
        return value + 4;   // value����4
    }

    if (*value == '\"') { return parse_string(item, value); }   // ���value����ʼֵΪ\���ţ���value��string��ʽ����item
    if (*value == '-' || (*value>='0' && *value <= '9')) { return parse_number(item, value); }  // ���value����ʼֵΪ-���ţ���value��number��ʽ����item
    if (*value == '[') { return parse_Array(item, value); }     // ���value����ʼֵΪ[���ţ���value��������ʽ����item
    if (*value == '{') { return parse_Object(item, value); }    // ���value��ֵΪ{���ţ���value�Լ�ֵ����ʽ����item

    ep = value; // ����ǰ����λ������Ϊvalue
    return NULL;    // ����NULL
}

cJSON *cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}

cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
    const char *end = NULL; // ��β�ڵ�
    cJSON *c = cJSON_New_Item();    // ����һ���µĽڵ�
    ep = NULL;  // ��ǰ������λ������ΪNULL
    if (!c) return NULL;    // ����ڴ����ʧ�ܣ�����NULL

    end = parse_value(c, skip(value));  // ��value����c�ڵ�
    if (!end)   // �����ǰ�Ľ�β�ڵ㲻Ϊ�գ���ɾ��c�ڵ㣬������NULL
    {
        cJSON_Delete(c);
        return NULL;
    }
    
    if (require_null_terminated)    // ���require_null_terminated��Ϊ��
    {
        end = skip(end);    // ��end����
        if (*end)   // ���endָ���ֵ��Ϊ��
        {
            cJSON_Delete(c);    // ɾ��c�ڵ�
            ep = end;   // ����ǰ����λ������Ϊend
            return NULL;    // ����NULL
        }
    }

    if (return_parse_end)   // ���return_parse_end��Ϊ��
    {
        *return_parse_end = end;    // ��return_parse_end����Ϊend
    }

    return c;

}

/*==========================================================================*/

char *cJSON_strdup(const char *str)
{
    char *copy = NULL;
    size_t len;

    len = strlen(str);  // ��ȡstr�ĳ���
    copy =(char *) cJSON_malloc(len + 1);   // ����һ���µ�copy�ڵ㣬������strһ��
    if (!copy) return NULL; // ���copy�ڵ㴴��ʧ�ܣ�����NULL
    memcpy(copy, str, len + 1); // ��str���Ƶ�copy��

    return copy;
}

char *print_number(cJSON *item)
{
    char *str = NULL;
    double d = item->valueDouble;   // ��item�е�valueDouble������ʱ����
    if (d == 0) // ���dΪ0��������ַ���"0"
    {
        str = (char *)cJSON_malloc(2);
        if (str) strcpy(str, "0");
    }

    //����
    else if ((fabs((double)(item->valueInt) - d) <= DBL_EPSILON) && d <= INT_MAX && d >= INT_MIN)   // valueDoubleΪ���������
    {
        str = (char *)cJSON_malloc(21);
        if (str) sprintf(str, "%d", item->valueInt);
    }
    else
    {
        str = (char *)cJSON_malloc(64); // valueDoubleΪ�����������
        if (str)
        {
            //1234xxxx.0
            if ((fabs(floor(d) - d) <= DBL_EPSILON) && fabs(d) < 1.0e60) { sprintf(str, "%.0f", d); }   // �����ְ�������.0�ķ�ʽ��ʾ
            else if (fabs(d) < 1.0e-6 || fabs(1.0e9)) { sprintf(str, "%e", d); }    // �����ְ��տ�ѧ�����ķ�ʽ��ʾ
            else { sprintf(str, "%f", d); } // �����ְ��ո������ķ�ʽ��ʾ
        }
    }

    return str;

}

char *print_string(cJSON *item, int isName)
{
    char *str = NULL;
    if(isName){ str = item->string; }   // ���isName��Ϊ�գ���item�ڵ��е�string����str
    else{ str = item->valueString; }    // ����item�е�valueString����str
    char *ptr, *ptr2, *out;
    unsigned char token;
    int flag = 0, len = 0;

    for (ptr = str; *ptr; ptr++)    // ����str����ptrָ���ʱѭ������
    {
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;  // �ж�str���Ƿ����ַ����ͻ���\���Ż���\\���ţ��ǵĻ�flagΪ1������flagΪ0
    }

    if (!flag)  // ���flagΪ0
    {
        len = ptr - str;    // ���str����
        out = (char *)cJSON_malloc(len + 2 + 1);    // �����µ�char�ڵ㣬ָ��ռ�Ĵ�С��str��2
        if (!out) return NULL;  // ����ڴ����ʧ�ܣ�����NULL

        ptr2 = out; // ʹptr��out���
        *ptr2++ = '\"'; // ptr2��\���ſ�ʼ
        strcpy(ptr2, str);  // ��str�е����ݸ��Ƶ�ptr2��
        //"abcde"\0
        ptr2[len] = '\"';   // ptr2��\���Ž���
        ptr2[len + 1] = '\0';   // ��ptr2��β����'\0'

        return out;
    }

    if (!str)   // ���strΪ�գ���""���ƽ�out��
    {
        out = (char *)cJSON_malloc(3);
        if (!out) return NULL;
        strcpy(out, "\"\"");
        return out;
    }

    ptr = str;
    while ((token = *ptr) && ++len)
    {
        // char *strchr(const char *str, int c);    ����str��c�ĵ�һ��ƥ�������Ҳ���c���򷵻�NULL�������а���NULL��ֹ���ַ�
        if (strchr("\"\\\b\f\n\r\t", token)) { len++; } // �����token���Ҳ���XXX�ȷ��ţ�len��������
        else if (token < 32) { len += 5; ptr++; }   // ���token���ַ���len����5��ptr��������
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
            switch (token = *ptr)   // �жϵ�ǰptrָ�������
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

    cJSON *child = item->child; // ����child�ڵ�Ϊitem���ӽڵ�
    int numEntries = 0;

    while (child)   // ����item���ӽڵ㣬��ͳ���ӽڵ������
    {
        //�������
        numEntries++;
        child = child->next;
    }

    if (!numEntries)    // ����ӽڵ������Ϊ0���򷵻��ַ���[]
    {
        out = (char *)cJSON_malloc(3);
        if (out) strcpy(out, "[]");
        return out;
    }

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));   // ����ӽڵ��������Ϊ0������һ������ΪnumEntries��char*�ڵ㣬�������ڴ�ռ�
    if (!entries) return NULL;  // ����ڴ����ʧ�ܣ�����NULL
    memset(entries, 0, numEntries * sizeof(char *));    // ��entries��ʼ��Ϊ0

    child = item->child;    // ��child����Ϊitem���ӽڵ�
    while (child)   // ����child��ֱ��childΪ�սڵ�
    {
        ret = print_value(child, depth + 1, fmt);   //�� ����δ֪
        entries[i++] = ret;
        if (ret) { len += strlen(ret) + 2 + (fmt ? 1 : 0); }
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }   // ���isFailΪ0��Ϊout����len���ȵĿռ�
    if (!out) { isFail = 1; }   // ���out����ռ�ʧ�ܣ���isFail����Ϊ1

    if (isFail) // ���isFailΪ1������ͷ�entries�е�����
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
    for (i = 0; i < numEntries; i++)    // ��entries�е����ݿ�����ptr�У�����ÿ��ֵ�м��ԣ����Ÿ�����ͬʱ�ͷ�entries�Ŀռ�
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

    cJSON *child = item->child; // ����childΪitem�ڵ���ӽڵ�
    int numEntries = 0, isFail = 0;

    while (child)   // ͳ��child�������ӽڵ������
    {
        numEntries++;
        child = child->next;
    }

    if (!numEntries)    // ���child���ӽڵ������Ϊ0
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

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));   // ���child���ӽڵ�������Ϊ0������һ������ΪnumEntries��char*�ڵ㣬���ڴ洢��ֵ���е�ֵ
    if (!entries) return NULL;

    names = (char **)cJSON_malloc(numEntries * sizeof(char *)); // ����һ������ΪnumEntries��char*�ڵ㣬����Ϊnames�����ڴ洢��ֵ���еļ�
    if (!names) { cJSON_free(entries); return NULL; }

    memset(entries, 0, numEntries * sizeof(char *));    // ʹ��0��ʼ��entries
    memset(names, 0, numEntries * sizeof(char *));  // ʹ��0��ʼ��names

    child = item->child;    // ��child����Ϊitem���ӽڵ�
    depth++;
    if (fmt) { len += depth; }

    while (child)   // ����child�����ӽڵ�
    {
        names[i] = str = print_string(child, 1);    // ȡ��child�ڵ���ַ�����������name��
        entries[i++] = ret = print_value(child, depth, fmt);
        if (str && ret) { len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0); }   // ���str��ret����Ϊ�գ����ȡ�ɹ�������isFail����Ϊ1
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }   // ���str��ret��ȡʧ�ܣ�����һ������Ϊlen��charָ��
    if (!out) { isFail = 1; }   // ���outΪ�գ���isFail����Ϊ1

    if (isFail) // ���isFailΪ1������ͷ�names��entries�е�����
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

    // ��ӡ��ֵ���е�����
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
    prev->next = item;  // ǰ���ڵ����һ���ڵ�Ϊitem
    item->prev = prev;  // ��item��ǰ���ڵ�����Ϊprev
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!item) { return; }
    
    if (!item->string) { cJSON_free(item->string); }    // ���item��stringΪ�գ��ͷ�item��string
    item->string = cJSON_strdup(string);    // ��item��string����Ϊstring
    cJSON_AddItemToArray(object, item); // ��item��ӵ�array�ڵ�
}

void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    cJSON *c = array->child;    // ��c����Ϊarray�ڵ���ӽڵ�
    if (!item) return;  // ���item�ڵ�Ϊ�գ�ֱ�ӷ���

    if (!c) // ���array���ӽڵ�Ϊ��
    {
        array->child = item;    // ��array���ӽڵ�����Ϊitem
    }
    else
    {
        while (c && c->next)    // �ҵ�array�����һ���ӽڵ�
        {
            c = c->next;
        }
        suffix_obect(c, item);  // ��item����Ϊc�ĺ�̽ڵ�
    }
}

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which)
{
    cJSON *c = array->child;    // ��c����Ϊarray���ӽڵ�
    while (c && which > 0)  // c��Ϊ�ղ���which����0��֮��c���һ���ڵ㣬which�Լ�1
    {
        c = c->next;
        which--;
    }
    if (!c) return NULL;    // �����ǰcΪ�գ�˵��δ�ҵ�������NULL
    if (c->prev) c->prev->next = c->next;   // �����ǰc��Ϊ�գ�˵���Ѿ��ҵ���ɾ��c�ڵ�
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
    if (!s1) return(s1 == s2) ? 0 : 1;  // ���s1Ϊ0������s2ҲΪ0���򷵻�0�����򷵻�1
    if (!s2) return 1;  // ���s2Ϊ0������1

    for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)    // �ֱ�s1��s2ȫ��תΪСд�������бȽϣ���s1��s2���ʱֹͣѭ��
    {
        if (*s1 == 0) return 0; // ���s1��ֵΪ0������0
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