#pragma once

#ifndef _CJSON_H_
#define _CJSON_H_

/*
两个板块：
json解析、节点的增删改查

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

逻辑上的树状结构

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
功能：创建一个string值为name的cJSON_False节点，并添加到object节点
*/
#define cJSON_AddFalseToObject(object, name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateFalse())

/*创建一个string值为name的cJSON_True节点,并添加到object节点*/
#define cJSON_AddTrueToObject(object,name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateTrue())

/*创建一个string值为name的cJSON_Bool/False节点,并添加到object节点*/
#define cJSON_AddBoolToObject(object,name, b) \
	cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))

/*创建一个string值为name的cJSON_NULL节点,并添加到object节点*/
#define cJSON_AddNULLToObject(object,name) \
	cJSON_AddItemToObject(object, name, cJSON_CreateNull())

/*创建一个string值为name的cJSON_Number节点,并添加到object节点*/
#define cJSON_AddNumberToObject(object,name, number) \
	cJSON_AddItemToObject(object, name, cJSON_CreateNumber(number))

/*创建一个string值为name的cJSON_String节点,并添加到object节点*/
#define cJSON_AddStringToObject(object,name, s) \
	cJSON_AddItemToObject(object, name, cJSON_CreateString(s))

#define cJSON_SetIntValue(object, val)\
	((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))	// 如果object不为真，执行(object)->valueInt=(object)->valueDouble=(val)，返回值为val，否则直接返回val

#define cJSON_SetNumberValue(object, val)\
	((object)?((object)->valueInt=(object)->valueDouble=(val)):(val))	// 如果object不为真，执行(object)->valueInt=(object)->valueDouble=(val)，返回值为val，否则直接返回val

typedef struct cJSON
{
	struct cJSON *next, *prev;	// 采用某种树结构存储cJSON节点
	struct cJSON *child;

	/*array以及object类型需要设立子节点*/
	int type;

	char *valueString;
	int valueInt;
	double valueDouble;

	char *string;
}cJSON;

/*常用解析函数*/


/*
函数功能：
	解析json数据，将数据填入json树中
参数：
	value (const char *):char*指针，指向待解析的json数据
返回值：
	json树的根节点
*/
cJSON *cJSON_Parse(const char *value);

/*
函数功能：
	从item节点开始递归遍历，将节点树转换为字符串
参数：
	item(cJSON *item):cJson节点
返回值：
	字符串指针，从item节点解析后的树字符串
注意事项：
	使用该函数后，需要根据返回的char*指针释放内存。
	out = cJSON_Print(cJSON *item)
	prinf("%s",out);
	free(out)
*/
char *cJSON_Print(cJSON *item);

/*
函数功能：
	从根节点开始递归删除json树各个节点，释放内存
参数：
	c(cJson *c):cJSON节点
返回值：
	void
*/
void cJSON_Delete(cJSON *c);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_Object
参数：
	void
返回值：
	cJson_Object 类型的节点指针。
*/
cJSON *cJSON_CreateObject(void);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_String
参数：
	string(char *string)
返回值：
	cJSON_String 类型的节点指针
*/
cJSON *cJSON_CreateString(const char *string);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_Number
参数：
	num(double)
返回值：
	cJSON_Number 类型的节点指针
*/
cJSON *cJSON_CreateNumber(double num);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_Array
参数：
	void
返回值：
	cJSON_Array 类型的节点指针
*/
cJSON *cJSON_CreateArray(void);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_Bool
参数：
	void
返回值：
	cJson_Bool 类型的节点指针
*/
cJSON *cJSON_CreateBool(int b);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_True
参数：
	void
返回值：
	cJSON_True 类型的节点指针
*/
cJSON *cJSON_CreateTrue(void);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_False
参数：
	void
返回值：
	cJSON_False 类型的节点指针
*/
cJSON *cJSON_CreateFalse(void);

/*
函数功能：
	创建一个cJSON节点，并设置节点类型为cJSON_Null
参数：
	void
返回值：
	cJSON_Null 类型的节点指针
*/
cJSON *cJSON_CreateNull(void);


/*
函数功能：
	将item节点名称设置为string，
	如果object节点没有子节点，就将item设置为object节点的子节点
	否则将item添加到object->child链表尾部，成为object子节点的兄弟节点
参数：
	object(cJSON *):被添加节点
	string(char *)：要添加的节点的名称
	item(cJSON *)：要添加的节点
返回值：
	void
*/
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);

/*
函数功能：
	将item添加到array节点
参数：
	array(cJSON *)：被添加节点
	item(cJSON *)：要添加的节点
返回值：
	void
*/
void cJSON_AddItemToArray(cJSON *array, cJSON *item);

/*
函数功能：
	？未知
参数：
	const char *value：
	const char *return_parse_end：
	int require_null_terminated：
返回值：
	cJSON*：
*/
cJSON *cJSON_ParseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/*
函数功能：
	创建一个新的cJSON节点，如果创建成功，使用0进行初始化
参数：
	void
返回值：
	cJSON*：新节点的指针
*/
cJSON *cJSON_New_Item();

/*
函数功能：
	对当前指针进行自增
参数：
	const char *in
返回值：
	const char*
*/
const char *skip(const char *in);
char *cJSON_GetEndPosition(void);

