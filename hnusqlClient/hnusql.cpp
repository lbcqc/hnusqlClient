#include <iostream>
#include <cstring>
#include <string>
#include <conio.h>
#include <mysql.h>
#include <iomanip>
using namespace std;

#define MAX(a,b) {a>b? a:b}

//打印帮助信息
void helpInfo(void)
{
	cout << "========help info=======\n"
		<< "NULL\n";
	exit(0);
}

//打印版本信息
void versionInfo(void)
{
	cout << "hnusql  Ver 14.14 Distrib 5.7.18, for Win32 (AMD64)" << endl;
	exit(0);
}

//数据库时所用连接信息
class HnusqlConnectInfo 
{
public:
	//连接MYSQL数据库参数，接收5个变量
	char user[20];
	char password[20];
	char port[20];
	char database[20];
	char host[20];

	HnusqlConnectInfo():user(""),password(""),port("3306"),database(""),host("localhost") {}

private:

}cntinfo;
//参数类型信息
enum ArgsType
{
	ARG,COMMAND
};



//通过名称前缀索引地址列表
class CntinfoList 
{
public:
	char *pre;
	void *addr;
	ArgsType argsType;
}cntinfoList[] = {
	//参数列表-可接收的10中连接信息参数前缀
	{ "-u", cntinfo.user, ARG },
	{ "user", cntinfo.user, ARG },
	{ "-p",cntinfo.password, ARG },
	{ "password",cntinfo.password, ARG },
	{ "-P",cntinfo.port, ARG },
	{ "port",cntinfo.port, ARG },
	{ "-D",cntinfo.database , ARG },
	{ "--database",cntinfo.database, ARG },
	{ "-h",cntinfo.host, ARG },
	{ "host",cntinfo.host, ARG },
	
	//参数列表-可接受的两种命令参数
	{ "--version", &versionInfo, COMMAND },
	{ "--help", &helpInfo, COMMAND },
};

//读取命令行参数
bool read_args(int argc, char* args[], HnusqlConnectInfo& cntinfo);
//命令行匹配参数
bool matchOption(const char* str);
//打印表数据
void print_table_data(MYSQL_RES* result);

int main(int argc, char* args[])
{
	/*连接程序-begin*/
	//此处处理命令行参数，执行初始化及连接操作
	if (argc <= 1) {
		cout << "Please input user and password.\n"
			<<"You also can use \"hnusql -h\" to get more arguments\n.";
		exit(1);
	}

	if (!read_args(argc, args, cntinfo)) 
	{
		cout << "arguments error.\n";
		exit(1);
	}

	MYSQL m_sqlCon; //MYSQL连接信息变量
	mysql_init(&m_sqlCon); //初始化MYSQL数据库对象
	if (!mysql_real_connect(&m_sqlCon, cntinfo.host, cntinfo.user,
		cntinfo.password, cntinfo.database, atoi(cntinfo.port), NULL, 0))
	{
		cout << "HNUSQL Connect Error" << endl;
		return 0;
	}
	else
	{
		cout << "HNUSQL Connect Success" << endl;
		mysql_query(&m_sqlCon, "set names 'gbk'"); //设置数据库字符格式，解决中文乱码问题  
	}
	/*连接程序-end*/

	/*读取命令并执行-begin*/
	//此处读取sql语句并执行
	MYSQL_RES* result;
	
	for (;;)
	{
		string buffer;
		string tmpbuf;
		getline(cin,buffer);
		int status = mysql_query(&m_sqlCon, buffer.c_str());
		if (status)
		{
			cout << "Could not execute statement(s)";
			mysql_close(&m_sqlCon);
			exit(0);
		}
		else 
		{
			//MYSQL查询student表，将结果保存到SQLSERVER数据库中
			
			result = mysql_store_result(&m_sqlCon);
			print_table_data(result);
		}
	}
	/*读取命令并执行-end*/
	return 0;
}

