#include <iostream>
#include <cstring>
#include <string>
#include <conio.h>
#include <mysql.h>
#include <iomanip>
using namespace std;

//���ظ������
#define MAX(a,b) {a>b? a:b}


void helpInfo(void); //��ӡ������Ϣ
void versionInfo(void); //��ӡ�汾��Ϣ
void cntInfo(MYSQL hnusql); //��ӡ���ӳɹ���ʱ����Ϣ

//���ݿ�ʱ����������Ϣ
class HnusqlConnectInfo 
{
public:
	//����MYSQL���ݿ����������5������
	char user[20];
	char password[20];
	char port[20];
	char database[20];
	char host[20];

	HnusqlConnectInfo():user(""),password(""),port("3306"),database(""),host("localhost") {}

private:

}cntinfo;
//����������Ϣ
enum ArgsType
{
	ARG,COMMAND
};



//ͨ������ǰ׺������ַ�б�
class CntinfoList 
{
public:
	char *pre;
	void *addr;
	ArgsType argsType;
}cntinfoList[] = {
	//�����б�-�ɽ��յ�10��������Ϣ����ǰ׺
	{ "-u", cntinfo.user, ARG },
	{ "--user=", cntinfo.user, ARG },
	{ "-p",cntinfo.password, ARG },
	{ "--password=",cntinfo.password, ARG },
	{ "-P",cntinfo.port, ARG },
	{ "--port=",cntinfo.port, ARG },
	{ "-D",cntinfo.database , ARG },
	{ "--database=",cntinfo.database, ARG },
	{ "-h",cntinfo.host, ARG },
	{ "--host=",cntinfo.host, ARG },
	
	//�����б�-�ɽ��ܵ������������
	{ "-V", &versionInfo, COMMAND },
	{ "--version", &versionInfo, COMMAND },
	{ "-?", &helpInfo, COMMAND },
	{ "--help", &helpInfo, COMMAND },
};

//��ȡ�����в���
bool read_args(int argc, char* args[], HnusqlConnectInfo& cntinfo);
//������ƥ�����
bool matchOption(const char* str);
//��ӡ������
void print_table_data(MYSQL_RES* result);

int main(int argc, char* args[])
{
	/*���ӳ���-begin*/
	//�˴����������в�����ִ�г�ʼ�������Ӳ���
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

	MYSQL m_sqlCon; //MYSQL������Ϣ����
	mysql_init(&m_sqlCon); //��ʼ��MYSQL���ݿ����
	if (!mysql_real_connect(&m_sqlCon, cntinfo.host, cntinfo.user,
		cntinfo.password, cntinfo.database, atoi(cntinfo.port), NULL, 0))
	{
		cout << "HNUSQL Connect Error" << endl;
		return 0;
	}
	else
	{
		//cout << "HNUSQL Connect Success" << endl;
		cntInfo(m_sqlCon);
		mysql_query(&m_sqlCon, "set names 'gbk'"); //�������ݿ��ַ���ʽ�����������������
	}
	/*���ӳ���-end*/

	/*��ȡ���ִ��-begin*/
	//�˴���ȡsql��䲢ִ��
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
			//MYSQL��ѯstudent����������浽SQLSERVER���ݿ���
			
			result = mysql_store_result(&m_sqlCon);
			print_table_data(result);
		}
	}
	/*��ȡ���ִ��-end*/
	return 0;
}

//��ȡ�����в���
bool read_args(int argc, char* args[], HnusqlConnectInfo& cntinfo)
{
	if (argc <= 1)
		return false;
	char *tmpstr; //��ʱ�ַ���
	//char prestr[50]; //ǰ׺
	bool errorFlag;
	bool pswFlag = false; //�������Ƿ�ָ������ѡ��
	//���в����ĸ�ֵ
	for (int i = 1; i < argc; i++)
	{
		tmpstr = args[i];
		//���ò����Ͳ�����Ϣ�б��е���Ϣ����ƥ��
		errorFlag = true;
		for (auto argsinfolist : cntinfoList)
		{
			if (strstr(tmpstr, argsinfolist.pre) == tmpstr) {
				if (strcmp(argsinfolist.pre, "-p") == 0 || strcmp(argsinfolist.pre, "--password=") == 0)
					pswFlag = true; //ѡ���д�������ѡ��

				if (argsinfolist.argsType == COMMAND) 
				{
					if (argsinfolist.addr != NULL) (*((void (*)())argsinfolist.addr))(); //ִ�к���ָ��
					exit(0); //ִ��ָ������֮���������
				}
				int len = strlen(argsinfolist.pre);
				if (tmpstr[len] == '\0') 
				{
					if (i + 1 < argc && !matchOption(args[i + 1])) //�����һ����������ѡ��Ļ�������Ϊ�Ǹò���������
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
				
				errorFlag = false; //ƥ��ɹ�-���ñ�־λ
				break;
			}
		}
		if (errorFlag) {
			cout << "hnusql: [ERROR] unknown option \'" << tmpstr << "\'" <<endl;
			exit(1); //��ĳһ���������б��е���Ϣ����ƥ��
		}
	}
	//�������ĺ�����
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
				if (j > 0) { //����ɾ������
					cout << '\b' << ' ' << '\b';
					j--;
				}
				j--;
			}
			else if (j == sizeof(cntinfo.password)-1) { //������������
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

//������ƥ�����
bool matchOption(const char* str)
{
	for (auto argsinfolist : cntinfoList)
	{
		if (strstr(str, argsinfolist.pre) == str) return true;
	}
	return false;
}


//��ӡ������
void print_table_data(MYSQL_RES* result)
{
	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	int i,j;
	int num_fields = (int)mysql_num_fields(result); //��ȡ�������
	fields = mysql_fetch_fields(result);   //��ȡ��������

	//��ӡ��һ�б߽����
	cout << "+";
	for (i = 0; i < num_fields; i++) //����ÿһ�е��������
	{
		fields[i].max_length = MAX(fields[i].name_length, fields[i].max_length);
		cout << "-";
		for (j = 0; j < fields[i].max_length; j++) cout << "-";
		cout << "-+";
	}
	cout << endl;
	
	//��ӡ����
	cout << "| ";
	for (i = 0; i < num_fields; i++)       //�������
		cout << setiosflags(ios::left) << setw(fields[i].max_length) << fields[i].name << " | ";
	cout << endl;

	//��ӡ�߽��з���
	cout << "+";
	for (i = 0; i < num_fields; i++) //����ÿһ�е��������
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
	
	//��ӡ�߽��з���
	cout << "+";
	for (i = 0; i < num_fields; i++) //����ÿһ�е��������
	{
		cout << "-";
		for (j = 0; j < fields[i].max_length; j++) cout << "-";
		cout << "-+";
	}
	cout << endl;

	return;
}

//��ӡ������Ϣ
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

//��ӡ�汾��Ϣ
void versionInfo(void)
{
	cout << "hnusql  Ver 14.14 Distrib 5.7.18, for Win32 (AMD64)" << endl;
	exit(0);
}

//��ӡ���ӳɹ�ʱ����Ϣ
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