#include <iostream>
#include <cstring>
#include <string>
#include <conio.h>
#include <mysql.h>
#include <iomanip>
using namespace std;

//返回更大的数
#define MAX(a,b) {a>b? a:b}
//去掉行首行尾的空格
void trim(string &s);

void helpInfo(void); //打印帮助信息
void versionInfo(void); //打印版本信息
void cntInfo(MYSQL hnusql); //打印连接成功的时的信息

void default_func(MYSQL hnusql, const char* buffer); //缺省函数
void use(MYSQL hnusql, const char* buffer); //选择数据库语句
void quit(MYSQL hnusql, const char* buffer); //退出语句
void select(MYSQL hnusql, const char* buffer); //查询语句

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
	//命令行类型
	ARG,COMMAND,
	//sql语句类型
	SIMPLE_CONMAND, //简单命令，比如use,quit等
	CONMAND_TABLE_RESULT, //返回表结果的命令，比如select
	CONMAND_INFO_RESULT,  //只有返回信息而不是表数据的命令，比如create database
};



//通过名称前缀索引地址列表
class InfoList
{
public:
	char *pre;
	void *addr;
	ArgsType argsType;
}cntinfoList[] = {
	//参数列表-可接收的10中连接信息参数前缀
	{ "-u", cntinfo.user, ARG },{ "--user=", cntinfo.user, ARG },
	{ "-p",cntinfo.password, ARG },{ "--password=",cntinfo.password, ARG },
	{ "-P",cntinfo.port, ARG },{ "--port=",cntinfo.port, ARG },
	{ "-D",cntinfo.database , ARG },{ "--database=",cntinfo.database, ARG },
	{ "-h",cntinfo.host, ARG },{ "--host=",cntinfo.host, ARG },
	
	//参数列表-可接受的两种命令参数
	{ "-V", &versionInfo, COMMAND },{ "--version", &versionInfo, COMMAND },
	{ "-?", &helpInfo, COMMAND },{ "--help", &helpInfo, COMMAND },
},sqlList[] = {
	{"use", &default_func, SIMPLE_CONMAND }, //缺省函数，尚未提供处理
	{"quit", &quit, SIMPLE_CONMAND },
	{"select", NULL, CONMAND_TABLE_RESULT },
};

//读取命令行参数
bool read_args(int argc, char* args[], HnusqlConnectInfo& cntinfo);
//命令行匹配参数
bool matchOption(const char* str);
//sql语法匹配参数
bool matchSqlOption(const char* str);
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
		cout << "HNUSQL Connect Failed." << endl;
		return 0;
	}
	else
	{
		cntInfo(m_sqlCon); //连接成功后打印信息
		mysql_query(&m_sqlCon, "set names 'gbk'"); //设置数据库字符格式，解决中文乱码问题
	}
	/*连接程序-end*/

	/*读取命令并执行-begin*/
	//此处读取sql语句并执行

	char default_prompt[20], prompt[20]; //提示字符格式
	string buffer;
	string tmpbuf;
	char delimiter = ';';
	strcpy(default_prompt, getenv("MYSQL_PS1") ? getenv("MYSQL_PS1") : "hnusql> ");


	bool firstLine = true;
	for (;;)
	{
		//打印提示符号
		if (firstLine) {
			strcpy(prompt, default_prompt);
			buffer.clear();
		}
		else
			strcpy(prompt, "     -> ");
		cout << prompt;

		//获取数据
		getline(cin, tmpbuf);
		//去掉首尾的空格
		trim(tmpbuf);
		if (tmpbuf.empty()) continue;
		buffer.append(tmpbuf);
		buffer.append(" ");

		for (auto sqllist : sqlList)
		{
			//存在匹配
			if (strstr(buffer.c_str(), sqllist.pre) == buffer.c_str())
			{
				if (firstLine && sqllist.addr != NULL && sqllist.argsType == SIMPLE_CONMAND)
				{
					if (buffer.find(delimiter) != -1)
						buffer.erase(buffer.find(delimiter));
					buffer.erase(buffer.find_last_not_of(" ") + 1);
					(*((void(*)(MYSQL, const char*))sqllist.addr))(m_sqlCon, buffer.c_str());
				}
				else if (buffer.find(delimiter) != -1) { //存在结束标志;分号
					buffer.erase(buffer.find(delimiter));
					buffer.erase(buffer.find_last_not_of(" ") + 1);
					MYSQL_RES* result;
					if (sqllist.addr != NULL)
					{
						(*((void(*)(MYSQL, const char*))sqllist.addr))(m_sqlCon, buffer.c_str());
					}
					else {
						int status = mysql_query(&m_sqlCon, buffer.c_str());
						if (status)
							cout << "Could not execute the statement(s)." << endl;
						else if (sqllist.argsType == CONMAND_TABLE_RESULT)
						{
							//MYSQL查询student表，将结果保存到SQLSERVER数据库中
							result = mysql_store_result(&m_sqlCon);
							print_table_data(result);
						}
						else
							cout << "ok." << endl;
					}

					firstLine = true;
				}
				else
					firstLine = false;
				break;
			}
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
				if (strcmp(argsinfolist.pre, "-p") == 0 || strcmp(argsinfolist.pre, "--password=") == 0)
					pswFlag = true; //选择中存在密码选项

				if (argsinfolist.argsType == COMMAND) 
				{
					if (argsinfolist.addr != NULL) (*((void (*)())argsinfolist.addr))(); //执行函数指针
					exit(0); //执行指定命令之后结束程序
				}
				int len = strlen(argsinfolist.pre);
				if (tmpstr[len] == '\0') 
				{
					if (i + 1 < argc && !matchOption(args[i + 1])) //如果下一个参数不是选项的话，则认为是该参数的内容
					{
						if (strcmp(argsinfolist.pre, "-p") == 0 || strcmp(argsinfolist.pre, "--password=") == 0)
							cout << "hnusql: [Warning] Using a password on the command line interface can be insecure." << endl;
						strcpy((char*)argsinfolist.addr, args[++i]);
						
					}
				}
				else
				{
					if (strcmp(argsinfolist.pre, "-p") == 0 || strcmp(argsinfolist.pre, "--password=") == 0)
						cout << "hnusql: [Warning] Using a password on the command line interface can be insecure." << endl;
					strcpy((char*)argsinfolist.addr, tmpstr + len);
				}
				
				errorFlag = false; //匹配成功-设置标志位
				break;
			}
		}
		if (errorFlag) {
			cout << "hnusql: [ERROR] unknown option \'" << tmpstr << "\'" <<endl;
			exit(1); //有某一个参数和列表中的信息均不匹配
		}
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

