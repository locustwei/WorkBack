//
#pragma once

#include <winsock.h>

typedef enum SOCKET_STATUS
{
	SS_NONE,
	SS_CONNECTED,
	SS_BINDED,
	SS_LISTENING
};

typedef struct _LD_CLIENT_SOCKET
{
	SOCKET m_Socket;
	char* lpRecvedBuffer;
	int nRecvSize;
	bool bClosed;
	_LD_CLIENT_SOCKET* pNext;
	DWORD tag;   //使用者自定义数据

}LD_CLIENT_SOCKET, *PLD_CLIENT_SOCKET;

struct ISocketListener //监听接口，处理Socket事件
{
	virtual void OnConnected(PLD_CLIENT_SOCKET) = 0;
	virtual void OnRecv(PLD_CLIENT_SOCKET) = 0;
	virtual void OnClosed(PLD_CLIENT_SOCKET) = 0;
	virtual void OnAccept(PLD_CLIENT_SOCKET) = 0;
	virtual void OnError(PLD_CLIENT_SOCKET, int) = 0;
};


class CLdSocket: public LD_CLIENT_SOCKET
{
	friend DWORD WINAPI SocketSelectThreadProc(_In_ LPVOID lpParameter);
public:
	CLdSocket(void);
	~CLdSocket(void);
	
	BOOL ConnectTo(LPCSTR szIp, int port);                            //连接服务地址（客户端）
	BOOL Listen(int port);                                             //使用TCP协议监听端口（服务端）
	BOOL Bind(int port);                                              //使用UDP协议监听端口（服务端）
	int Send(char* buffer, int nSize, PLD_CLIENT_SOCKET pClient = NULL);  //发送数据
	PLD_CLIENT_SOCKET GetClientHead();                                  //Server 连接的客户端列表。
	void SetListener(ISocketListener* listener);                          //设置监听
	void Close();
	SOCKET_STATUS GetStatus();
	ISocketListener* GetListener();
protected:
	HANDLE m_hSelectThread;
	ISocketListener* m_Listner;
	SOCKET_STATUS m_Status;

	BOOL StartSelectThread();
	PLD_CLIENT_SOCKET m_ClientHead;
	PLD_CLIENT_SOCKET AddClient(SOCKET s);
	void DoRead();
	void DoClientRead(PLD_CLIENT_SOCKET pClient);
	void DoClientExcept(PLD_CLIENT_SOCKET pClient);
	void DoAccept();
	int RecvData(PLD_CLIENT_SOCKET pClient);
	int GetSocketError(SOCKET Socket);
	void DoClientClosed(PLD_CLIENT_SOCKET pClient);
	void RemoveClient(PLD_CLIENT_SOCKET pClient);
};

