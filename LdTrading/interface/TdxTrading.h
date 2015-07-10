/**
与通达信交易外挂程序通讯接口。
*/

#pragma once
#include "..\ITradInterface.h"
#include "..\..\PublicLib\socket\LdSocket.h"
#include "..\..\TdxPlugin\TDXSocketData.h"

class CTdxTrading :public ITradInterface
{
	friend class CTdxListenner;
public:
	CTdxTrading(void);
	~CTdxTrading(void);

	virtual BOOL StockBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL StockSell( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL IsAvailable();
private:
	BOOL m_bAvailable;
	CLdSocket* m_Socket;
	HANDLE m_hEvent;

	void ConnectTdx();
	BOOL SendStockDataWait(PLD_CLIENT_SOCKET pSocket, PTDX_SOCKET_DATA pData, int nSize);  //发送数据等待回信
	BOOL SendStockData(PLD_CLIENT_SOCKET pSocket, PTDX_SOCKET_DATA pData, int nSize);      //发送数据不等待回信  
	BOOL WaitReturn(DWORD msecond);
};

