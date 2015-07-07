#include "..\StdAfx.h"
#include "TdxTrading.h"

class CTdxListenner: public ISocketListener
{
	virtual void OnConnected(PLD_CLIENT_SOCKET)
	{
		
	}

	virtual void OnRecv(PLD_CLIENT_SOCKET)
	{
		
	}

	virtual void OnClosed(PLD_CLIENT_SOCKET)
	{
		
	}

	virtual void OnAccept(PLD_CLIENT_SOCKET)
	{
		
	}

	virtual void OnError(PLD_CLIENT_SOCKET, int)
	{
		
	}

};

CTdxTrading::CTdxTrading(void)
{
	m_bAvailable = FALSE;

	m_Socket = new CLdSocket();
	m_Listenner = new CTdxListenner();
	m_Socket->SetListener(m_Listenner);

	ConnectTdx();
}


CTdxTrading::~CTdxTrading(void)
{
	if(m_Socket)
		delete m_Socket;
	if(m_Listenner)
		delete m_Listenner;
}

BOOL CTdxTrading::StockBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume )
{
	
	return FALSE;
}

BOOL CTdxTrading::StockSell( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume )
{
	return FALSE;
}

BOOL CTdxTrading::Available()
{
	return m_bAvailable;
}

void CTdxTrading::ConnectTdx()
{
	int i=0;
	do{ //尝试3次连接TDX交易软件。
		if(m_Socket->ConnectTo("127.0.0.1", TDX_SOCKET_PORT)){
			m_bAvailable = TRUE;
			return;
		}
	}while(i++<3);

	if(m_Socket->GetStatus()==SS_NONE){ //连接失败
		//不能连接认为是交易软件没有启动，把自己设为监听状态等待连接。
		m_Socket->Listen(TDX_SOCKET_PORT);
	}
}
