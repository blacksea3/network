#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "Log.h"

//HTTP响应类

class HttpRequest
{
private:
	//类常量-恒定内容
	enum HTTPCODE{HTTP200, HTTP404, HTTP400, HTTP500, HTTP501};
	const std::string LOGFILENAME = "log/log.txt";                   //日志文件路径
	const std::string HTTP_CODE200 = "HTTP/1.1 200 OK\r\n";
	const std::string HTTP_CODE404 = "HTTP/1.1 404 NOT FOUND\r\n";
	const std::string HTTP_CODE400 = "HTTP/1.1 400 BAD REQUEST\r\n";
	const std::string HTTP_CODE500 = "HTTP/1.1 500 Internal Server Error\r\n";
	const std::string HTTP_CODE501 = "HTTP/1.0 501 Method Not Implemented\r\n";
	const std::string HTTP_SERVER = "Server: jxthttpd/0.0.1\r\n";
	const std::string HTTP_CHARSET = "Content-Type: text/html;charset=utf-8\r\n";
	const std::string HTTP_EMPTR_LINE = "\r\n";

	const std::string FILE_NOTFOUND = "html/404.html";
	const std::string FILE_BAD_REQUEST = "html/400.html";
	const std::string FILE_METHOD_NOT_IMPLEMENTED = "html/501.html";
	const std::string FILE_INTERNAL_SERVER_ERROR = "html/500.html";

	const std::string DEFAULT_FILE = "index.html";

	//类常量-设置
	const bool PRINT_ALL_RAW_DATA_DEBUG = false;  //如果true: 仅打印所有请求信息, 全部按照404响应(或其他单一响应), 否则正常响应

private:  //私有变量
	unsigned int clientSocketID;   //客户端SocketID  (其实就是SOCKET类型)
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
	HttpRequest(unsigned int c);
	void acceptRequestInterface();                  //接受请求接口
	void closeRequestInterface();                   //关闭请求接口
	~HttpRequest();
};

void acceptRequestThread(unsigned int client);                    //供子进程调用的函数入口
