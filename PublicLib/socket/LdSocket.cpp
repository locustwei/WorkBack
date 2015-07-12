#include "LdSocket.h"
#include <malloc.h>

#pragma comment(lib, "ws2_32.lib")

#define RECV_BUFFER_LEN 1024

CLdSocket::CLdSocket(void)
{
	m_Status = SS_NONE;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	m_Socket = INVALID_SOCKET;
	lpRecvedBuffer = NULL;
	nRecvSize = 0;
	bClosed = false;
	pNext = NULL;

	m_ClientHead = NULL;
	m_Listner = NULL;
}


CLdSocket::~CLdSocket(void)
{
	//todo (线程同步问题）
	PLD_CLIENT_SOCKET pClient = m_ClientHead;
	while(m_ClientHead){        //删除客户端
		RemoveClient(m_ClientHead);
	}

	if(m_Socket!=INVALID_SOCKET){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	m_Listner = NULL;
	m_Status = SS_NONE;
}

BOOL CLdSocket::Bind(int port)
{
	if(m_Socket!=INVALID_SOCKET)
		return FALSE;
	m_Socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_Socket==INVALID_SOCKET)
		return FALSE;
	sockaddr_in address = {0};
	address.sin_family = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if(bind(m_Socket, (const sockaddr *)&address, sizeof(address))==SOCKET_ERROR){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	};

	m_Status = SS_BINDED;

	return TRUE;
}

BOOL CLdSocket::Listen(int port)
{
	if(m_Socket!=INVALID_SOCKET)
		return FALSE;
	m_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_Socket==INVALID_SOCKET)
		return FALSE;
	sockaddr_in address = {0};
	address.sin_family = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	if(bind(m_Socket, (const sockaddr *)&address, sizeof(address))==SOCKET_ERROR){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	};

	if(listen(m_Socket, SOMAXCONN)==SOCKET_ERROR){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	if(!StartSelectThread()){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	m_Status = SS_LISTENING;

	return TRUE;
}

BOOL CLdSocket::ConnectTo(LPCSTR szIp, int port)
{
	if(m_Socket!=INVALID_SOCKET)
		return FALSE;
	m_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_Socket==INVALID_SOCKET)
		return FALSE;
	sockaddr_in address = {0};
	address.sin_family = PF_INET;
	address.sin_addr.s_addr = inet_addr(szIp);
	address.sin_port = htons(port);
	
	if(connect(m_Socket, (const sockaddr *)&address, sizeof(address))==SOCKET_ERROR){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	u_long nNoBlock = 1;
	ioctlsocket(m_Socket, FIONBIO, &nNoBlock);

	if(!StartSelectThread()){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return FALSE;
	}

	m_Status = SS_CONNECTED;

	if(m_Listner)
		m_Listner->OnConnected(this);

	return TRUE;
}

DWORD WINAPI SocketSelectThreadProc(
	_In_ LPVOID lpParameter
	)
{
	fd_set ReadSet, WriteSet, ExceptSet;
	CLdSocket* ldSocket = (CLdSocket*)lpParameter;
	while(true){
		if(ldSocket->m_Socket == INVALID_SOCKET)
			break;

		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);        //不处理send
		FD_ZERO(&ExceptSet);

		FD_SET(ldSocket->m_Socket, &ReadSet);
		FD_SET(ldSocket->m_Socket, &ExceptSet);

		//todo (线程同步问题）
		PLD_CLIENT_SOCKET pClient = ldSocket->GetClientHead();
		while(pClient){
			if(!pClient->bClosed){
				FD_SET(pClient->m_Socket, &ReadSet);
				FD_SET(pClient->m_Socket, &ExceptSet);
			}
			pClient = pClient->pNext;
		}

		if (select(0, &ReadSet, NULL, &ExceptSet, 0) > 0){
			if(FD_ISSET(ldSocket->m_Socket, &ReadSet)){
				ldSocket->DoRead();
			}

			if(FD_ISSET(ldSocket->m_Socket, &ExceptSet)){
				ldSocket->DoClientExcept(ldSocket);
			}
			pClient = ldSocket->GetClientHead();
			while(pClient){
				if(!pClient->bClosed){
					if(FD_ISSET(pClient->m_Socket, &ReadSet)){
						ldSocket->DoClientRead(pClient);
					}

					if(FD_ISSET(ldSocket->m_Socket, &ExceptSet)){
						ldSocket->DoClientExcept(pClient);
					}
				}
				pClient = pClient->pNext;
			}

			pClient = ldSocket->GetClientHead();
			while(pClient){  //删除已经断开连接的客户端
				if(pClient->bClosed){
					PLD_CLIENT_SOCKET pTmp = pClient->pNext;
					ldSocket->RemoveClient(pClient);
					pClient = pTmp;
				}else
					pClient = pClient->pNext;
			}
		}

		
	}
	ldSocket->m_hSelectThread = NULL;
	return 0;
}

BOOL CLdSocket::StartSelectThread()
{
	HANDLE result = NULL;
	DWORD dwThreadId = 0;
	result = CreateThread(NULL, 0, &SocketSelectThreadProc, this, 0, &dwThreadId);
	m_hSelectThread = result;
	return result!=NULL;
}

