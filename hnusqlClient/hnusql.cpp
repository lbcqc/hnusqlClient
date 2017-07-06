#include <iostream>
#include <string>
#include <mysql.h>
using namespace std;
int main(int argc, char* args[])
{
	/*���ӳ���-begin*/
	//�˴����������в�����ִ�г�ʼ�������Ӳ���
	//MYSQL����
	MYSQL m_sqlCon;

	//��ʼ��MYSQL���ݿ����
	mysql_init(&m_sqlCon);
	//����MYSQL���ݿ�
	if (!mysql_real_connect(&m_sqlCon, "localhost", "root",
		"123456", "school", 3306, NULL, 0))
	{
		cout << "MYSQL���ݿ�����ʧ��" << endl;
		return 0;
	}
	else
	{
		cout << "MYSQL���ݿ����ӳɹ�" << endl;
		mysql_query(&m_sqlCon, "set names 'gbk'"); //�������ݿ��ַ���ʽ�����������������  
	}
	/*���ӳ���-end*/

	/*��ȡ���ִ��-begin*/
	//�˴���ȡsql��䲢ִ��
	MYSQL_RES* result;
	MYSQL_ROW row;
	MYSQL_FIELD *fields;
	int i;
	for (;;)
	{
		string buffer;
		string tmpbuf;
		getline(cin,buffer);
		cout << buffer << endl;
		int status = mysql_query(&m_sqlCon, buffer.c_str());
		if (status)
		{
			printf("Could not execute statement(s)");
			mysql_close(&m_sqlCon);
			exit(0);
		}
		else 
		{
			//MYSQL��ѯstudent����������浽SQLSERVER���ݿ���
			unsigned int num_fields;
			result = mysql_store_result(&m_sqlCon);

			num_fields = mysql_num_fields(result); //��ȡ�������
			fields = mysql_fetch_fields(result);   //��ȡ��������
			for (i = 0; i < num_fields; i++)       //�������
				printf("%s\t", fields[i].name);
			printf("\n");
			while ((row = mysql_fetch_row(result)))
			{
				unsigned long *lengths;
				lengths = mysql_fetch_lengths(result);
				for (i = 0; i < num_fields; i++)
				{
					printf("[%s] ", row[i] ? row[i] : "NULL");
				}
				printf("\n");
			}
			printf("\n");
		}
	}
	/*��ȡ���ִ��-end*/
	return 0;
}