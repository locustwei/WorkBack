#pragma once

#include "stdafx.h"
#include "..\StockDataAPI\IDataInterface.h"

struct ITradInterface
{
	virtual BOOL IsAvailable() = 0;   // «∑Òø…”√
	virtual BOOL StockBy(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume) = 0;
	virtual BOOL StockSell(STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume) = 0;
};