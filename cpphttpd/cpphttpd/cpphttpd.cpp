//待日后删除
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
//#include <sys/socket.h>
#include <sys/types.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <unistd.h>
#include <ctype.h>
//#include <strings.h>
#include <string.h>
#include <sys/stat.h>
//#include <pthread.h>
//#include <sys/wait.h>
#include <stdlib.h>

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ws2tcpip.h>
#include <thread>

#include <fstream>
#include <algorithm>



//宏定义，是否是空格
#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"
#define PORT 8090

#define PRINT_ALL_RAW_DATA_DEBUG false

 //每次收到请求，创建一个线程来处理接受到的请求
 //把client_sock转成地址作为参数传入pthread_create
void accept_request(void *arg);
//void accept_request(int, std::string);

//错误请求
void bad_request(int);

//读取文件
void cat(int, FILE *);

//无法执行
void cannot_execute(int);

//错误输出
void error_die(const char *);

//执行cig脚本
//void execute_cgi(int, const char *, const char *, const char *);

//得到一行数据,只要发现c为\n,就认为是一行结束，如果读到\r,再用MSG_PEEK的方式读入一个字符，如果是\n，从socket用读出
//如果是下个字符则不处理，将c置为\n，结束。如果读到的数据为0中断，或者小于0，也视为结束，c置为\n
int get_line(int, char *, int);

//返回http头
void headers(int, const char *);

//没有发现文件
void not_found(int);

//如果不是CGI文件，直接读取文件返回给请求的http客户端
void serve_file(int, const char *);

//开启tcp连接，绑定端口等操作
int startup(u_short *);

//如果不是Get或者Post，就报方法没有实现
void unimplemented(int);

/*
//linux的进程操作
typedef int pid_t;
int fork();
void waitpid(pid_t, int*, int);

//linux的管道操作
int pipe(int[]);

//linux输出重定向
void dup2(int, int);

//linux exec系列函数
void execl(const char*, const char*, int);
*/

// Http请求，后续主要是处理这个头
//
// GET / HTTP/1.1
// Host: 192.168.0.23:47310
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/55.0.2883.87 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*; q = 0.8
// Accept - Encoding: gzip, deflate, sdch
// Accept - Language : zh - CN, zh; q = 0.8
// Cookie: __guid = 179317988.1576506943281708800.1510107225903.8862; monitor_count = 5
//

// POST / color1.cgi HTTP / 1.1
// Host: 192.168.0.23 : 47310
// Connection : keep - alive
// Content - Length : 10
// Cache - Control : max - age = 0
// Origin : http ://192.168.0.23:40786
// Upgrade - Insecure - Requests : 1
// User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 55.0.2883.87 Safari / 537.36
// Content - Type : application / x - www - form - urlencoded
// Accept : text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, */*;q=0.8
// Referer: http://192.168.0.23:47310/
// Accept-Encoding: gzip, deflate
// Accept-Language: zh-CN,zh;q=0.8
// Cookie: __guid=179317988.1576506943281708800.1510107225903.8862; monitor_count=281
// Form Data
// color=gray

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
 /**********************************************************************/
