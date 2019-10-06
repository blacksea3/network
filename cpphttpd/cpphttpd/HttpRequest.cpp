#include "HttpRequest.h"
#include "HttpServer.h"
#include "HttpStringParser.h"

/*
 * 获取当前程序运行路径
 */
static std::string GetProgramDir()
{
	char exeFullPath[MAX_PATH]; // Full path
	std::string strPath = "";
	GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
	strPath = (std::string)exeFullPath;    // Get full path of the file
	int pos = strPath.find_last_of('\\', strPath.length());
	return strPath.substr(0, pos);  // Return the directory without the file name
}

/* 
 * 得到一行数据,只要发现c为\n,就认为是一行结束，如果读到\r,再用MSG_PEEK的方式读入一个字符，如果是\n，从socket用读出
 * 如果是下个字符则不处理，将c置为\n，结束。如果读到的数据为0中断，或者小于0，也视为结束，c置为\n
 */
int HttpRequest::getLine(int sock, char * buf, int size)
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

/*
 * 通常响应, 200
 * s: 备选字符串, isRenderFile: 是否用文件内容
 */
void HttpRequest::CommonResponse(std::string s)
{
	FILE *resource = NULL;
	errno_t err;
	err = fopen_s(&resource, s.c_str(), "r");
	if (err != 0)
	{
		this->NotFound("url file not found", true);
	}
	else
	{
		send(this->clientSocketID, this->HTTP_CODE200.c_str(), (int)this->HTTP_CODE200.size(), 0);
		send(this->clientSocketID, this->HTTP_SERVER.c_str(), (int)this->HTTP_SERVER.size(), 0);
		send(this->clientSocketID, this->HTTP_CHARSET.c_str(), (int)this->HTTP_CHARSET.size(), 0);
		send(this->clientSocketID, this->HTTP_EMPTR_LINE.c_str(), (int)this->HTTP_EMPTR_LINE.size(), 0);
		this->SendFileContent(resource);
	}
}

/*
 * 找不到
 * s: 备选字符串, isRenderFile: 是否用文件内容
 */
void HttpRequest::NotFound(std::string s, bool isRenderFile)
{
	send(this->clientSocketID, this->HTTP_CODE404.c_str(), (int)this->HTTP_CODE404.size(), 0);
	send(this->clientSocketID, this->HTTP_SERVER.c_str(), (int)this->HTTP_SERVER.size(), 0);
	send(this->clientSocketID, this->HTTP_CHARSET.c_str(), (int)this->HTTP_CHARSET.size(), 0);
	send(this->clientSocketID, this->HTTP_EMPTR_LINE.c_str(), (int)this->HTTP_EMPTR_LINE.size(), 0);
	if (isRenderFile)
	{
		if (!this->SendFileContent(FILE_NOTFOUND.c_str()))
			send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
	}
	else
		send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
}

/*
 * 请求错误
 * s: 备选字符串, isRenderFile: 是否用文件内容
 */
void HttpRequest::BadRequest(std::string s, bool isRenderFile)
{
	send(this->clientSocketID, this->HTTP_CODE400.c_str(), (int)this->HTTP_CODE400.size(), 0);
	send(this->clientSocketID, this->HTTP_SERVER.c_str(), (int)this->HTTP_SERVER.size(), 0);
	send(this->clientSocketID, this->HTTP_CHARSET.c_str(), (int)this->HTTP_CHARSET.size(), 0);
	send(this->clientSocketID, this->HTTP_EMPTR_LINE.c_str(), (int)this->HTTP_EMPTR_LINE.size(), 0);
	if (isRenderFile)
	{
		if (!this->SendFileContent(FILE_BAD_REQUEST.c_str()))
			send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
	}
	else
		send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
}

/*
 * 方法未实现
 * s: 备选字符串, isRenderFile: 是否用文件内容
 */
void HttpRequest::MethodNotImplemented(std::string s, bool isRenderFile)
{
	send(this->clientSocketID, this->HTTP_CODE501.c_str(), (int)this->HTTP_CODE501.size(), 0);
	send(this->clientSocketID, this->HTTP_SERVER.c_str(), (int)this->HTTP_SERVER.size(), 0);
	send(this->clientSocketID, this->HTTP_CHARSET.c_str(), (int)this->HTTP_CHARSET.size(), 0);
	send(this->clientSocketID, this->HTTP_EMPTR_LINE.c_str(), (int)this->HTTP_EMPTR_LINE.size(), 0);
	if (isRenderFile)
	{
		if (!this->SendFileContent(FILE_METHOD_NOT_IMPLEMENTED.c_str()))
			send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
	}
	else
		send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
}

/*
 * 内部错误
 * s: 备选字符串, isRenderFile: 是否用文件内容
 */
