#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include "cJson.h"


static void *(*cJSON_malloc)(size_t sz) = malloc;	// 类似重命名函数，将malloc函数重命名为cJSON_malloc，增加可读性
static void(*cJSON_free)(void *ptr) = free;	// 类似重命名函数，将free函数重命名为cJSON_free，增加可读性

static const char *ep;	//EndPostion缩写，指向当前解析的结尾位置

char *cJSON_GetEndPosition(void)
{
    return ep;
}

cJSON *cJSON_New_Item()
{
    cJSON * node = (cJSON *)cJSON_malloc(sizeof(cJSON));	// 分配内存空间
    if (node) { memset(node, 0, sizeof(cJSON)); }	// 如果内存分配成功，将空间中的值使用0初始化
    return node;	// 返回cJSON新节点的指针
}

void cJSON_Delete(cJSON *c)	
{
    cJSON *next;
    while (c)	// 遍历cJSON存储结构
    {
        next = c->next;	// 使用临时变量存储当前节点的下一个节点
        if (c->child) { cJSON_Delete(c->child); }	// 如果当前节点的子节点不为空，首先删除子节点
        if (c->valueString) { cJSON_free(c->valueString); }	// 如果当前节点的valueString不为空，释放值的空间
        if (c->string) { cJSON_free(c->string); }	// 如果当前节点的string不为空，释放string的空间
        cJSON_free(c);	// 释放当前节点的空间
        c = next;
    }
}

const char *skip(const char *in)
{
    while (in && *in && ((unsigned char)*in<=32))    // 如果in不为零且*in不为零且*in的大小小于32
    {
        in++;   // in进行自增
    }
    return in;  // 返回in
}