/*
void accept_request(void *arg)
{
	//socket
	int client = (intptr_t)arg;
	std::string filename = "D:\\PC\\GitFiles\\network\\cpphttpd\\log\\log.txt";
	std::ofstream file;
	file.open(filename, std::ios::out | std::ios::app);

	char buf[1024];
	std::string sbuf;
	int numchars;
	//int cgi = 0;     becomes true if server decides this is a CGI program 

	//获取第一行
	numchars = get_line(client, buf, sizeof(buf));

	if (numchars == 0)
	{
		file << "begin accept_request:" << std::endl;
		file << "Confusing HTTP data: empty" << std::endl;
		file << "end accept_request:" << std::endl << std::endl;
	}
	else if (PRINT_ALL_RAW_DATA_DEBUG)
	{
		file << "begin accept_request:" << std::endl;
		sbuf = std::string(buf);
		file << sbuf.c_str();
		int dump = 0;

		//读取所有信息
		while ((numchars > 0) && strcmp("\n", buf))
		{
			numchars = get_line(client, buf, sizeof(buf));
			sbuf = std::string(buf);
			dump++;
			file << sbuf.c_str();
		}
		//bad_request(client);
		not_found(client);
		file << "end accept_request:" << std::endl << std::endl;
	}
	else
	{
		file << "begin accept_request:" << std::endl;
		//可解析的第一行字符串是get或post, 然后有空格 get或post大小写不限定
		sbuf = std::string(buf);
		httpMethodStr hms = parserFirstLine(sbuf);
		file << sbuf.c_str();
		int dump = 0;

		//读取所有信息
		while ((numchars > 0) && strcmp("\n", buf))
		{
			numchars = get_line(client, buf, sizeof(buf));
			sbuf = std::string(buf);
			dump++;
			file << sbuf.c_str();
		}

		if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_OTHER)
		{
			unimplemented(client);
		}
		else if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_GET_PARAERROR)
		{
			bad_request(client);
		}
		else if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_GET_COMMON || 
			hms.httpMethod == HTTPMETHOD::HTTPMETHOD_POST)   //查找文件
		{
			std::fstream _file;
			std::string dir = GetProgramDir();
			std::replace(hms.dir.begin(), hms.dir.end(), '/', '\\');
			if (hms.dir[0] == '\\') hms.dir.erase(hms.dir.begin());

			std::string fullPath = dir + "\\" + hms.dir;
			_file.open(fullPath, std::ios::in);
			if (!_file) //如果找不到文件
			{
				not_found(client);
			}
			else //找到了
			{
				//not_found(client);
				serve_file(client, fullPath.c_str());
			}
			//执行cgi文件
			//execute_cgi(client, path, method, query_string);
		}
		file << "end accept_request:" << std::endl << std::endl;
	}

	//执行完毕关闭socket
	closesocket(client);
	file.close();
}
*/

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
 * Parameters: client socket */
 /**********************************************************************/
void bad_request(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.1 400 BAD REQUEST\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client, buf, sizeof(buf), 0);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
 /**********************************************************************/

 //得到文件内容，发送
void cat(int client, FILE *resource)
{
	char buf[1024];

	fgets(buf, sizeof(buf), resource);
	//循环读
	while (!feof(resource))
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
	if (strlen(buf) != 0) send(client, buf, strlen(buf), 0);
	return;
}

/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
 * Parameter: the client socket descriptor. */
 /**********************************************************************/
