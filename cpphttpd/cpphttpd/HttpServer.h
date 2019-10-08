#pragma once
//HTTP服务器类

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ws2tcpip.h>
#include <thread>
#include <filesystem>
#include "HttpRequest.h"

class HttpServer
{
private:
	const u_short DEFAULTPORT = 8090;				    //默认端口
	u_short port;                                       //当前端口

	int serverSockID;                                   //服务端socket ID
	int clientSockID;									//客户端socket ID
	struct sockaddr_in clientName;						//客户端名字
	int acceptCount;									//计数器, 保留, 统计客户端请求次数
	socklen_t clientNameLength;							//客户端名字字节大小

	int StartServer(u_short *port);

	const bool useParentDir = true;  //如果true, 使用当前目录的父目录作为项目目录, 否则直接将当前目录作为项目目录
public:
	HttpServer();
	void InitServer();
	void RunServer();
	~HttpServer();
};
