#pragma once

#ifndef _CJSON_H_
#define _CJSON_H_

/*
������飺
json�������ڵ����ɾ�Ĳ�

{
	"name": "Jack (\"Bee\") Nimble",
	"format": {
		"type":       "rect",
		"width":      1920,
		"height":     1080,
		"interlace":  false,
		"frame rate": 24
	}
}

�߼��ϵ���״�ṹ

root ---> child(name)
				format--->child(type)
								width
								height
								interlace
								frame rate
*/

#define cJSON_False		0
#define cJSON_True		1
#define cJSON_NULL		2
#define cJSON_Number	3
#define cJSON_String	4
#define cJSON_Array		5
#define cJSON_Object	6


/*
���ܣ�����һ��stringֵΪname��cJSON_False�ڵ㣬����ӵ�object�ڵ�
*/
#define cJSON_AddFalseToObject(object, name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateFalse())

/*����һ��stringֵΪname��cJSON_True�ڵ�,����ӵ�object�ڵ�*/
#define cJSON_AddTrueToObject(object,name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())

/*����һ��stringֵΪname��cJSON_Bool/False�ڵ�,����ӵ�object�ڵ�*/
#define cJSON_AddBoolToObject(object,name, b) \
	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))

/*����һ��stringֵΪname��cJSON_NULL�ڵ�,����ӵ�object�ڵ�*/
#define cJSON_AddNULLToObject(object,name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateNull())

/*����һ��stringֵΪname��cJSON_Number�ڵ�,����ӵ�object�ڵ�*/
#define cJSON_AddNumberToObject(object,name, number) \
	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(number))

/*����һ��stringֵΪname��cJSON_String�ڵ�,����ӵ�object�ڵ�*/
#define cJSON_AddStringToObject(object,name, s) \
	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#define cJSON_SetIntValue(object, val)\
	((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))	// ���object��Ϊ�棬ִ��(object)->valueInt=(object)->valueDouble=(val)������ֵΪval������ֱ�ӷ���val

#define cJSON_SetNumberValue(object, val)\
	((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))	// ���object��Ϊ�棬ִ��(object)->valueInt=(object)->valueDouble=(val)������ֵΪval������ֱ�ӷ���val

typedef struct cJSON
{
	struct cJSON *next, *prev;	// ����ĳ�����ṹ�洢cJSON�ڵ�
	struct cJSON *child;

	/*array�Լ�object������Ҫ�����ӽڵ�*/
	int type;

	char *valueString;
	int valueInt;
	double valueDouble;

	char *string;
}cJSON;

/*���ý�������*/


/*
�������ܣ�
	����json���ݣ�����������json����
������
	value (const char *):char*ָ�룬ָ���������json����
����ֵ��
	json���ĸ��ڵ�
*/
cJSON *cJSON_Parse(const char *value);

/*
�������ܣ�
	��item�ڵ㿪ʼ�ݹ���������ڵ���ת��Ϊ�ַ���
������
	item(cJSON *item):cJson�ڵ�
����ֵ��
	�ַ���ָ�룬��item�ڵ����������ַ���
ע�����
	ʹ�øú�������Ҫ���ݷ��ص�char*ָ���ͷ��ڴ档
	out = cJSON_Print(cJSON *item)
	prinf("%s",out);
	free(out)
*/
char *cJSON_Print(cJSON *item);

/*
�������ܣ�
	�Ӹ��ڵ㿪ʼ�ݹ�ɾ��json�������ڵ㣬�ͷ��ڴ�
������
	c(cJson *c):cJSON�ڵ�
����ֵ��
	void
*/
void cJSON_Delete(cJSON *c);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_Object
������
	void
����ֵ��
	cJson_Object ���͵Ľڵ�ָ�롣
*/
cJSON *cJSON_CreateObject(void);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_String
������
	string(char *string)
����ֵ��
	cJSON_String ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateString(const char *string);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_Number
������
	num(double)
����ֵ��
	cJSON_Number ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateNumber(double num);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_Array
������
	void
����ֵ��
	cJSON_Array ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateArray(void);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_Bool
