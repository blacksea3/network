#include "HttpServer.h"
#include "HttpRequest.h"


/*
 * ���캯��: ��ʼ��HttpServer��, ��ʼ�������
 */
HttpServer::HttpServer():port(DEFAULTPORT)
{
	;
}

/*
 * ��ʼ��������
 * ����Ws2_32.dll, ������ĿĿ¼
 */
void HttpServer::InitServer()
{
	//��ʼ���׽��ֿ�
	WORD w_req = MAKEWORD(2, 2);//�汾��
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		std::cout << "��ʼ���׽��ֿ�ʧ�ܣ�" << std::endl;
	}
	else {
		std::cout << "��ʼ���׽��ֿ�ɹ���" << std::endl;
	}
	//���汾��
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		std::cout << "�׽��ֿ�汾�Ų�����" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "�׽��ֿ�汾��ȷ��" << std::endl;
	}
	//������˵�ַ��Ϣ

	if (useParentDir)
	{
		std::filesystem::path parentFullPath = std::filesystem::absolute(std::filesystem::path("../"));
		std::filesystem::current_path(parentFullPath);
	}
}

/*
 * ��ʼ������
 * ����ʼTCP�����
 */
SOCKET HttpServer::StartServer(u_short port)
{
	//���峤�ȱ���
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//���巢�ͻ������ͽ��ܻ�����
	char send_buf[100];
	char recv_buf[100];
	//���������׽��֣����������׽���
	SOCKET s_server;
	SOCKET s_accept;
	//����˵�ַ�ͻ��˵�ַ
	SOCKADDR_IN server_addr;
	SOCKADDR_IN accept_addr;


	//���������Ϣ
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	//�����׽���
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		std::cout << "�׽��ְ�ʧ�ܣ�" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "�׽��ְ󶨳ɹ���" << std::endl;
	}
	//�����׽���Ϊ����״̬
	if (listen(s_server, SOMAXCONN) < 0) {
		std::cout << "���ü���״̬ʧ�ܣ�" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "���ü���״̬�ɹ���" << std::endl;
	}
	std::cout << "��������ڼ������ӣ����Ժ�...." << std::endl;
	return s_server;
}

/*
 * ���з�����, �˷�������while(1)ѭ��!
 */
void HttpServer::RunServer()
{
	//���峤�ȱ���
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//���巢�ͻ������ͽ��ܻ�����
	char send_buf[100];
	char recv_buf[100];
	//���������׽��֣����������׽���
	SOCKET ss_server;
	SOCKET s_accept;
	SOCKADDR_IN server_addr, accept_addr;


	ss_server = this->StartServer(8868);
	//������������
	len = sizeof(SOCKADDR);

	while (1)
	{
		s_accept = accept(ss_server, (SOCKADDR *)&accept_addr, &len);
		if (s_accept == SOCKET_ERROR) {
			std::cout << "����ʧ�ܣ�" << std::endl;
			WSACleanup();
			continue;
		}
		std::cout << "���ӽ�����׼����������" << std::endl;
		//acceptRequestThread(s_accept);  //���߳�
		//��������
		while (1) {
			recv_len = recv(s_accept, recv_buf, 100, 0);
			if (recv_len < 0) {
				std::cout << "����ʧ�ܣ�" << std::endl;
				break;
			}
			else if (recv_len == 0)
			{
				std::cout << "û����Ϣ��, 0����" << std::endl;
				break;
			}
			else {
				std::cout << "�ͻ�����Ϣ:" << recv_buf << std::endl;
			}
			if (recv_len < 100)
			{
				std::cout << "û����Ϣ��, " << recv_len << "����" << std::endl;
				break;
			}
			//std::cout << "������ظ���Ϣ:";
			//std::cin >> send_buf;
			//send_len = send(s_accept, send_buf, 100, 0);
			//if (send_len < 0) {
			//	std::cout << "����ʧ�ܣ�" << std::endl;
			//	break;
			//}
		}
		//�ر��׽���
		closesocket(ss_server);
	}
	closesocket(s_accept);
	//�ͷ�DLL��Դ
	WSACleanup();






	/*
	while (true)
	{
		s_accept = accept(s_server, (SOCKADDR *)&accept_addr, &len);
		if (s_accept == SOCKET_ERROR)
		{
			std::cout << "����ʧ�ܣ�" << std::endl;
			WSACleanup();
			return;
		}
		std::cout << "���ӽ�����׼����������" << std::endl;
		acceptRequestThread(s_accept);  //���߳�
		
		//��������
		while (1) {
			recv_len = recv(s_accept, recv_buf, 100, 0);
			if (recv_len < 0) {
				std::cout << "����ʧ�ܣ�" << std::endl;
				break;
			}
			else {
				std::cout << "�ͻ�����Ϣ:" << recv_buf << std::endl;
			}
			std::cout << "������ظ���Ϣ:";
			std::cin >> send_buf;
			send_len = send(s_accept, send_buf, 100, 0);
			if (send_len < 0) {
				std::cout << "����ʧ�ܣ�" << std::endl;
				break;
			}
		}
		
		closesocket(s_accept);
	}
	*/
	/*
	while (1)
	{
		//�������󣬺���ԭ��
		//#include <sys/types.h>
		//#include <sys/socket.h>
		//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		this->clientSockID = accept(this->serverSockID, (SOCKADDR *)&accept_addr, &len);
		if (this->clientSockID == SOCKET_ERROR)
			throw new std::exception("����ʧ��!");

		acceptRequestThread((void *)(intptr_t)this->clientSockID);  //���߳�

		 //ÿ���յ����󣬴���һ���߳���������ܵ�������
		 //��client_sockת�ɵ�ַ��Ϊ��������
		//std::thread t1(acceptRequestThread, (void *)(intptr_t)this->clientSockID);
		//t1.join();   //�ȴ�t1����
	}
	closesocket(this->serverSockID);
	*/
}