const char* parse_string(cJSON *item, const char *str)
{
    const char *ptr = str + 1;
    char *ptr2;
    char *out;

    int len = 0;
    if (*str != '\"')   // 如果str以\符号开头
    {
        ep = str;   // 将结束位置设置为str
        return NULL;    // 返回NULL
    }

    while (*ptr != '\"' && *ptr && ++len)   // 如果当前字符不为\符号，并且*ptr不为空，并且len自增后大于零，
    {
        if (*ptr++ == '\\') { ptr++; }  // 如果*ptr等于\\符号（对ptr进行自增），ptr增加一次自增，
    }

    out = (char *)cJSON_malloc(len + 1);    // 对out分配一个len+1的内存空间
    if (!out) return NULL;  // 如果内存空间分配失败，返回NULL

    ptr = str + 1;
    ptr2 = out;

    while (*ptr != '\"' && *ptr)
    {
        if (*ptr != '\\') { *ptr2++ = *ptr++; } // 如果ptr不等于\\，将ptr中的内容存储到ptr2中
        else
        {
            // 否则将ptr中的转义符进行替换
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
    
    item->valueString = out;    // 将out的值设置为valueString
    item->type = cJSON_String;  // 将item->type设置为cJSON_String

    return ptr;
}

const char* parse_number(cJSON *item, const char *num)
{
    double n = 0;		//最终计算的数字
    int sign = 1;	//正负号
    int signSubScale = 1;   // 采用科学计数法时，小数点后移或者前移，默认为后移
    int scale = 0;  // 小数点后位数
    int subscale = 0;   // 科学计数法的位数

    if (*num == '-')    // 如果数字为负数
    {
        sign = -1;
        num++;
    }

    if (*num == '0') { num++; } // 跳过数字开头的0

    if (*num >= '0' && *num <= '9') // 当数字开头不为0后，首先解析数字的整数部分
    {
        do
        {
            n = (n*10.0) + (*num++ - '0');
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == '.' && num[1] >= '0' && num[1] <= '9')  // 数字的整数部分解析完毕后，开始解析数字的小数部分
    {
        num++;
        do
        {
            n = (n*10.0) + (*num++ - '0');
            scale--;
        } while (*num >= '0' && *num <= '9');
    }

    if (*num == 'e' || *num == 'E') // 如果数字采用科学计数法
    {
        num++;
        if (*num == '+') num++; // 小数点后移
        else if (*num == '-')   // 小数点前移
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

    item->valueDouble = n;  // 将数字的浮点值存储入item节点的valueDouble
    item->valueInt = (int)n;    // 将数字的整数值存储入item节点的valueInt，向下取整
    item->type = cJSON_Number;  // 将item的类型设置为cJSON_Number

    return num;
}

const char* parse_Array(cJSON *item, const char *value)
{
    //["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
    //   [],
    cJSON *child;   // 创建一个新的cJSON节点
    if (*value != '[')  // 如果value不以[符号开头，将value设置为结尾，并返回NULL
    {
        ep = value;
        return NULL;
    }

    item->type = cJSON_Array;   // 将item的type设置为cJSON_Array
    value = skip(value + 1);    // 跳过[符号
    if (*value == ']') return value + 1;    // 如果value为]符号，说明当前Array为[]，返回'\0'

    item->child = child = cJSON_New_Item(); // 为新的cJSON节点创建空间
    if (!item->child) return NULL;  // 如果创建空间失败，返回NULL

    value = skip(parse_value(child, skip(value)));  // 将值存储入子节点中，并且子节点的类型设置为value
    if (!value) return NULL;    // 如果value为空，说明已经到达结尾，返回NULL

    while (*value == ',')   // 如果value为,符号，说明后续仍有值，继续创建新的子节点并存储
    {
        cJSON *new_item;
        if (!(new_item = cJSON_New_Item())) return NULL;

        child->next = new_item;
        new_item->prev = child;
        child = new_item;   // 在之前的子节点下创建新的子节点，并分别设置前后子节点的前驱节点和后继节点

        value = skip(parse_value(child, skip(value + 1)));  // 将值存储入子节点中，并且子节点的类型设置为value
        if (!value) return NULL;    // 如果value为空，说明已经到达结尾，返回NULL
    }

    if (*value == ']') return value + 1;    // 如果value为]符号，说明当前Array为[]，返回'\0'
    ep = value; // 将结尾符号设置为value

    return NULL;
}

const char* parse_Object(cJSON *item, const char *value)
{
    cJSON *child;
    if (*value != '{')  // 如果value不是以{符号开头，说明不是键值对，将结尾位置设置为value，并返回NULL
    {
        ep = value;
        return NULL;
    }

    item->type = cJSON_Object;  // 将item的type设置为cJSON_Object节点
    value = skip(value + 1);    // 跳过value开头的{符号
    if (*value == '}') return value + 1;    // 如果当前value的值为}符号，说明已经到结尾，直接返回'\0'结束符号

    item->child = child = cJSON_New_Item(); // 创建新的cJSON节点作为子节点
    if (!item->child) return NULL;  // 如果创建节点失败，返回NULL

    value = skip(parse_string(child, skip(value))); // 将value中的string存储到子节点中，并对value进行自增
    if (!value) return NULL;    // 如果当前value为空，返回NULL

    child->string = child->valueString; // 将子节点中的valueString存入string
    child->valueString = NULL;  // 将子节点中的valueString设置为NULL
    if (*value != ':')  // 如果当前value不为:符号
    {
        ep = value; // 将结尾位置设置为value
        return NULL;    // 返回NULL
    }

    value = skip(parse_value(child, skip(value + 1)));  // 将value中的值存储到子节点中，并对value进行自增
    if (!value) return NULL;    // 如果当前value为空，返回NULL

    while (*value == ',')   // 遍历value，如果当前value为,符号，说明存在下一个键值对需要进行存储
    {
        cJSON *new_item;    // 创建一个新的cJSON节点
        if (!(new_item = cJSON_New_Item())) return NULL;    // 如果新节点创建失败，返回NULL

        child->next = new_item;
        new_item->prev = child;
        child = new_item;   // 调整前后子节点的前驱节点和后继节点

        value = skip(parse_string(child, skip(value + 1))); // 将value中的string值存入子节点，并对value进行自增
        if (!value) return NULL;    // 如果当前value为空，返回NULL
        
        child->string = child->valueString; // 步骤与之前存储value值相同
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

    ep = value; // 将当前解析的结尾位置设置为value
    return NULL;
}

const char *parse_value(cJSON *item, const char *value)
{
    if (!value) return NULL;    // 如果值的指针为空，返回NULL
    // strncmp(string1, string2, count): 比较string1和string2中最多前count个字符，如果string1的子字符串小于string2的子字符串，返回值<0，如果string1的子字符串等于string2的子字符串，返回值=0，如果string1的子字符串大于string2的子字符串，返回值>0，
    if (!strncmp(value, "false", 5))    // 如果value等于false
    {
        item->type = cJSON_False;   // 将item的type设置为cJSON_False
        return value + 5;   // value增加5
    }

    if (!strncmp(value, "true", 4)) // 如果value等于true
    {
        item->type = cJSON_True;    // 将item的type设置为cJSON_True
        item->valueInt = 1; // 将item的valueInt设置为1
        return value + 4;   // value增加4
    }

    if (!strncmp(value, "null", 4)) // 如果value等于null
    {
        item->type = cJSON_NULL;    // 将item的type设置为cJSON_NULL
        return value + 4;   // value增加4
    }

    if (*value == '\"') { return parse_string(item, value); }   // 如果value的起始值为\符号，将value以string形式存入item
    if (*value == '-' || (*value>='0' && *value <= '9')) { return parse_number(item, value); }  // 如果value的起始值为-符号，将value以number形式存入item
    if (*value == '[') { return parse_Array(item, value); }     // 如果value的起始值为[符号，将value以数组形式存入item
    if (*value == '{') { return parse_Object(item, value); }    // 如果value的值为{符号，将value以键值对形式存入item

    ep = value; // 将当前解析位置设置为value
    return NULL;    // 返回NULL
}

cJSON *cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}

cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated)
{
    const char *end = NULL; // 结尾节点
    cJSON *c = cJSON_New_Item();    // 创建一个新的节点
    ep = NULL;  // 当前解析的位置设置为NULL
    if (!c) return NULL;    // 如果内存分配失败，返回NULL

    end = parse_value(c, skip(value));  // 将value存入c节点
    if (!end)   // 如果当前的结尾节点不为空，则删除c节点，并返回NULL
    {
        cJSON_Delete(c);
        return NULL;
    }
    
    if (require_null_terminated)    // 如果require_null_terminated不为空
    {
        end = skip(end);    // 对end自增
        if (*end)   // 如果end指向的值不为空
        {
            cJSON_Delete(c);    // 删除c节点
            ep = end;   // 将当前解析位置设置为end
            return NULL;    // 返回NULL
        }
    }

    if (return_parse_end)   // 如果return_parse_end不为空
    {
        *return_parse_end = end;    // 将return_parse_end设置为end
    }

    return c;

}

/*==========================================================================*/

char *cJSON_strdup(const char *str)
{
    char *copy = NULL;
    size_t len;

    len = strlen(str);  // 获取str的长度
    copy =(char *) cJSON_malloc(len + 1);   // 创建一个新的copy节点，长度与str一致
    if (!copy) return NULL; // 如果copy节点创建失败，返回NULL
    memcpy(copy, str, len + 1); // 将str复制到copy中

    return copy;
}

char *print_number(cJSON *item)
{
    char *str = NULL;
    double d = item->valueDouble;   // 将item中的valueDouble存入临时变量
    if (d == 0) // 如果d为0，则存入字符串"0"
    {
        str = (char *)cJSON_malloc(2);
        if (str) strcpy(str, "0");
    }

    //整数
    else if ((fabs((double)(item->valueInt) - d) <= DBL_EPSILON) && d <= INT_MAX && d >= INT_MIN)   // valueDouble为整数的情况
    {
        str = (char *)cJSON_malloc(21);
        if (str) sprintf(str, "%d", item->valueInt);
    }
    else
    {
        str = (char *)cJSON_malloc(64); // valueDouble为浮点数的情况
        if (str)
        {
            //1234xxxx.0
            if ((fabs(floor(d) - d) <= DBL_EPSILON) && fabs(d) < 1.0e60) { sprintf(str, "%.0f", d); }   // 将数字按照整数.0的方式表示
            else if (fabs(d) < 1.0e-6 || fabs(1.0e9)) { sprintf(str, "%e", d); }    // 将数字按照科学计数的方式表示
            else { sprintf(str, "%f", d); } // 将数字按照浮点数的方式表示
        }
    }

    return str;

}

char *print_string(cJSON *item, int isName)
{
    char *str = NULL;
    if(isName){ str = item->string; }   // 如果isName不为空，将item节点中的string存入str
    else{ str = item->valueString; }    // 否则将item中的valueString存入str
    char *ptr, *ptr2, *out;
    unsigned char token;
    int flag = 0, len = 0;

    for (ptr = str; *ptr; ptr++)    // 遍历str，当ptr指向空时循环结束
    {
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;  // 判断str中是否有字符类型或者\符号或者\\符号，是的话flag为1，否则flag为0
    }

    if (!flag)  // 如果flag为0
    {
        len = ptr - str;    // 求解str长度
        out = (char *)cJSON_malloc(len + 2 + 1);    // 创建新的char节点，指向空间的大小比str大2
        if (!out) return NULL;  // 如果内存分配失败，返回NULL

        ptr2 = out; // 使ptr与out相等
        *ptr2++ = '\"'; // ptr2以\符号开始
        strcpy(ptr2, str);  // 将str中的内容复制到ptr2中
        //"abcde"\0
        ptr2[len] = '\"';   // ptr2以\符号结束
        ptr2[len + 1] = '\0';   // 在ptr2结尾增加'\0'

        return out;
    }

    if (!str)   // 如果str为空，将""复制进out中
    {
        out = (char *)cJSON_malloc(3);
        if (!out) return NULL;
        strcpy(out, "\"\"");
        return out;
    }

    ptr = str;
    while ((token = *ptr) && ++len)
    {
        // char *strchr(const char *str, int c);    查找str中c的第一个匹配项，如果找不到c，则返回NULL，搜索中包括NULL终止的字符
        if (strchr("\"\\\b\f\n\r\t", token)) { len++; } // 如果在token中找不到XXX等符号，len进行自增
        else if (token < 32) { len += 5; ptr++; }   // 如果token是字符，len自增5，ptr进行自增
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
            switch (token = *ptr)   // 判断当前ptr指向的内容
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

    cJSON *child = item->child; // 设置child节点为item的子节点
    int numEntries = 0;

    while (child)   // 遍历item的子节点，并统计子节点的数量
    {
        //广度优先
        numEntries++;
        child = child->next;
    }

    if (!numEntries)    // 如果子节点的数量为0，则返回字符串[]
    {
        out = (char *)cJSON_malloc(3);
        if (out) strcpy(out, "[]");
        return out;
    }

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));   // 如果子节点的数量不为0，创建一个长度为numEntries的char*节点，并分配内存空间
    if (!entries) return NULL;  // 如果内存分配失败，返回NULL
    memset(entries, 0, numEntries * sizeof(char *));    // 将entries初始化为0

    child = item->child;    // 将child重置为item的子节点
    while (child)   // 遍历child，直至child为空节点
    {
        ret = print_value(child, depth + 1, fmt);   //？ 功能未知
        entries[i++] = ret;
        if (ret) { len += strlen(ret) + 2 + (fmt ? 1 : 0); }
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }   // 如果isFail为0，为out分配len长度的空间
    if (!out) { isFail = 1; }   // 如果out分配空间失败，将isFail设置为1

    if (isFail) // 如果isFail为1，逐个释放entries中的内容
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
    for (i = 0; i < numEntries; i++)    // 将entries中的内容拷贝到ptr中，并且每个值中间以，符号隔开，同时释放entries的空间
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

    cJSON *child = item->child; // 创建child为item节点的子节点
    int numEntries = 0, isFail = 0;

    while (child)   // 统计child的所有子节点的数量
    {
        numEntries++;
        child = child->next;
    }

    if (!numEntries)    // 如果child的子节点的数量为0
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

    entries = (char **)cJSON_malloc(numEntries * sizeof(char *));   // 如果child的子节点数量不为0，创建一个长度为numEntries的char*节点，用于存储键值对中的值
    if (!entries) return NULL;

    names = (char **)cJSON_malloc(numEntries * sizeof(char *)); // 创建一个长度为numEntries的char*节点，命名为names，用于存储键值对中的键
    if (!names) { cJSON_free(entries); return NULL; }

    memset(entries, 0, numEntries * sizeof(char *));    // 使用0初始化entries
    memset(names, 0, numEntries * sizeof(char *));  // 使用0初始化names

    child = item->child;    // 将child重置为item的子节点
    depth++;
    if (fmt) { len += depth; }

    while (child)   // 遍历child及其子节点
    {
        names[i] = str = print_string(child, 1);    // 取出child节点的字符串，并存入name中
        entries[i++] = ret = print_value(child, depth, fmt);
        if (str && ret) { len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0); }   // 如果str和ret均不为空，则读取成功，否则将isFail设置为1
        else { isFail = 1; }

        child = child->next;
    }

    if (!isFail) { out = (char *)cJSON_malloc(len); }   // 如果str和ret读取失败，创建一个长度为len的char指针
    if (!out) { isFail = 1; }   // 如果out为空，将isFail设置为1

    if (isFail) // 如果isFail为1，逐个释放names和entries中的内容
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

    // 打印键值对中的内容
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
    prev->next = item;  // 前驱节点的下一个节点为item
    item->prev = prev;  // 将item的前驱节点设置为prev
}

void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (!item) { return; }
    
    if (!item->string) { cJSON_free(item->string); }    // 如果item的string为空，释放item的string
    item->string = cJSON_strdup(string);    // 将item的string设置为string
    cJSON_AddItemToArray(object, item); // 将item添加到array节点
}

void cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    cJSON *c = array->child;    // 将c设置为array节点的子节点
    if (!item) return;  // 如果item节点为空，直接返回

    if (!c) // 如果array的子节点为空
    {
        array->child = item;    // 将array的子节点设置为item
    }
    else
    {
        while (c && c->next)    // 找到array的最后一个子节点
        {
            c = c->next;
        }
        suffix_obect(c, item);  // 将item设置为c的后继节点
    }
}

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which)
{
    cJSON *c = array->child;    // 将c设置为array的子节点
    while (c && which > 0)  // c不为空并且which大于0，之后c向后一个节点，which自减1
    {
        c = c->next;
        which--;
    }
    if (!c) return NULL;    // 如果当前c为空，说明未找到，返回NULL
    if (c->prev) c->prev->next = c->next;   // 如果当前c不为空，说明已经找到，删除c节点
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
    if (!s1) return(s1 == s2) ? 0 : 1;  // 如果s1为0，并且s2也为0，则返回0，否则返回1
    if (!s2) return 1;  // 如果s2为0，返回1

    for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)    // 分别将s1和s2全部转为小写，并进行比较，当s1和s2相等时停止循环
    {
        if (*s1 == 0) return 0; // 如果s1的值为0，返回0
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