/*
函数功能：
	将value存入item节点，过程中会判断value的类型，并设置为对应类型的节点
参数：
	cJSON *item：存入的cJSON节点指针
	const char *value：待解析的值的指针
返回值：
	const char*：NULL
*/
const char *parse_value(cJSON *item, const char *value);

/*
函数功能：
	将str的内容进行解析，并存入item节点，item节点设置为cJSON_String类型
参数：
	cJSON *item：存入的cJSON节点指针
	const char *str：待解析的字符串
返回值：
	const char*：待解析的字符串中除去首个字符的字符串
*/
const char* parse_string(cJSON *item, const char *str);

/*
函数功能：
	将num的内容进行解析，将数字的浮点值存储入item节点的valueDouble，整数值存储入item节点的valueInt（向下取整），并将item节点设置为cJSON_Number类型
参数：
	cJSON *item：存入的cJSON节点指针
	const char *num：存入的cJSON节点数字的指针
返回值：
	const char*：存入的cJSON节点数字的指针
*/
const char* parse_number(cJSON *item, const char *num);

/*
函数功能：
	解析数组，并将其中的每个值存为item的子节点
参数：
	cJSON *item：存入的cJSON节点指针
	const char *value：存入的数组的指针
返回值：
	const char*：NULL
*/
const char* parse_Array(cJSON *item, const char *value);

/*
函数功能：
	解析Object（键值对）
参数：
	cJSON *item：存入的cJSON节点指针
	const char *value：存入的键值对的指针
返回值：
	const char*：NULL
*/
const char* parse_Object(cJSON *item, const char *value);

/*
函数功能：
	复制str中的内容到一个新的char节点中，并返回该节点的指针
参数：
	const char *str：被复制的字符串
返回值：
	char*：新的char节点的指针
*/
char *cJSON_strdup(const char *str);

/*
函数功能：
	查询item节点中的valueDouble的值，并以字符串的形式返回
参数：
	cJSON *item：查询的cJSON节点
返回值：
	char*：使用item节点中valueDouble表示的字符串
*/
char *print_number(cJSON *item);

/*
函数功能：
	将item节点中的字符串内容进行解析，并以字符串的形式返回
参数：
	cJSON *item：待解析的item节点
返回值：
	char *：解析后保存着item节点中的字符串内容的字符串
*/
char *print_string(cJSON *item);

//["OSCAR",123,XXX]

/*
函数功能：
	解析item节点中的array内容
参数：
	cJSON *item：待解析的item节点
	int depth：item所包含的子节点的深度
	int fmt：？未知
返回值：
	char *：解析出的包含item节点中的array内容的字符串
*/
char *print_array(cJSON *item, int depth, int fmt);

/*
函数功能：
	解析item节点中的object内容，并以字符串的形式返回
参数：
	cJSON *item：待解析的item节点
	int depth：item所包含的子节点的深度
	int fmt：?未知
返回值：
	char *：解析出的包含item节点中的object内容的字符串
*/
char *print_object(cJSON *item, int depth, int fmt);

/*
函数功能：
	按照item节点的类型，对其内容进行解析，并以字符串的形式返回
参数：
	cJSON *item：待解析的item节点
	int depth：item所包含的子节点的深度
	int fmt：？未知
返回值：
	char *：解析出的包含item节点中内容的字符串
*/
char *print_value(cJSON *item,int depth, int fmt);


/*
函数功能：
	将item从array中删除，并返回item节点
参数：
	cJSON *array：待寻找的array节点
	int which：item所在array中的位置
返回值：
	cJSON *：item节点
*/
cJSON *cJSON_DetachItemFromArray(cJSON *array, int which);

/*
函数功能：
	从array中删除item节点
参数：
	cJSON *array：待寻找的array节点
	int which：item所在array中的位置
返回值：
	void
*/
void cJSON_DeleteItemFromArray(cJSON *array, int which);

/*
函数功能：

参数：

返回值：

*/
int cJSON_strcasecmp(const char *s1, const char *s2); //strcmp()

/*
函数功能：
	将item节点object节点中移除，并返回item节点
参数：
	cJSON *object：待寻找的object节点
	const char *string：待寻找的键值对的键
返回值：
	cJSON*：返回的item节点
*/
cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string);

/*
函数功能：
	将item节点object节点中删除
参数：
	cJSON *object：待寻找的object节点
	const char *string：待删除的键值对的键
返回值：
	void
*/
void cJSON_DeleteItemFromObject(cJSON *object, int which);

/*
函数功能：
	向array节点中的指定位置插入一个item节点
参数：
	cJSON *array：待插入的array节点
	int which：插入位置
	cJSON *newItem：待插入的item节点
返回值：
	void
*/
void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newItem);

/*
函数功能：
	将array节点中的指定位置采用item节点进行替换
参数：
	cJSON *array：待插入的array节点
	int which：替换位置
	cJSON *newItem：待替换的item节点
返回值：
	void
*/
void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newItem);

/*
函数功能：
	将object节点中的指定位置采用item节点进行替换
参数：
	cJSON *object：待插入的object节点
	int which：替换位置
	cJSON *newItem：待替换的item节点
返回值：

*/
void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newItem);

#endif // !_CJSON_H_
