#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "Log.h"

//HTTP响应类

class HttpRequest
{
	//类常量
private:
	enum HTTPCODE{HTTP200, HTTP404, HTTP400, HTTP500, HTTP501};

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

private:  //私有变量
	int clientSocketID;   //客户端SocketID
	Mlog* pmLog;          //Mlog指针, 这是唯一的, Mlog是单例设计

private:  //对win32 socket api部分函数的封装, 牺牲效率, 提高可读性
	int getLine(int sock, char *buf, int size);        //获取一行数据
	std::vector<std::string> getRequestContent();      //获取多行数据放入vector中, 每个string内都有一个换行符

private:  //上层函数
	void CommonResponse(std::string s, bool isRenderFile, enum HTTPCODE hc);  //通用响应函数
	//以下5个方法: s: 备选字符串, isRenderFile: 是否用文件内容
	inline void NormalRequest(std::string s, bool isRenderFile)
	{
		this->CommonResponse(s, isRenderFile, HTTP200);
	};
	inline void NotFound(std::string s = "not found", bool isRenderFile = true)
	{
		this->CommonResponse(s, isRenderFile, HTTP404);
	};
	void BadRequest(std::string s = "bad request", bool isRenderFile = true)
	{
		this->CommonResponse(s, isRenderFile, HTTP400);
	};
	void MethodNotImplemented(std::string s = "method not implemented", bool isRenderFile = true)
	{
		this->CommonResponse(s, isRenderFile, HTTP501);
	};
	void InternalServerError(std::string s = "internal server error", bool isRenderFile = true)
	{
		this->CommonResponse(s, isRenderFile, HTTP500);
	};
	bool SendFileContent(const char * filename);    //发送文件内容
public:
	HttpRequest(int c);
	void acceptRequestInterface();                  //接受请求接口
	void closeRequestInterface();                   //关闭请求接口
	~HttpRequest();
};

void acceptRequestThread(void*);                    //供子进程调用的函数入口
