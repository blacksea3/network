#include "HttpServer.h"
#include "HttpRequest.h"


/*
 * ���캯��: ��ʼ��HttpServer��, ��ʼ�������
 */
HttpServer::HttpServer():clientSockID(-1), acceptCount(0),
	clientNameLength(sizeof(clientName)), port(DEFAULTPORT)
{
	;
}

/*
 * ��ʼ��������
 * ���ڽ����Ǽ���Ws2_32.dll
 */
void HttpServer::InitServer()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		throw new std::exception("cannot load ws2_32.dll?");
}

/*
 * ��ʼ������
 * ����ʼTCP�����
 */
int HttpServer::StartServer(u_short * port)
{
	//int httpd = 0;
	struct sockaddr_in name;

	//httpd = socket(PF_INET, SOCK_STREAM, 0);
	SOCKET httpd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (httpd == INVALID_SOCKET)
		throw new std::exception("socket create error");
	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(*port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	//��socket
	if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)
		throw new std::exception("bind error");
	//����˿�û�����ã��ṩ������˿�
	if (*port == 0)  /* if dynamically allocating a port */
	{
		socklen_t namelen = sizeof(name);
		if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)
			throw new std::exception("getsockname error");
		*port = ntohs(name.sin_port);
	}
	//����
	if (listen(httpd, 5) < 0)
		throw new std::exception("listen error");
	return httpd;
}

/*
 * ���з�����, �˷�������while(1)ѭ��!
 */
void HttpServer::RunServer()
{
	this->serverSockID = this->StartServer(&this->port);
	//printf("httpd running on port %d\n", port);

	while (1)
	{
		//�������󣬺���ԭ��
		//#include <sys/types.h>
		//#include <sys/socket.h>
		//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		this->clientSockID = accept(this->serverSockID,
			(struct sockaddr *)&this->clientName,
			&this->clientNameLength);
		if (this->clientSockID == -1)
			throw new std::exception("client socket accept error");

		//accept_request((void *)(intptr_t)client_sock);  //���߳�

		 //ÿ���յ����󣬴���һ���߳���������ܵ�������
		 //��client_sockת�ɵ�ַ��Ϊ��������
		std::thread t1(acceptRequestThread, (void *)(intptr_t)this->clientSockID);
		t1.join();
	}
	closesocket(this->serverSockID);
	int r = WSACleanup();
	if (r != 0)
	{
		throw new std::exception("WSACleanup func error");
	}
}

/*
 * Ĭ����������
 */
HttpServer::~HttpServer()
{
	;
}

int main()
{
	HttpServer hs = HttpServer();
	hs.InitServer();
	hs.RunServer();
	return 0;
}
