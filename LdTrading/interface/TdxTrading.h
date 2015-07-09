/**
与通达信交易外挂程序通讯接口。
*/

#pragma once
#include "..\ITradInterface.h"
#include "..\..\PublicLib\socket\LdSocket.h"
#include <malloc.h>

#define TDX_SOCKET_PORT 0x3421

enum TDX_TRAD_FUN
{
	TF_REGISTER,        //注册（连接后确认身份）
	TF_STOCKBY,         //股票买入
	TF_STOCKBYED,       //股票买入返回
	TF_STOCKSEL,        //股票卖出
	TF_STOCKSELED       //卖出返回
};

typedef struct _TDX_SOCKET_DATA
{
	TDX_TRAD_FUN ID;
	WORD nLen;
	unsigned char data[0];
}TDX_SOCKET_DATA, *PTDX_SOCKET_DATA;

inline PTDX_SOCKET_DATA MakeStockData(LPVOID p, int n, TDX_TRAD_FUN id, int& nSize)
{
	nSize = sizeof(TDX_SOCKET_DATA)+n;
	PTDX_SOCKET_DATA result = (PTDX_SOCKET_DATA)malloc(nSize);
	ZeroMemory(result, nSize);
	result->ID = id;
	result->nLen = n;
	CopyMemory(result->data, p, n);
	return result;
};

inline PTDX_SOCKET_DATA MakeStockByData(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume, int& nSize)
{
	S_TDX_STOCK_BY tb;
	tb.mark = mark;
	tb.fPrice = fPrice;
	tb.dwVolume = dwVolume;
	strcpy_s(tb.Code, szCode);

	return MakeStockData(&tb, sizeof(tb), TF_STOCKBY, nSize);
};

typedef struct _S_TDX_STOCK_BY
{
	STOCK_MARK mark;
	char Code[7];
	float fPrice;
	DWORD dwVolume;
}S_TDX_STOCK_BY, *PS_TDX_STOCK_BY;

class CTdxTrading :public ITradInterface
{
	friend class CTdxListenner;
public:
	CTdxTrading(void);
	~CTdxTrading(void);

	virtual BOOL StockBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL StockSell( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL IsAvailable();
	void ConnectTdx();
	BOOL SendStockBy(PLD_CLIENT_SOCKET pSocket, STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume);
private:
	BOOL m_bAvailable;
	CLdSocket* m_Socket;
	ISocketListener* m_Listenner;
};

