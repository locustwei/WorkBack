#include "TdxTradSocket.h"
#include "winds\TDXMain.h"

class CTradSocketListenner: public ISocketListener
{
public:

	CTdxTradSocket* m_TradSocket;

	virtual void OnConnected(PLD_CLIENT_SOCKET pSocket)
	{
		int nSize = 0;
		PTDX_SOCKET_DATA pData = MakeStockData(NULL, TF_REGISTER, nSize);
		m_TradSocket->Send((char*)pData, nSize, pSocket);
	}

	virtual void OnRecv(PLD_CLIENT_SOCKET pSocket)
	{
		BOOL result = FALSE;
		PS_TDX_STOCK_BY pBy;

		if(pSocket->nRecvSize==0 || pSocket->lpRecvedBuffer==NULL)
			return;

		PTDX_SOCKET_DATA pData = (PTDX_SOCKET_DATA)pSocket->lpRecvedBuffer;
		switch(pData->ID){
		case TF_REGISTER:
			pSocket->tag = 1;  //验证成功 //todo
			m_TradSocket->m_bAvailable = TRUE;
			break;
		case TF_STOCKBY:
			pBy = (PS_TDX_STOCK_BY)pData->data;
			if(CTDXMain::WndHooker!=NULL){
				result = CTDXMain::WndHooker->DoStockBy(pBy->mark, pBy->Code, pBy->fPrice, pBy->dwVolume);
			}
			break;
		case TF_STOCKBYED:
			break;
		case TF_STOCKSEL:
			break;
		case TF_STOCKSELED:
			break;
		}
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

CTdxTradSocket::CTdxTradSocket(void)
{
	m_bAvailable = FALSE;

	SetListener(new CTradSocketListenner());

	ConnectCtrlor();
}


CTdxTradSocket::~CTdxTradSocket(void)
{
	if(m_Listner)
		delete m_Listner;
}

void CTdxTradSocket::ConnectCtrlor()
{
	int i=0;
	do{ //尝试3次连接TDX交易软件。
		if(ConnectTo("127.0.0.1", TDX_SOCKET_PORT)){
			m_bAvailable = TRUE;
			return;
		}
	}while(i++<3);

	if(GetStatus()==SS_NONE){ //连接失败
		//不能连接认为是交易软件没有启动，把自己设为监听状态等待连接。
		Listen(TDX_SOCKET_PORT);
	}
}