������
	void
����ֵ��
	cJson_Bool ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateBool(int b);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_True
������
	void
����ֵ��
	cJSON_True ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateTrue(void);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_False
������
	void
����ֵ��
	cJSON_False ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateFalse(void);

/*
�������ܣ�
	����һ��cJSON�ڵ㣬�����ýڵ�����ΪcJSON_Null
������
	void
����ֵ��
	cJSON_Null ���͵Ľڵ�ָ��
*/
cJSON *cJSON_CreateNull(void);


/*
�������ܣ�
	��item�ڵ���������Ϊstring��
	���object�ڵ�û���ӽڵ㣬�ͽ�item����Ϊobject�ڵ���ӽڵ�
	����item��ӵ�object->child����β������Ϊobject�ӽڵ���ֵܽڵ�
������
	object(cJSON *):����ӽڵ�
	string(char *)��Ҫ��ӵĽڵ������
	item(cJSON *)��Ҫ��ӵĽڵ�
����ֵ��
	void
*/
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);

/*
�������ܣ�
	��item��ӵ�array�ڵ�
������
	array(cJSON *)������ӽڵ�
	item(cJSON *)��Ҫ��ӵĽڵ�
����ֵ��
	void
*/
void cJSON_AddItemToArray(cJSON *array, cJSON *item);

/*
�������ܣ�
	��δ֪
������
	const char *value��
	const char *return_parse_end��
	int require_null_terminated��
����ֵ��
	cJSON*��
*/
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/*
�������ܣ�
	����һ���µ�cJSON�ڵ㣬��������ɹ���ʹ��0���г�ʼ��
������
	void
����ֵ��
	cJSON*���½ڵ��ָ��
*/
cJSON *cJSON_New_Item();

/*
�������ܣ�
	�Ե�ǰָ���������
������
	const char *in
����ֵ��
	const char*
*/
const char *skip(const char *in);
char *cJSON_GetEndPosition(void);

/*
�������ܣ�
	��value����item�ڵ㣬�����л��ж�value�����ͣ�������Ϊ��Ӧ���͵Ľڵ�
������
	cJSON *item�������cJSON�ڵ�ָ��
	const char *value����������ֵ��ָ��
����ֵ��
	const char*��NULL
*/
const char *parse_value(cJSON *item, const char *value);

/*
�������ܣ�
	��str�����ݽ��н�����������item�ڵ㣬item�ڵ�����ΪcJSON_String����
������
	cJSON *item�������cJSON�ڵ�ָ��
	const char *str�����������ַ���
����ֵ��
	const char*�����������ַ����г�ȥ�׸��ַ����ַ���
*/
const char* parse_string(cJSON *item, const char *str);

/*
�������ܣ�
	��num�����ݽ��н����������ֵĸ���ֵ�洢��item�ڵ��valueDouble������ֵ�洢��item�ڵ��valueInt������ȡ����������item�ڵ�����ΪcJSON_Number����
������
	cJSON *item�������cJSON�ڵ�ָ��
	const char *num�������cJSON�ڵ����ֵ�ָ��
����ֵ��
	const char*�������cJSON�ڵ����ֵ�ָ��
*/
const char* parse_number(cJSON *item, const char *num);

/*
�������ܣ�
	�������飬�������е�ÿ��ֵ��Ϊitem���ӽڵ�
������
	cJSON *item�������cJSON�ڵ�ָ��
	const char *value������������ָ��
����ֵ��
	const char*��NULL
*/
const char* parse_Array(cJSON *item, const char *value);

/*
�������ܣ�
	����Object����ֵ�ԣ�
������
	cJSON *item�������cJSON�ڵ�ָ��
	const char *value������ļ�ֵ�Ե�ָ��
����ֵ��
	const char*��NULL
*/
const char* parse_Object(cJSON *item, const char *value);

/*
�������ܣ�
	����str�е����ݵ�һ���µ�char�ڵ��У������ظýڵ��ָ��
������
	const char *str�������Ƶ��ַ���
����ֵ��
	char*���µ�char�ڵ��ָ��
*/
char *cJSON_strdup(const char *str);