//读取命令行参数
bool read_args(int argc, char* args[], HnusqlConnectInfo& cntinfo)
{
	if (argc <= 1)
		return false;
	char *tmpstr; //临时字符串
	//char prestr[50]; //前缀
	bool errorFlag;
	bool pswFlag = false; //参数中是否指定密码选项
	//进行参数的赋值
	for (int i = 1; i < argc; i++)
	{
		tmpstr = args[i];
		//将该参数和参数信息列表中的信息进行匹配
		errorFlag = true;
		for (auto argsinfolist : cntinfoList)
		{
			if (strstr(tmpstr, argsinfolist.pre) == tmpstr) {
				if (strcmp(argsinfolist.pre, "-p") == 0 || strcmp(argsinfolist.pre, "password") == 0)
					pswFlag = true; //选择中存在密码选项
				if (argsinfolist.argsType == COMMAND) 
				{
					if (argsinfolist.addr != NULL) (*((void (*)())argsinfolist.addr))(); //执行函数指针
					exit(0); //执行指定命令之后结束程序
				}
				int len = strlen(argsinfolist.pre);
				if (tmpstr[len] == '\0') 
				{
					if (i+1 < argc && !matchOption(args[i+1])) //如果下一个参数不是选项的话，则认为是该参数的内容
						strcpy((char*)argsinfolist.addr, args[++i]);
				}
				else
					strcpy((char*)argsinfolist.addr, tmpstr + len);
				
				errorFlag = false; //匹配成功-设置标志位
				break;
			}
		}
		if(errorFlag)
			return false; //有某一个参数和列表中的信息均不匹配
	}
	//检测参数的合理性
	if (cntinfo.user[0] == '\0')
	{
		cout << "please input user name" << endl;
		exit(1);
	}
	if (!pswFlag) 
	{
		cout << "please input password option" << endl;
		exit(1);
	}
	if (cntinfo.password[0] == '\0')
	{
		cout << "Enter password:";
		int j;
		for (j = 0; tmpstr[j] = getch(); j++)
		{
			if (tmpstr[j] == '\r')
				break;
			else if (tmpstr[j] == '\b') {
				if (j > 0) { //控制删除底线
					cout << '\b' << ' ' << '\b';
					j--;
				}
				j--;
			}
			else if (j == sizeof(cntinfo.password)-1) { //控制输入上线
				j--;
			}
			else
				cout << "*";
		}
		cout << endl;
		tmpstr[j] = '\0';
		strcpy(cntinfo.password, tmpstr);
	}
	return true;
	
}

//命令行匹配参数
bool matchOption(const char* str)
{
	for (auto argsinfolist : cntinfoList)
	{
		if (strstr(str, argsinfolist.pre) == str) return true;
	}
	return false;
}


//打印表数据
void print_table_data(MYSQL_RES* result)
{
	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	int i,j;
	int num_fields = (int)mysql_num_fields(result); //获取结果列数
	fields = mysql_fetch_fields(result);   //获取列名数组

	//打印第一列边界符号
	cout << "+";
	for (i = 0; i < num_fields; i++) //设置每一列的输出长度
	{
		fields[i].max_length = MAX(fields[i].name_length, fields[i].max_length);
		cout << "-";
		for (j = 0; j < fields[i].max_length; j++) cout << "-";
		cout << "-+";
	}
	cout << endl;
	
	//打印列名
	cout << "| ";
	for (i = 0; i < num_fields; i++)       //输出列名
		cout << setiosflags(ios::left) << setw(fields[i].max_length) << fields[i].name << " | ";
	cout << endl;

	//打印边界列符号
	cout << "+";
	for (i = 0; i < num_fields; i++) //设置每一列的输出长度
	{
		cout << "-";
		for (j = 0; j < fields[i].max_length; j++) cout << "-";
		cout << "-+";
	}
	cout << endl;

	while ((row = mysql_fetch_row(result)))
	{
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(result);
		cout << "| ";
		for (i = 0; i < num_fields; i++)
		{
			cout << setiosflags(ios::left) << setw(fields[i].max_length) << row[i] << " | ";
		}
		cout << endl;
	}
	
	//打印边界列符号
	cout << "+";
	for (i = 0; i < num_fields; i++) //设置每一列的输出长度
	{
		cout << "-";
		for (j = 0; j < fields[i].max_length; j++) cout << "-";
		cout << "-+";
	}
	cout << endl;

	return;
}