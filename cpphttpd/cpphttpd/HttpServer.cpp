#include "HttpServer.h"
#include "HttpRequest.h"


/*
 * 构造函数: 初始化HttpServer类, 初始化类变量
 */
HttpServer::HttpServer():port(DEFAULTPORT)
{
	;
}

/*
 * 初始化服务器
 * 加载Ws2_32.dll, 生成项目目录
 */
void HttpServer::InitServer()
{
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		std::cout << "初始化套接字库失败！" << std::endl;
	}
	else {
		std::cout << "初始化套接字库成功！" << std::endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		std::cout << "套接字库版本号不符！" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "套接字库版本正确！" << std::endl;
	}
	//填充服务端地址信息

	if (useParentDir)
	{
		std::filesystem::path parentFullPath = std::filesystem::absolute(std::filesystem::path("../"));
		std::filesystem::current_path(parentFullPath);
	}
}

/*
 * 开始服务器
 * 即开始TCP服务端
 */
SOCKET HttpServer::StartServer(u_short port)
{
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	char recv_buf[100];
	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	SOCKET s_accept;
	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	SOCKADDR_IN accept_addr;


	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		std::cout << "套接字绑定失败！" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "套接字绑定成功！" << std::endl;
	}
	//设置套接字为监听状态
	if (listen(s_server, SOMAXCONN) < 0) {
		std::cout << "设置监听状态失败！" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "设置监听状态成功！" << std::endl;
	}
	std::cout << "服务端正在监听连接，请稍候...." << std::endl;
	return s_server;
}

/*
 * 运行服务器, 此方法内置while(1)循环!
 */
void HttpServer::RunServer()
{
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	char recv_buf[100];
	//定义服务端套接字，接受请求套接字
	SOCKET ss_server;
	SOCKET s_accept;
	SOCKADDR_IN server_addr, accept_addr;


	ss_server = this->StartServer(8868);
	//接受连接请求
	len = sizeof(SOCKADDR);

	while (1)
	{
		s_accept = accept(ss_server, (SOCKADDR *)&accept_addr, &len);
		if (s_accept == SOCKET_ERROR) {
			std::cout << "连接失败！" << std::endl;
			WSACleanup();
			continue;
		}
		std::cout << "连接建立，准备接受数据" << std::endl;
		//acceptRequestThread(s_accept);  //单线程
		//接收数据
		while (1) {
			recv_len = recv(s_accept, recv_buf, 100, 0);
			if (recv_len < 0) {
				std::cout << "接受失败！" << std::endl;
				break;
			}
			else if (recv_len == 0)
			{
				std::cout << "没有消息了, 0长度" << std::endl;
				break;
			}
			else {
				std::cout << "客户端信息:" << recv_buf << std::endl;
			}
			if (recv_len < 100)
			{
				std::cout << "没有消息了, " << recv_len << "长度" << std::endl;
				break;
			}
			//std::cout << "请输入回复信息:";
			//std::cin >> send_buf;
			//send_len = send(s_accept, send_buf, 100, 0);
			//if (send_len < 0) {
			//	std::cout << "发送失败！" << std::endl;
			//	break;
			//}
		}
		//关闭套接字
		closesocket(ss_server);
	}
	closesocket(s_accept);
	//释放DLL资源
	WSACleanup();






	/*
	while (true)
	{
		s_accept = accept(s_server, (SOCKADDR *)&accept_addr, &len);
		if (s_accept == SOCKET_ERROR)
		{
			std::cout << "连接失败！" << std::endl;
			WSACleanup();
			return;
		}
		std::cout << "连接建立，准备接受数据" << std::endl;
		acceptRequestThread(s_accept);  //单线程
		
		//接收数据
		while (1) {
			recv_len = recv(s_accept, recv_buf, 100, 0);
			if (recv_len < 0) {
				std::cout << "接受失败！" << std::endl;
				break;
			}
			else {
				std::cout << "客户端信息:" << recv_buf << std::endl;
			}
			std::cout << "请输入回复信息:";
			std::cin >> send_buf;
			send_len = send(s_accept, send_buf, 100, 0);
			if (send_len < 0) {
				std::cout << "发送失败！" << std::endl;
				break;
			}
		}
		
		closesocket(s_accept);
	}
	*/
	/*
	while (1)
	{
		//接受请求，函数原型
		//#include <sys/types.h>
		//#include <sys/socket.h>
		//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		this->clientSockID = accept(this->serverSockID, (SOCKADDR *)&accept_addr, &len);
		if (this->clientSockID == SOCKET_ERROR)
			throw new std::exception("连接失败!");

		acceptRequestThread((void *)(intptr_t)this->clientSockID);  //单线程

		 //每次收到请求，创建一个线程来处理接受到的请求
		 //把client_sock转成地址作为参数传入
		//std::thread t1(acceptRequestThread, (void *)(intptr_t)this->clientSockID);
		//t1.join();   //等待t1结束
	}
	closesocket(this->serverSockID);
	*/
}

void HttpServer::BIGRunServer()
{
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	char recv_buf[100];
	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	SOCKET s_accept;
	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	SOCKADDR_IN accept_addr;


	//填充服务端信息
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8070);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		std::cout << "套接字绑定失败！" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "套接字绑定成功！" << std::endl;
	}
	//设置套接字为监听状态
	if (listen(s_server, SOMAXCONN) < 0) {
		std::cout << "设置监听状态失败！" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "设置监听状态成功！" << std::endl;
	}
	std::cout << "服务端正在监听连接，请稍候...." << std::endl;
	//接受连接请求
	len = sizeof(SOCKADDR);
	s_accept = accept(s_server, (SOCKADDR *)&accept_addr, &len);
	if (s_accept == SOCKET_ERROR) {
		std::cout << "连接失败！" << std::endl;
		WSACleanup();
		return;
	}
	std::cout << "连接建立，准备接受数据" << std::endl;
	//接收数据
	while (1) {
		recv_len = recv(s_accept, recv_buf, 100, 0);
		if (recv_len < 0) {
			std::cout << "接受失败！" << std::endl;
			break;
		}
		else {
			std::cout << "客户端信息:" << recv_buf << std::endl;
		}
		//std::cout << "请输入回复信息:";
		//std::cin >> send_buf;
		//send_len = send(s_accept, send_buf, 100, 0);
		//if (send_len < 0) {
		//	std::cout << "发送失败！" << std::endl;
		//	break;
		//}
	}
	//关闭套接字
	closesocket(s_server);
	closesocket(s_accept);
	//释放DLL资源
	WSACleanup();
}

/*
 * 默认析构函数
 */
HttpServer::~HttpServer()
{
	;
}

/*
int main()
{	
	
	HttpServer hs = HttpServer();
	hs.InitServer();
	hs.BIGRunServer();
	return 0;
	
	HttpServer hs = HttpServer();
	hs.InitServer();
	hs.RunServer();
	return 0;
}*/

