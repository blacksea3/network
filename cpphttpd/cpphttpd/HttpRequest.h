#pragma once
#include <iostream>
#include <fstream>

//待日后删除
#define _CRT_SECURE_NO_WARNINGS

//HTTP响应类

class HttpRequest
{
	//类常量
private:
	const std::string filename = "D:\\PC\\GitFiles\\network\\cpphttpd\\log\\log.txt";  //日志文件路径
	const std::string HTTP_CODE200 = "HTTP/1.1 200 OK\r\n";
	const std::string HTTP_CODE404 = "HTTP/1.1 404 NOT FOUND\r\n";
	const std::string HTTP_CODE400 = "HTTP/1.1 400 BAD REQUEST\r\n";
	const std::string HTTP_CODE500 = "HTTP/1.1 500 Internal Server Error\r\n";
	const std::string HTTP_CODE501 = "HTTP/1.0 501 Method Not Implemented\r\n";
	const std::string HTTP_SERVER = "Server: jxthttpd/0.0.1\r\n";
	const std::string HTTP_CHARSET = "Content-Type: text/html;charset=utf-8\r\n";
	const std::string HTTP_EMPTR_LINE = "\r\n";

	const std::string FILE_NOTFOUND = "D:\\PC\\GitFiles\\network\\cpphttpd\\html\\404.html";
	const std::string FILE_BAD_REQUEST = "D:\\PC\\GitFiles\\network\\cpphttpd\\html\\400.html";
	const std::string FILE_METHOD_NOT_IMPLEMENTED = "D:\\PC\\GitFiles\\network\\cpphttpd\\html\\501.html";
	const std::string FILE_INTERNAL_SERVER_ERROR = "D:\\PC\\GitFiles\\network\\cpphttpd\\html\\500.html";

	const bool PRINT_ALL_RAW_DATA_DEBUG = false;

private:
	int clientSocketID;   //客户端SocketID
	int getLine(int sock, char *buf, int size);  //获取一行数据
	void CommonResponse(std::string s);
	void NotFound(std::string s, bool isRenderFile);
	void BadRequest(std::string s, bool isRenderFile);
	void MethodNotImplemented(std::string s, bool isRenderFile);
	void InternalServerError(std::string s, bool isRenderFile);
	void SendFileContent(FILE *resource);
	bool SendFileContent(const char * filename);
public:
	HttpRequest(int c);
	void acceptRequestInterface();
	
	~HttpRequest();
};

void acceptRequestThread(void*);
