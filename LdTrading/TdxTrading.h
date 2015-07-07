/**
与通达信交易外挂程序通讯接口。
*/

#pragma once
#include "ITradInterface.h"

class CTdxTrading :public ITradInterface
{
public:
	CTdxTrading(void);
	~CTdxTrading(void);

	virtual BOOL StockBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

	virtual BOOL StockSell( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );

};