/*
�������ܣ�
	��ѯitem�ڵ��е�valueDouble��ֵ�������ַ�������ʽ����
������
	cJSON *item����ѯ��cJSON�ڵ�
����ֵ��
	char*��ʹ��item�ڵ���valueDouble��ʾ���ַ���
*/
char *print_number(cJSON *item);

/*
�������ܣ�
	��item�ڵ��е��ַ������ݽ��н����������ַ�������ʽ����
������
	cJSON *item����������item�ڵ�
����ֵ��
	char *�������󱣴���item�ڵ��е��ַ������ݵ��ַ���
*/
char *print_string(cJSON *item);

//["OSCAR",123,XXX]

/*
�������ܣ�
	����item�ڵ��е�array����
������
	cJSON *item����������item�ڵ�
	int depth��item���������ӽڵ�����
	int fmt����δ֪
����ֵ��
	char *���������İ���item�ڵ��е�array���ݵ��ַ���
*/
char *print_array(cJSON *item, int depth, int fmt);

/*
�������ܣ�
	����item�ڵ��е�object���ݣ������ַ�������ʽ����
������
	cJSON *item����������item�ڵ�
	int depth��item���������ӽڵ�����
	int fmt��?δ֪
����ֵ��
	char *���������İ���item�ڵ��е�object���ݵ��ַ���
*/
char *print_object(cJSON *item, int depth, int fmt);

/*
�������ܣ�
	����item�ڵ�����ͣ��������ݽ��н����������ַ�������ʽ����
������
	cJSON *item����������item�ڵ�
	int depth��item���������ӽڵ�����
	int fmt����δ֪
����ֵ��
	char *���������İ���item�ڵ������ݵ��ַ���
*/
char *print_value(cJSON *item,int depth, int fmt);


/*
�������ܣ�
	��item��array��ɾ����������item�ڵ�
������
	cJSON *array����Ѱ�ҵ�array�ڵ�
	int which��item����array�е�λ��
����ֵ��
	cJSON *��item�ڵ�
*/
cJSON *cJSON_DetachItemFromArray(cJSON *array, int which);

/*
�������ܣ�
	��array��ɾ��item�ڵ�
������
	cJSON *array����Ѱ�ҵ�array�ڵ�
	int which��item����array�е�λ��
����ֵ��
	void
*/
void cJSON_DeleteItemFromArray(cJSON *array, int which);

/*
�������ܣ�

������

����ֵ��

*/
int cJSON_strcasecmp(const char *s1, const char *s2); //strcmp()

/*
�������ܣ�
	��item�ڵ�object�ڵ����Ƴ���������item�ڵ�
������
	cJSON *object����Ѱ�ҵ�object�ڵ�
	const char *string����Ѱ�ҵļ�ֵ�Եļ�
����ֵ��
	cJSON*�����ص�item�ڵ�
*/
cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string);

/*
�������ܣ�
	��item�ڵ�object�ڵ���ɾ��
������
	cJSON *object����Ѱ�ҵ�object�ڵ�
	const char *string����ɾ���ļ�ֵ�Եļ�
����ֵ��
	void
*/
void cJSON_DeleteItemFromObject(cJSON *object, int which);

/*
�������ܣ�
	��array�ڵ��е�ָ��λ�ò���һ��item�ڵ�
������
	cJSON *array���������array�ڵ�
	int which������λ��
	cJSON *newItem���������item�ڵ�
����ֵ��
	void
*/
void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newItem);

/*
�������ܣ�
	��array�ڵ��е�ָ��λ�ò���item�ڵ�����滻
������
	cJSON *array���������array�ڵ�
	int which���滻λ��
	cJSON *newItem�����滻��item�ڵ�
����ֵ��
	void
*/
void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newItem);

/*
�������ܣ�
	��object�ڵ��е�ָ��λ�ò���item�ڵ�����滻
������
	cJSON *object���������object�ڵ�
	int which���滻λ��
	cJSON *newItem�����滻��item�ڵ�
����ֵ��

*/
void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newItem);

#endif // !_CJSON_H_
