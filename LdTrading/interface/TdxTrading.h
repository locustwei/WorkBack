/**
与通达信交易外挂程序通讯接口。
*/

#pragma once
#include "..\ITradInterface.h"
#include "..\..\PublicLib\comps\LdSocket.h"

#define TDX_SOCKET_PORT 0x3421

class CTdxTrading :public ITradInterface
{
public:
	CTdxTrading(void);
	~CTdxTrading(void);

	virtual BOOL StockBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL StockSell( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL Available();
	void ConnectTdx();
private:
	BOOL m_bAvailable;
	CLdSocket* m_Socket;
	ISocketListener* m_Listenner;
};

