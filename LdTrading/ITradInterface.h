#pragma once

#include "stdafx.h"
#include "..\StockDataAPI\IDataInterface.h"

struct ITradInterface
{
	virtual BOOL IsAvailable() = 0;   //是否可用
	virtual BOOL StockBy(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume) = 0;    //买入
	virtual BOOL StockSell(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume) = 0;  //卖出
	//virtual float GetYe() = 0;   //可用余额
};