void HttpServer::BIGRunServer()
{
	//���峤�ȱ���
	int send_len = 0;
	int recv_len = 0;
	int len = 0;
	//���巢�ͻ������ͽ��ܻ�����
	char send_buf[100];
	char recv_buf[100];
	//���������׽��֣����������׽���
	SOCKET s_server;
	SOCKET s_accept;
	//����˵�ַ�ͻ��˵�ַ
	SOCKADDR_IN server_addr;
	SOCKADDR_IN accept_addr;


	//���������Ϣ
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8070);
	//�����׽���
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (bind(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		std::cout << "�׽��ְ�ʧ�ܣ�" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "�׽��ְ󶨳ɹ���" << std::endl;
	}
	//�����׽���Ϊ����״̬
	if (listen(s_server, SOMAXCONN) < 0) {
		std::cout << "���ü���״̬ʧ�ܣ�" << std::endl;
		WSACleanup();
	}
	else {
		std::cout << "���ü���״̬�ɹ���" << std::endl;
	}
	std::cout << "��������ڼ������ӣ����Ժ�...." << std::endl;
	//������������
	len = sizeof(SOCKADDR);
	s_accept = accept(s_server, (SOCKADDR *)&accept_addr, &len);
	if (s_accept == SOCKET_ERROR) {
		std::cout << "����ʧ�ܣ�" << std::endl;
		WSACleanup();
		return;
	}
	std::cout << "���ӽ�����׼����������" << std::endl;
	//��������
	while (1) {
		recv_len = recv(s_accept, recv_buf, 100, 0);
		if (recv_len < 0) {
			std::cout << "����ʧ�ܣ�" << std::endl;
			break;
		}
		else {
			std::cout << "�ͻ�����Ϣ:" << recv_buf << std::endl;
		}
		//std::cout << "������ظ���Ϣ:";
		//std::cin >> send_buf;
		//send_len = send(s_accept, send_buf, 100, 0);
		//if (send_len < 0) {
		//	std::cout << "����ʧ�ܣ�" << std::endl;
		//	break;
		//}
	}
	//�ر��׽���
	closesocket(s_server);
	closesocket(s_accept);
	//�ͷ�DLL��Դ
	WSACleanup();
}

/*
 * Ĭ����������
 */
HttpServer::~HttpServer()
{
	;
}

/*
int main()
{	
	
	HttpServer hs = HttpServer();
	hs.InitServer();
	hs.BIGRunServer();
	return 0;
	
	HttpServer hs = HttpServer();
	hs.InitServer();
	hs.RunServer();
	return 0;
}*/