//sql语法匹配参数
bool matchSqlOption(const char* str)
{
	for (auto sqllist : sqlList)
	{
		if (strstr(str, sqllist.pre) == str) return true;
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

void trim(string &s)
{

	if (!s.empty())
	{
		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
	}
}

void use(MYSQL hnusql, const char* buffer)
{

}
void quit(MYSQL hnusql, const char *buffer)
{
	mysql_close(&hnusql);
	cout << "Bye" << endl;
	exit(0);
}
void select(MYSQL hnusql, const char* sql)
{
	MYSQL_RES* result;
	int status = mysql_query(&hnusql, sql);
	if (status)
	{
		cout << "Could not execute the statement(s)";
	}
	else
	{
		//MYSQL查询student表，将结果保存到SQLSERVER数据库中
		result = mysql_store_result(&hnusql);
		print_table_data(result);
	}
}
//缺省函数
void default_func(MYSQL hnusql, const char* buffer)
{
	cout << "Not supported the sql" << endl;
}


//打印帮助信息
void helpInfo(void)
{
	cout << "hnusql  Ver 14.14 Distrib 5.7.18, for Win32 (AMD64)" << endl;
	cout << endl;

	cout << "Copyright(c) 2000, 2017, Hnu and/or its affiliates.All rights reserved." << endl;
	cout << endl;

	cout << "Usage: hnusql [OPTIONS] [database]" << endl;
	cout << "  -?, --help          Display this help and exit." << endl;
	cout << "  -D, --database=name Database to use." << endl;
	cout << "  -h, --host = name     Connect to host." << endl;
	cout << "  -p, --password[=name]" << endl;
	cout << "                      Password to use when connecting to server. If password is" << endl;
	cout << "                      not given it's asked from the tty." << endl;
	cout << "  -P, --port=#        Port number to use for connection or 0 for default to, in" << endl;
	cout << "                      order of preference, my.cnf, $MYSQL_TCP_PORT, " << endl;
	cout << "                      /etc/services, built-in default (3306)." << endl;
	cout << "  -u, --user=name     User for login if not current user." << endl;
	cout << "  -V, --version       Output version information and exit." << endl;
	cout << endl;
	
	cout << "SQL syntax supported." << endl;
	cout << "  select" << endl;
	cout << endl;
	exit(0);
}


//打印版本信息
void versionInfo(void)
{
	cout << "hnusql  Ver 14.14 Distrib 5.7.18, for Win32 (AMD64)" << endl;
	exit(0);
}

//打印连接成功时的信息
void cntInfo(MYSQL hnusql)
{
	cout << "Welcome to the HnuSQL monitor.  Commands end with \';\'." << endl;
	cout << "Your HnuSQL connection id is " << mysql_thread_id(&hnusql)  << "."<< endl;
	cout << "Server version : "<< hnusql.server_version << "." << endl;
	cout << endl;
	cout << "Copyright(c) 2000, 2017, Hnu and/or its affiliates.All rights reserved." << endl;
	cout << endl;
	cout << "Type 'help;' or '\\h' for help.Type '\\c' to clear the current input statement.\n" << endl;
}