#pragma once
//HTTP��������

#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ws2tcpip.h>
#include <thread>

#include "HttpRequest.h"

class HttpServer
{
private:
	const u_short DEFAULTPORT = 8080;				    //Ĭ�϶˿�
	u_short port;                                       //��ǰ�˿�

	int serverSockID;                                   //�����socket ID
	int clientSockID;									//�ͻ���socket ID
	struct sockaddr_in clientName;						//�ͻ�������
	int acceptCount;									//������, ����, ͳ�ƿͻ����������
	socklen_t clientNameLength;							//�ͻ��������ֽڴ�С

	int StartServer(u_short *port);
public:
	HttpServer();
	void InitServer();
	void RunServer();
	~HttpServer();
};