void cannot_execute(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.1 500 Internal Server Error\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
 /**********************************************************************/
void error_die(const char *sc)
{
	perror(sc);
	exit(1);
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
 * appropriate.
 * Parameters: client socket descriptor
 *             path to the CGI script */
 /**********************************************************************/
//修改: 采用父子线程方式执行此函数, windows上没有fork机制
//子线程执行脚本结果放在记事本中, 然后子线程定时循环查看记事本信息, 执行脚本完毕后通知父线程
//父线程再从记事本中获取信息
//void execute_cgi(int client, const char *path,
//	const char *method, const char *query_string)
//{
//	 //500 INTERNAL ERROR
//	 //cannot_execute(client);
//
//
//	//缓冲区
//	char buf[1024];
//
//	//2根管道
//	int cgi_output[2];
//	int cgi_input[2];
//
//	//进程pid和状态
//	pid_t pid;
//	int status;
//
//	int i;
//	char c;
//
//	//读取的字符数
//	int numchars = 1;
//
//	//http的content_length
//	int content_length = -1;
//
//	//默认字符
//	buf[0] = 'A'; buf[1] = '\0';
//
//	//忽略大小写比较字符串
//	if (_stricmp(method, "GET") == 0)
//		//读取数据，把整个header都读掉，以为Get写死了直接读取index.html，没有必要分析余下的http信息了
//		while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
//			numchars = get_line(client, buf, sizeof(buf));
//	else    /* POST */
//	{
//		numchars = get_line(client, buf, sizeof(buf));
//		while ((numchars > 0) && strcmp("\n", buf))
//		{
//			//如果是POST请求，就需要得到Content-Length，Content-Length：这个字符串一共长为15位，所以
//			//取出头部一句后，将第16位设置结束符，进行比较
//			//第16位置为结束
//			buf[15] = '\0';
//			if (_stricmp(buf, "Content-Length:") == 0)
//				//内存从第17位开始就是长度，将17位开始的所有字符串转成整数就是content_length
//				content_length = atoi(&(buf[16]));
//			numchars = get_line(client, buf, sizeof(buf));
//		}
//		if (content_length == -1) {
//			bad_request(client);
//			return;
//		}
//	}
//
//	sprintf(buf, "HTTP/1.0 200 OK\r\n");
//	send(client, buf, strlen(buf), 0);
//	//建立output管道
//	if (pipe(cgi_output) < 0) {
//		cannot_execute(client);
//		return;
//	}
//
//	//建立input管道
//	if (pipe(cgi_input) < 0) {
//		cannot_execute(client);
//		return;
//	}
//	//       fork后管道都复制了一份，都是一样的
//	//       子进程关闭2个无用的端口，避免浪费             
//	//       ×<------------------------->1    output
//	//       0<-------------------------->×   input 
//
//	//       父进程关闭2个无用的端口，避免浪费             
//	//       0<-------------------------->×   output
//	//       ×<------------------------->1    input
//	//       此时父子进程已经可以通信
//
//
//	//fork进程，子进程用于执行CGI
//	//父进程用于收数据以及发送子进程处理的回复数据
//	if ((pid = fork()) < 0) {
//		cannot_execute(client);
//		return;
//	}
//	if (pid == 0)  /* child: CGI script */
//	{
//		char meth_env[255];
//		char query_env[255];
//		char length_env[255];
//
//		//子进程输出重定向到output管道的1端
//		dup2(cgi_output[1], 1);
//		//子进程输入重定向到input管道的0端
//		dup2(cgi_input[0], 0);
//
//		//关闭无用管道口
//		close(cgi_output[0]);
//		close(cgi_input[1]);
//
//		//CGI环境变量
//		sprintf(meth_env, "REQUEST_METHOD=%s", method);
//		putenv(meth_env);
//		if (_stricmp(method, "GET") == 0) {
//			sprintf(query_env, "QUERY_STRING=%s", query_string);
//			putenv(query_env);
//		}
//		else {   /* POST */
//			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
//			putenv(length_env);
//		}
//		//替换执行path
//		execl(path, path, NULL);
//		//int m = execl(path, path, NULL);
//		//如果path有问题，例如将html网页改成可执行的，但是执行后m为-1
//		//退出子进程，管道被破坏，但是父进程还在往里面写东西，触发Program received signal SIGPIPE, Broken pipe.
//		exit(0);
//	}
//	else {    /* parent */
//
//  //关闭无用管道口
//		close(cgi_output[1]);
//		close(cgi_input[0]);
//		if (_stricmp(method, "POST") == 0)
//			for (i = 0; i < content_length; i++) {
//				//得到post请求数据，写到input管道中，供子进程使用
//				recv(client, &c, 1, 0);
//				write(cgi_input[1], &c, 1);
//			}
//		//从output管道读到子进程处理后的信息，然后send出去
//		while (read(cgi_output[0], &c, 1) > 0)
//			send(client, &c, 1, 0);
//
//		//完成操作后关闭管道
//		close(cgi_output[0]);
//		close(cgi_input[1]);
//
//		//等待子进程返回
//		waitpid(pid, &status, 0);
//
//	}
//}

/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character.  If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null.  If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
 /**********************************************************************/

 //得到一行数据,只要发现c为\n,就认为是一行结束，如果读到\r,再用MSG_PEEK的方式读入一个字符，如果是\n，从socket用读出
 //如果是下个字符则不处理，将c置为\n，结束。如果读到的数据为0中断，或者小于0，也视为结束，c置为\n
int get_line(int sock, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
		n = recv(sock, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if (n > 0)
		{
			if (c == '\r')
			{
				//偷窥一个字节，如果是\n就读走
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					//不是\n（读到下一行的字符）或者没读到，置c为\n 跳出循环,完成一行读取
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';

	return(i);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
 *             the name of the file */
 /**********************************************************************/

 //加入http的headers
void headers(int client, const char *filename)
{
	//char buf[1024];
	//(void)filename;  /* could use filename to determine file type */

	send(client, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
	send(client, SERVER_STRING, strlen(SERVER_STRING), 0);
	send(client, "Content-Type: text/html\r\n", strlen("Content-Type: text/html\r\n"), 0);
	send(client, "\r\n", strlen("\r\n"), 0);

	send(client, "?????", strlen("?????"), 0);

	//strcpy(buf, "HTTP/1.1 200 OK\r\n");
	//send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/

//如果资源没有找到得返回给客户端下面的信息
void not_found(int client)
{
	//char buf[1024];

	//sprintf(buf, "HTTP/1.1 404 NOT FOUND\r\n");
	send(client, "HTTP/1.1 404 NOT FOUND\r\n", strlen("HTTP/1.1 404 NOT FOUND\r\n"), 0);

	//sprintf(buf, SERVER_STRING);
	//send(client, buf, strlen(buf), 0);

	//sprintf(buf, "Date: Sat, 05 Oct 2019 11 : 18 : 13 GMT\r\n");
	//send(client, buf, strlen(buf), 0);
	/*
	sprintf(buf, "X-Ua-Compatible: IE=Edge,chrome=1\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Encoding: gzip\r\n");
	send(client, buf, strlen(buf), 0);
	*/
	//sprintf(buf, "Content-Type: text/html;charset=utf-8\r\n");
	send(client, "Content-Type: text/html;charset=utf-8\r\n", strlen("Content-Type: text/html;charset=utf-8\r\n"), 0);
	
	//sprintf(buf, "\r\n");
	send(client, "\r\n", strlen("\r\n"), 0);

	//sprintf(buf, "I WANNA FUCK YOU\r\n");
	send(client, "404 NOT FOUND\r\n", strlen("404 NOT FOUND\r\n"), 0);

	/*

	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
	*/
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
 /**********************************************************************/

 //如果不是CGI文件，直接读取文件返回给请求的http客户端
void serve_file(int client, const char *filename)
{
	FILE *resource = NULL;

	resource = fopen(filename, "r");
	if (resource == NULL)
		not_found(client);
	else
	{
		headers(client, filename);
		cat(client, resource);
	}
	fclose(resource);
}

/**********************************************************************/
/* This function starts the process of listening for web connections
 * on a specified port.  If the port is 0, then dynamically allocate a
 * port and modify the original port variable to reflect the actual
 * port.
 * Parameters: pointer to variable containing the port to connect on
 * Returns: the socket */
 /**********************************************************************/
/*
int startup(u_short *port)
{
	//int httpd = 0;
	struct sockaddr_in name;

	//httpd = socket(PF_INET, SOCK_STREAM, 0);
	SOCKET httpd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpd == INVALID_SOCKET)
		error_die("socket");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	//绑定socket
	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		error_die("bind");
	//如果端口没有设置，提供个随机端口
	if (*port == 0)  //if dynamically allocating a port
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
			error_die("getsockname");
		*port = ntohs(name.sin_port);
	}
	//监听
	if (listen(httpd, 5) < 0)
		error_die("listen");
	return(httpd);
}
*/

/**********************************************************************/
/* Inform the client that the requested web method has not been
 * implemented.
 * Parameter: the client socket */
 /**********************************************************************/

 //如果方法没有实现，就返回此信息
void unimplemented(int client)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client, buf, strlen(buf), 0);
}

/**********************************************************************/
/*
int main(void)
{
	//parserFirstLineTest();

	int server_sock = -1;
	u_short port = PORT;
	int client_sock = -1;
	struct sockaddr_in client_name;

	int acceptCount = 0;

	//这边要为socklen_t类型
	socklen_t client_name_len = sizeof(client_name);

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		throw new std::exception("WSAStartup func error");
	}

	server_sock = startup(&port);
	printf("httpd running on port %d\n", port);

	while (1)
	{
		//接受请求，函数原型
		//#include <sys/types.h>
		//#include <sys/socket.h>  
		//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		client_sock = accept(server_sock,
			(struct sockaddr *)&client_name,
			&client_name_len);
		if (client_sock == -1)
			error_die("accept");

		//accept_request((void *)(intptr_t)client_sock);  //单线程

		 //每次收到请求，创建一个线程来处理接受到的请求
		 //把client_sock转成地址作为参数传入
		std::thread t1(accept_request, (void *)(intptr_t)client_sock);
		t1.join();
	}
	closesocket(server_sock);
	int r = WSACleanup();
	if (r != 0)
	{
		throw new std::exception("WSACleanup func error");
	}

	return(0);
}
*/
