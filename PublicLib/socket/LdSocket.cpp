#include "LdSocket.h"


CLdSocket::CLdSocket(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,0), &wsaData);
}


CLdSocket::~CLdSocket(void)
{
}
