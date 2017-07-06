#include <iostream>
#include <string>
#include <mysql.h>
using namespace std;
int main(int argc, char* args[])
{
	/*连接程序-begin*/
	//此处处理命令行参数，执行初始化及连接操作
	//MYSQL变量
	MYSQL m_sqlCon;

	//初始化MYSQL数据库对象
	mysql_init(&m_sqlCon);
	//连接MYSQL数据库
	if (!mysql_real_connect(&m_sqlCon, "localhost", "root",
		"123456", "school", 3306, NULL, 0))
	{
		cout << "MYSQL数据库连接失败" << endl;
		return 0;
	}
	else
	{
		cout << "MYSQL数据库连接成功" << endl;
		mysql_query(&m_sqlCon, "set names 'gbk'"); //设置数据库字符格式，解决中文乱码问题  
	}
	/*连接程序-end*/

	/*读取命令并执行-begin*/
	//此处读取sql语句并执行
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
			//MYSQL查询student表，将结果保存到SQLSERVER数据库中
			unsigned int num_fields;
			result = mysql_store_result(&m_sqlCon);

			num_fields = mysql_num_fields(result); //获取结果列数
			fields = mysql_fetch_fields(result);   //获取列名数组
			for (i = 0; i < num_fields; i++)       //输出列名
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
	/*读取命令并执行-end*/
	return 0;
}