PLD_CLIENT_SOCKET CLdSocket::AddClient(SOCKET s)
{
	if(s==INVALID_SOCKET)
		return m_ClientHead;

	u_long nNoBlock = 1;
	ioctlsocket(s, FIONBIO, &nNoBlock);

	PLD_CLIENT_SOCKET pClient = new LD_CLIENT_SOCKET;
	ZeroMemory(pClient, sizeof(LD_CLIENT_SOCKET));
	pClient->m_Socket = s;
	pClient->pNext = m_ClientHead;
	m_ClientHead = pClient;
	return m_ClientHead;
}

PLD_CLIENT_SOCKET CLdSocket::GetClientHead()
{
	return m_ClientHead;
}

void CLdSocket::DoRead()
{
	if(m_Status == SS_LISTENING)
		return DoAccept();
	else
		return DoClientRead(this);
}

void CLdSocket::DoClientRead(PLD_CLIENT_SOCKET pClient)
{
	int n = RecvData(pClient);
	if(n == SOCKET_ERROR){
		DoClientExcept(pClient);
	}else if(n == 0)
		DoClientClosed(pClient);
	else if(m_Listner)
		m_Listner->OnRecv(pClient);
}

void CLdSocket::DoClientExcept(PLD_CLIENT_SOCKET pClient)
{
	int err = WSAGetLastError();
	switch(err){
	case WSAECONNRESET:
		DoClientClosed(pClient);
		break;
	default:
		if(m_Listner){
			m_Listner->OnError(pClient, err);
		};
		break;
	}
}

void CLdSocket::DoAccept()
{
	sockaddr_in ClientAddress;
	int nClientLength = sizeof(ClientAddress);

	SOCKET Socket = accept(m_Socket, (sockaddr*)&ClientAddress, &nClientLength);

	if (INVALID_SOCKET == Socket){
		DoClientExcept(this);
		return;
	}
	AddClient(Socket);
	if(m_Listner)
		m_Listner->OnAccept(m_ClientHead);
}

int CLdSocket::RecvData(PLD_CLIENT_SOCKET pClient)
{
	//todo 线程同步问题

	int nBytes = 0, nTotal = 0;;
	if(pClient->lpRecvedBuffer)
		ZeroMemory(pClient->lpRecvedBuffer, pClient->nRecvSize);
	do 
	{
		char buffer[RECV_BUFFER_LEN] = {0};
		nBytes = recv(pClient->m_Socket, buffer, RECV_BUFFER_LEN, 0);
		if(nBytes>0){
			if(nTotal+nBytes>pClient->nRecvSize){
				pClient->lpRecvedBuffer = (char*)realloc(pClient->lpRecvedBuffer, nTotal+nBytes);
			}
			CopyMemory(pClient->lpRecvedBuffer+nTotal, buffer, nBytes);
			nTotal += nBytes;
		}
	} while (nBytes>=RECV_BUFFER_LEN);

	pClient->nRecvSize = nTotal;

	if(nBytes==SOCKET_ERROR)
		return SOCKET_ERROR;
	if(nTotal==0)
		return 0;
	return nTotal;
}

void CLdSocket::DoClientClosed(PLD_CLIENT_SOCKET pClient)
{
	if(pClient->m_Socket!=INVALID_SOCKET){
		closesocket(pClient->m_Socket);
		pClient->m_Socket = INVALID_SOCKET;
		pClient->bClosed = true;
	}
	if(m_Listner)
		m_Listner->OnClosed(pClient);
}

void CLdSocket::SetListener(ISocketListener* listener)
{
	m_Listner = listener;
}

int CLdSocket::Send(char* buffer, int nSize, PLD_CLIENT_SOCKET pClient)
{
	if(pClient == NULL || pClient->m_Socket == INVALID_SOCKET || !buffer || nSize == 0)
		return 0;

	int nCount = 0;
	do 
	{
		if(pClient)
			nCount = send(pClient->m_Socket, buffer, nSize, 0);
		else
			nCount = send(m_Socket, buffer, nSize, 0);
		if(nCount==SOCKET_ERROR){
			DoClientExcept(this);
			break;
		}else{
			buffer += nCount;
			nSize -= nCount;
		}

	} while (nSize>0);
	return nCount;
}

void CLdSocket::RemoveClient(PLD_CLIENT_SOCKET pClient)
{
	if(!pClient)
		return;

	if(pClient==m_ClientHead){
		m_ClientHead = m_ClientHead->pNext;
	}else{
		PLD_CLIENT_SOCKET pTmp = m_ClientHead;
		while(pTmp && pTmp->pNext!=pClient)
			pTmp = pTmp->pNext;
		if(!pTmp)
			return;
		pTmp->pNext = pClient->pNext;
	}
	if(pClient->m_Socket!=INVALID_SOCKET)
		closesocket(pClient->m_Socket);
	if(pClient->lpRecvedBuffer)
		delete pClient->lpRecvedBuffer;
	delete pClient;
}

void CLdSocket::Close()
{
	if(m_Socket!=INVALID_SOCKET){
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}

SOCKET_STATUS CLdSocket::GetStatus()
{
	return m_Status;
}

ISocketListener* CLdSocket::GetListener()
{
	return m_Listner;
}
