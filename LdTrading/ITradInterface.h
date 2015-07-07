#pragma once

#include "stdafx.h"
#include "..\StockDataAPI\IDataInterface.h"

typedef struct ITradInterface
{
	virtual BOOL Available() = 0;   // «∑Òø…”√
	virtual BOOL StockBy(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume) = 0;
	virtual BOOL StockSell(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume) = 0;
};