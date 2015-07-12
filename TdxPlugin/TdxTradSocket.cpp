#include "TdxTradSocket.h"
#include "winds\TDXMain.h"

class CTradSocketListenner: public ISocketListener
{
public:

	CTdxTradSocket* m_TradSocket;

	virtual void OnConnected(PLD_CLIENT_SOCKET pSocket)
	{
		m_TradSocket->SendStockData(TF_REGISTER, NULL, 0);
	}

	virtual void OnRecv(PLD_CLIENT_SOCKET pSocket)
	{
		if(pSocket->nRecvSize==0 || pSocket->lpRecvedBuffer==NULL)
			return;

		PTDX_SOCKET_DATA pData = (PTDX_SOCKET_DATA)pSocket->lpRecvedBuffer;
		PS_TDX_STOCK_BY pBy;
		BOOL result = FALSE;
		switch(pData->ID){
		case TF_REGISTER:
			pSocket->tag = 1;  //验证成功 //todo
			m_TradSocket->m_bAvailable = TRUE;
			break;
		case TF_STOCKBY:
			pBy = (PS_TDX_STOCK_BY)pData->data;
			if(CTDXMain::WndHooker!=NULL){
				PostMessage(CTDXMain::WndHooker->m_hWnd, MM_TDXSOCKET, TF_STOCKBY, (LPARAM)pBy);
			}
			break;
		case TF_STOCKBYED:
			break;
		case TF_STOCKSEL:
			pBy = (PS_TDX_STOCK_BY)pData->data;
			if(CTDXMain::WndHooker!=NULL){
				result = CTDXMain::WndHooker->DoStockBy(pBy->mark, pBy->Code, pBy->fPrice, pBy->dwVolume);
			}
			m_TradSocket->SendStockData(TF_STOCKBYED, &result, sizeof(result));
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
	CTradSocketListenner* listener = new CTradSocketListenner();
	listener->m_TradSocket = this;
	SetListener(listener);

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

BOOL CTdxTradSocket::SendStockData(TDX_TRAD_FUN fID, LPVOID pData, int nSize)
{
	PTDX_SOCKET_DATA pTdxData = MakeStockData(pData, fID, nSize);

	PLD_CLIENT_SOCKET pSocket = GetActiveSocket();

	BOOL result = Send((char*)pTdxData, nSize, pSocket)==nSize;

	free(pTdxData);

	return result;
}

PLD_CLIENT_SOCKET CTdxTradSocket::GetActiveSocket()
{
	if(GetStatus()==SS_LISTENING && GetClientHead()!=NULL){  //作为服务端
		return GetClientHead();
	}else if(GetStatus()==SS_CONNECTED)     //作为客户端
		return this;
	return NULL;
}
