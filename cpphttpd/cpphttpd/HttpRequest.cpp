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

std::vector<std::string> HttpRequest::getRequestContent()
{
	std::vector<std::string> res;
	char buf[1024];
	int numchars = this->getLine(this->clientSocketID, buf, sizeof(buf));;
	if (numchars == 0) return res;
	else
	{
		res.emplace_back(std::string(buf));
		while (true)
		{
			numchars = this->getLine(this->clientSocketID, buf, sizeof(buf));
			if (!((numchars > 0) && strcmp("\n", buf))) break;
			else
			{
				res.emplace_back(std::string(buf));
			}
		}
	}
	return res;
}

/*
 * 通用请求
 * s: 备选字符串, 对于非200响应是默认发送消息, 对于200响应是文件路径
 *  , isRenderFile: 是否用文件内容, (enum)en请求类型
 */
void HttpRequest::CommonResponse(std::string s, bool isRenderFile, enum HTTPCODE hc)
{
	switch (hc)
	{
	case HttpRequest::HTTP200:
	{
		send(this->clientSocketID, this->HTTP_CODE200.c_str(), (int)this->HTTP_CODE200.size(), 0);
		break;
	}
	case HttpRequest::HTTP404:
	{
		send(this->clientSocketID, this->HTTP_CODE404.c_str(), (int)this->HTTP_CODE404.size(), 0);
		break;
	}
	case HttpRequest::HTTP400:
	{
		send(this->clientSocketID, this->HTTP_CODE400.c_str(), (int)this->HTTP_CODE400.size(), 0);
		break;
	}
	case HttpRequest::HTTP500:
	{
		send(this->clientSocketID, this->HTTP_CODE500.c_str(), (int)this->HTTP_CODE500.size(), 0);
		break;
	}
	case HttpRequest::HTTP501:
	{
		send(this->clientSocketID, this->HTTP_CODE501.c_str(), (int)this->HTTP_CODE501.size(), 0);
		break;
	}
	default:
		break;
	}
	send(this->clientSocketID, this->HTTP_SERVER.c_str(), (int)this->HTTP_SERVER.size(), 0);
	send(this->clientSocketID, this->HTTP_CHARSET.c_str(), (int)this->HTTP_CHARSET.size(), 0);
	send(this->clientSocketID, this->HTTP_EMPTR_LINE.c_str(), (int)this->HTTP_EMPTR_LINE.size(), 0);
	if (isRenderFile)
	{
		switch (hc)
		{
		case HttpRequest::HTTP200:
		{
			if (!this->SendFileContent(s.c_str()))
			{
				if (!this->SendFileContent(FILE_NOTFOUND.c_str()))
					send(this->clientSocketID, "url file not found", (int)s.size(), 0);
			}
			break;
		}
		case HttpRequest::HTTP404:
		{
			if (!this->SendFileContent(FILE_NOTFOUND.c_str()))
				send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
			break;
		}
		case HttpRequest::HTTP400:
		{
			if (!this->SendFileContent(FILE_BAD_REQUEST.c_str()))
				send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
			break;
		}
		case HttpRequest::HTTP500:
		{
			if (!this->SendFileContent(FILE_INTERNAL_SERVER_ERROR.c_str()))
				send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
			break;
		}
		case HttpRequest::HTTP501:
		{
			if (!this->SendFileContent(FILE_METHOD_NOT_IMPLEMENTED.c_str()))
				send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
			break;
		}
		default:
			break;
		}
	}
	else
		send(this->clientSocketID, s.c_str(), (int)s.size(), 0);
}

/*
 * 发送文件内容
 * 返回true说明文件存在发送成功, false说明文件不存在, 需要调用者自行处理content内容
 *    注意: 此处用到工作目录, filename 按照如下格式html/404.html
 *    因此: 用filesystem库格式化这个目录, 再用C api进行文件读写
 */
bool HttpRequest::SendFileContent(const char * filename)
{
	std::filesystem::path filePath = std::filesystem::absolute(filename);
	std::string windowsTypePath = filePath.string();

	FILE *resource = NULL;
	errno_t err;
	err = fopen_s(&resource, windowsTypePath.c_str(), "r");
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
 * HttpRequest类初始化, 初始化客户端SocketID
 * 初始化项目目录
 */
HttpRequest::HttpRequest(int c) :clientSocketID(c)
{
	pmLog = Mlog::Instance();
}

/*
 * 接受请求总接口
 */
void HttpRequest::acceptRequestInterface()
{
	//获取请求数据
	pmLog->print(this->filename, "                                 ");
	pmLog->print(this->filename, "=================================");
	pmLog->print(this->filename, "begin accept_request");
	std::vector<std::string> requestContent =  this->getRequestContent();

	//打印request所有信息
	for (auto& rC : requestContent) pmLog->print(this->filename, rC);

	if (requestContent.empty())
		pmLog->print(this->filename, "Confusing HTTP data: empty");
	else if (PRINT_ALL_RAW_DATA_DEBUG)
		this->NotFound("NOT FOUNDDDDD", true);
	else
	{
		httpMethodStr hms = parserFirstLine(requestContent[0]);   //解析第一行字符串
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
			//this->NotFound("NOT FOUNDDDDD", true);
			//如果hms.dir为空, 则使用DEFAULT_FILE
			//否则如果hms.dir[0] == '/'则去除头部(实际上一定是'/')再用hms.dir
			//否则直接用hms.dir
			if (hms.dir.empty()) this->NormalRequest(DEFAULT_FILE, true);
			else if (hms.dir[0] == '/')
			{
				hms.dir.erase(hms.dir.begin());
				this->NormalRequest(hms.dir, true);
			}
			else this->NormalRequest(hms.dir, true);
			//执行cgi文件
			//execute_cgi(client, path, method, query_string);
		}
	}
	pmLog->print(this->filename, "end accept_request");
	//执行完毕关闭socket
	closesocket(this->clientSocketID);
}

void HttpRequest::closeRequestInterface()
{
	this->pmLog->destroy();
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
	h.closeRequestInterface();
}
