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

/*
 * ͨ����Ӧ, 200
 * s: ��ѡ�ַ���, isRenderFile: �Ƿ����ļ�����
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
 * �Ҳ���
 * s: ��ѡ�ַ���, isRenderFile: �Ƿ����ļ�����
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
 * �������
 * s: ��ѡ�ַ���, isRenderFile: �Ƿ����ļ�����
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
 * ����δʵ��
 * s: ��ѡ�ַ���, isRenderFile: �Ƿ����ļ�����
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
 * �ڲ�����
 * s: ��ѡ�ַ���, isRenderFile: �Ƿ����ļ�����
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
 * �����ļ�����, �ⲿ�Ѿ�ȷ���ļ�����
 */
void HttpRequest::SendFileContent(FILE *resource)
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
}

/*
 * �����ļ�����
 * ����true˵���ļ����ڷ��ͳɹ�, false˵���ļ�������, ��Ҫ���������д���content����
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
 * HttpRequest���ʼ��, ����ʼ���ͻ���SocketID
 */
HttpRequest::HttpRequest(int c) :clientSocketID(c)
{
	;
}

/*
 * ���������ܽӿ�
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

	//��ȡ��һ��
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

		//��ȡ������Ϣ������
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

		httpMethodStr hms = parserFirstLine(sbuf);   //������һ���ַ���

		//��ȡ������Ϣ������
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
			hms.httpMethod == HTTPMETHOD::HTTPMETHOD_POST)   //�����ļ�
		{
			std::fstream _file;
			std::string dir = GetProgramDir();
			std::replace(hms.dir.begin(), hms.dir.end(), '/', '\\');
			if (hms.dir[0] == '\\') hms.dir.erase(hms.dir.begin());

			std::string fullPath = dir + "\\" + hms.dir;
			this->CommonResponse(fullPath);
			//ִ��cgi�ļ�
			//execute_cgi(client, path, method, query_string);
		}
		file << "end accept_request:" << std::endl << std::endl;
	}

	//ִ����Ϲر�socket
	closesocket(this->clientSocketID);
	file.close();
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
}
