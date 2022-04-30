#include <stdio.h>
#include "cJSON.h"

int main(void)
{
	cJSON *root = cJSON_CreateObject();		// 创建一个cJSON节点
	cJSON *node1 = cJSON_CreateObject();	// 创建一个cJSON节点
	cJSON *node2 = cJSON_CreateObject();	// 创建一个cJSON节点
	cJSON *node3 = cJSON_CreateObject();	// 创建一个cJSON节点

	cJSON_AddFalseToObject(node1, "node1_key1");	// 创建一个string值为node1_key1的cJSON_False节点，并添加到object节点
	cJSON_AddTrueToObject(node1, "node1_key2");		// 创建一个string值为node1_key2的cJSON_True节点,并添加到object节点

	cJSON_AddStringToObject(node2, "node2_key1", "node2_value1");	// 创建一个string值为node2_key1的cJSON_String节点,并添加到object节点
	cJSON_AddStringToObject(node2, "node2_key2", "node2_value2");	// 创建一个string值为node2_key2的cJSON_String节点,并添加到object节点

	cJSON_AddNumberToObject(node3, "node3_key1", 1000);	// 创建一个string值为1000的cJSON_Number节点,并添加到object节点
	cJSON_AddNumberToObject(node3, "node3_key2", 2000);	// 创建一个string值为2000的cJSON_Number节点,并添加到object节点

	cJSON_AddItemToObject(root, "root_node1", node1);	// 将item节点名称设置为root_node1，如果root节点没有子节点，就将node1设置为root节点的子节点,否则将node1添加到root->child链表尾部，成为root子节点的兄弟节点
	cJSON_AddItemToObject(root, "root_node2", node2);	// 将item节点名称设置为root_node2，如果root节点没有子节点，就将node2设置为root节点的子节点,否则将node2添加到root->child链表尾部，成为root子节点的兄弟节点

	cJSON_AddItemToObject(node1, "node1_node3", node3);	// 将item节点名称设置为root_node3，如果root节点没有子节点，就将node3设置为root节点的子节点,否则将node3添加到root->child链表尾部，成为root子节点的兄弟节点


	char *buf = cJSON_Print(root);	// 从item节点开始递归遍历，将节点树转换为字符串
	printf("json:\n%s\n", buf);

	cJSON *parse_node = cJSON_Parse(buf);	// 解析json数据，将数据填入json树中

	cJSON_Delete(root);	// 从根节点开始递归删除json树各个子节点，释放内存
	cJSON_Delete(parse_node);	// 从根节点开始递归删除json树各个子节点，释放内存
	free(buf);
	return 0;
}