void HttpRequest::InternalServerError(std::string s, bool isRenderFile)
{
	send(this->clientSocketID, this->HTTP_CODE500.c_str(), (int)this->HTTP_CODE500.size(), 0);
	send(this->clientSocketID, this->HTTP_SERVER.c_str(), (int)this->HTTP_SERVER.size(), 0);
	send(this->clientSocketID, this->HTTP_CHARSET.c_str(), (int)this->HTTP_CHARSET.size(), 0);
	send(this->clientSocketID, this->HTTP_EMPTR_LINE.c_str(), (int)this->HTTP_EMPTR_LINE.size(), 0);
	if (isRenderFile)
	{
		if (!this->SendFileContent(FILE_INTERNAL_SERVER_ERROR.c_str()))
			send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
	}
	else
		send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
}

/*
 * 发送文件内容, 外部已经确认文件存在
 */
void HttpRequest::SendFileContent(FILE *resource)
{
	char buf[1024];
	fgets(buf, sizeof(buf), resource);
	//循环读
	while (!feof(resource))
	{
		send(this->clientSocketID, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
	if (strlen(buf) != 0) send(this->clientSocketID, buf, strlen(buf), 0);
	fclose(resource);
}

/*
 * 发送文件内容
 * 返回true说明文件存在发送成功, false说明文件不存在, 需要调用者自行处理content内容
 */
bool HttpRequest::SendFileContent(const char * filename)
{
	FILE *resource = NULL;
	errno_t err;
	err = fopen_s(&resource, filename, "r");
	if (err != 0) return false;
	else
	{
		char buf[1024];
		fgets(buf, sizeof(buf), resource);
		//循环读
		while (!feof(resource))
		{
			send(this->clientSocketID, buf, strlen(buf), 0);
			fgets(buf, sizeof(buf), resource);
		}
		if (strlen(buf) != 0) send(this->clientSocketID, buf, strlen(buf), 0);
		fclose(resource);
		return true;
	}
}



/*
 * HttpRequest类初始化, 仅初始化客户端SocketID
 */
HttpRequest::HttpRequest(int c) :clientSocketID(c)
{
	;
}

/*
 * 接受请求总接口
 */
void HttpRequest::acceptRequestInterface()
{
	//socket
	std::ofstream file;
	file.open(filename, std::ios::out | std::ios::app);

	char buf[1024];
	std::string sbuf;
	int numchars;
	//int cgi = 0;     becomes true if server decides this is a CGI program 

	//获取第一行
	numchars = this->getLine(this->clientSocketID, buf, sizeof(buf));

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

		//读取后续信息并抛弃
		while ((numchars > 0) && strcmp("\n", buf))
		{
			numchars = this->getLine(this->clientSocketID, buf, sizeof(buf));
			sbuf = std::string(buf);
			dump++;
			file << sbuf.c_str();
		}
		//bad_request(client);
		this->NotFound("NOT FOUNDDDDD", true);
		file << "end accept_request:" << std::endl << std::endl;
	}
	else
	{
		file << "begin accept_request:" << std::endl;
		sbuf = std::string(buf);
		file << sbuf.c_str();
		int dump = 0;

		httpMethodStr hms = parserFirstLine(sbuf);   //解析第一行字符串

		//读取后续信息并抛弃
		while ((numchars > 0) && strcmp("\n", buf))
		{
			numchars = this->getLine(this->clientSocketID, buf, sizeof(buf));
			sbuf = std::string(buf);
			dump++;
			file << sbuf.c_str();
		}

		if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_OTHER)
		{
			this->MethodNotImplemented("NOT IMPLEMENTED!!", true);
		}
		else if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_GET_PARAERROR)
		{
			this->BadRequest("BAD REQUEST!!", true);
		}
		else if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_GET_COMMON ||
			hms.httpMethod == HTTPMETHOD::HTTPMETHOD_POST)   //查找文件
		{
			std::fstream _file;
			std::string dir = GetProgramDir();
			std::replace(hms.dir.begin(), hms.dir.end(), '/', '\\');
			if (hms.dir[0] == '\\') hms.dir.erase(hms.dir.begin());

			std::string fullPath = dir + "\\" + hms.dir;
			this->CommonResponse(fullPath);
			//执行cgi文件
			//execute_cgi(client, path, method, query_string);
		}
		file << "end accept_request:" << std::endl << std::endl;
	}

	//执行完毕关闭socket
	closesocket(this->clientSocketID);
	file.close();
}

HttpRequest::~HttpRequest()
{

}

/*
 * 子线程函数
 */
void acceptRequestThread(void *arg)
{
	int client = (intptr_t)arg;
	HttpRequest h(client);
	h.acceptRequestInterface();
}
