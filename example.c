#include <stdio.h>
#include "cJSON.h"

int main(void)
{
	cJSON *root = cJSON_CreateObject();		// ����һ��cJSON�ڵ�
	cJSON *node1 = cJSON_CreateObject();	// ����һ��cJSON�ڵ�
	cJSON *node2 = cJSON_CreateObject();	// ����һ��cJSON�ڵ�
	cJSON *node3 = cJSON_CreateObject();	// ����һ��cJSON�ڵ�

	cJSON_AddFalseToObject(node1, "node1_key1");	// ����һ��stringֵΪnode1_key1��cJSON_False�ڵ㣬����ӵ�object�ڵ�
	cJSON_AddTrueToObject(node1, "node1_key2");		// ����һ��stringֵΪnode1_key2��cJSON_True�ڵ�,����ӵ�object�ڵ�

	cJSON_AddStringToObject(node2, "node2_key1", "node2_value1");	// ����һ��stringֵΪnode2_key1��cJSON_String�ڵ�,����ӵ�object�ڵ�
	cJSON_AddStringToObject(node2, "node2_key2", "node2_value2");	// ����һ��stringֵΪnode2_key2��cJSON_String�ڵ�,����ӵ�object�ڵ�

	cJSON_AddNumberToObject(node3, "node3_key1", 1000);	// ����һ��stringֵΪ1000��cJSON_Number�ڵ�,����ӵ�object�ڵ�
	cJSON_AddNumberToObject(node3, "node3_key2", 2000);	// ����һ��stringֵΪ2000��cJSON_Number�ڵ�,����ӵ�object�ڵ�

	cJSON_AddItemToObject(root, "root_node1", node1);	// ��item�ڵ���������Ϊroot_node1�����root�ڵ�û���ӽڵ㣬�ͽ�node1����Ϊroot�ڵ���ӽڵ�,����node1��ӵ�root->child����β������Ϊroot�ӽڵ���ֵܽڵ�
	cJSON_AddItemToObject(root, "root_node2", node2);	// ��item�ڵ���������Ϊroot_node2�����root�ڵ�û���ӽڵ㣬�ͽ�node2����Ϊroot�ڵ���ӽڵ�,����node2��ӵ�root->child����β������Ϊroot�ӽڵ���ֵܽڵ�

	cJSON_AddItemToObject(node1, "node1_node3", node3);	// ��item�ڵ���������Ϊroot_node3�����root�ڵ�û���ӽڵ㣬�ͽ�node3����Ϊroot�ڵ���ӽڵ�,����node3��ӵ�root->child����β������Ϊroot�ӽڵ���ֵܽڵ�


	char *buf = cJSON_Print(root);	// ��item�ڵ㿪ʼ�ݹ���������ڵ���ת��Ϊ�ַ���
	printf("json:\n%s\n", buf);

	cJSON *parse_node = cJSON_Parse(buf);	// ����json���ݣ�����������json����

	cJSON_Delete(root);	// �Ӹ��ڵ㿪ʼ�ݹ�ɾ��json�������ӽڵ㣬�ͷ��ڴ�
	cJSON_Delete(parse_node);	// �Ӹ��ڵ㿪ʼ�ݹ�ɾ��json�������ӽڵ㣬�ͷ��ڴ�
	free(buf);
	return 0;
}

