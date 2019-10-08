#include "HttpRequest.h"
#include "HttpServer.h"
#include "HttpStringParser.h"

/*
 * ��ȡ��ǰ��������·��
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
 * �õ�һ������,ֻҪ����cΪ\n,����Ϊ��һ�н������������\r,����MSG_PEEK�ķ�ʽ����һ���ַ��������\n����socket�ö���
 * ������¸��ַ��򲻴�����c��Ϊ\n���������������������Ϊ0�жϣ�����С��0��Ҳ��Ϊ������c��Ϊ\n
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
				//͵��һ���ֽڣ������\n�Ͷ���
				n = recv(sock, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					recv(sock, &c, 1, 0);
				else
					//����\n��������һ�е��ַ�������û��������cΪ\n ����ѭ��,���һ�ж�ȡ
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
 * ͨ������
 * s: ��ѡ�ַ���, ���ڷ�200��Ӧ��Ĭ�Ϸ�����Ϣ, ����200��Ӧ���ļ�·��
 *  , isRenderFile: �Ƿ����ļ�����, (enum)en��������
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
 * �����ļ�����
 * ����true˵���ļ����ڷ��ͳɹ�, false˵���ļ�������, ��Ҫ���������д���content����
 *    ע��: �˴��õ�����Ŀ¼, filename �������¸�ʽhtml/404.html
 *    ���: ��filesystem���ʽ�����Ŀ¼, ����C api�����ļ���д
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
		//ѭ����
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
 * HttpRequest���ʼ��, ��ʼ���ͻ���SocketID
 * ��ʼ����ĿĿ¼
 */
HttpRequest::HttpRequest(int c) :clientSocketID(c)
{
	pmLog = Mlog::Instance();
}

/*
 * ���������ܽӿ�
 */
void HttpRequest::acceptRequestInterface()
{
	//��ȡ��������
	pmLog->print(this->filename, "                                 ");
	pmLog->print(this->filename, "=================================");
	pmLog->print(this->filename, "begin accept_request");
	std::vector<std::string> requestContent =  this->getRequestContent();

	//��ӡrequest������Ϣ
	for (auto& rC : requestContent) pmLog->print(this->filename, rC);

	if (requestContent.empty())
		pmLog->print(this->filename, "Confusing HTTP data: empty");
	else if (PRINT_ALL_RAW_DATA_DEBUG)
		this->NotFound("NOT FOUNDDDDD", true);
	else
	{
		httpMethodStr hms = parserFirstLine(requestContent[0]);   //������һ���ַ���
		if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_OTHER)
		{
			this->MethodNotImplemented("NOT IMPLEMENTED!!", true);
		}
		else if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_GET_PARAERROR)
		{
			this->BadRequest("BAD REQUEST!!", true);
		}
		else if (hms.httpMethod == HTTPMETHOD::HTTPMETHOD_GET_COMMON ||
			hms.httpMethod == HTTPMETHOD::HTTPMETHOD_POST)   //�����ļ�
		{
			//this->NotFound("NOT FOUNDDDDD", true);
			//���hms.dirΪ��, ��ʹ��DEFAULT_FILE
			//�������hms.dir[0] == '/'��ȥ��ͷ��(ʵ����һ����'/')����hms.dir
			//����ֱ����hms.dir
			if (hms.dir.empty()) this->NormalRequest(DEFAULT_FILE, true);
			else if (hms.dir[0] == '/')
			{
				hms.dir.erase(hms.dir.begin());
				this->NormalRequest(hms.dir, true);
			}
			else this->NormalRequest(hms.dir, true);
			//ִ��cgi�ļ�
			//execute_cgi(client, path, method, query_string);
		}
	}
	pmLog->print(this->filename, "end accept_request");
	//ִ����Ϲر�socket
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
 * ���̺߳���
 */
void acceptRequestThread(void *arg)
{
	int client = (intptr_t)arg;
	HttpRequest h(client);
	h.acceptRequestInterface();
	h.closeRequestInterface();
}
