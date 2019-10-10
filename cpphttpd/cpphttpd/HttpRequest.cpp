#include "HttpRequest.h"
#include "HttpServer.h"
#include "HttpStringParser.h"

/*
 * 字符串分割函数
 */
static std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			result.emplace_back(str.substr(i, pos - i));
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

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
 * 获得请求内容, 内部做分行处理
 */
std::vector<std::string> HttpRequest::getRequestContent()
{
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	char recv_buf[100];
	while (1) {
		recv_len = recv(this->clientSocketID, recv_buf, 100, 0);
		if (recv_len < 0) {
			std::cout << "接受失败！" << std::endl;
			break;
		}
		else {
			std::cout << "客户端信息:" << recv_buf << std::endl;
		}
		std::cout << "请输入回复信息:";
		//std::cin >> send_buf;
		//send_len = send(s_accept, send_buf, 100, 0);
		if (send_len < 0) {
			std::cout << "发送失败！" << std::endl;
			break;
		}
	}

	/*
	char buf[20480];
	int rcd;
	std::string longContent;
	while (true)
	{
		memset(buf, 0, 0);
		rcd = recv(this->clientSocketID, buf, 20480, 0);
		if (rcd == 0) break;
		if (rcd == SOCKET_ERROR) {
			rcd = WSAGetLastError();
			if (rcd == WSAETIMEDOUT) break;
			this->pmLog->print(this->LOGFILENAME, "FATAL ERROR 接收时出现错误!\nRecv error!\nError code:" + std::to_string(WSAGetLastError()));
			break;
		}
		longContent += std::string(buf);
	}
	std::vector<std::string> res = split(longContent, "\r\n");*/
	/*
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
	}*/
	std::vector<std::string> res = {};
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
HttpRequest::HttpRequest(unsigned int c) :clientSocketID(c)
{
	pmLog = Mlog::Instance();
}

/*
 * 接受请求总接口
 */
void HttpRequest::acceptRequestInterface()
{
	/*
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	char recv_buf[100];
	while (1) {
		recv_len = recv(this->clientSocketID, recv_buf, 100, 0);
		if (recv_len < 0) {
			std::cout << "接受失败！" << std::endl;
			break;
		}
		else {
			std::cout << "客户端信息:" << recv_buf << std::endl;
		}
		std::cout << "请输入回复信息:";
		std::cin >> send_buf;
		send_len = send(this->clientSocketID, send_buf, 100, 0);
		if (send_len < 0) {
			std::cout << "发送失败！" << std::endl;
			break;
		}
	}*/
	
	//获取请求数据
	pmLog->print(this->LOGFILENAME, "                                 ");
	pmLog->print(this->LOGFILENAME, "=================================");
	pmLog->print(this->LOGFILENAME, "begin accept_request");
	std::vector<std::string> requestContent =  this->getRequestContent();

	//打印request所有信息
	for (auto& rC : requestContent) this->pmLog->print(this->LOGFILENAME, rC);

	if (requestContent.empty())
		this->pmLog->print(this->LOGFILENAME, "Confusing HTTP data: empty");
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
	this->pmLog->print(this->LOGFILENAME, "end accept_request");
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
void acceptRequestThread(unsigned int client)
{
	HttpRequest h(client);
	h.acceptRequestInterface();
	h.closeRequestInterface();
}
