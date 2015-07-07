#pragma once

#include <time.h>
#include "..\IDataInterface.h"

class CHttpStockData: public IDataInterface
{
public:
	CHttpStockData(void);
	~CHttpStockData(void);

	virtual BOOL GetStockPK(_Inout_ PSTOCK_DATA_PK* pStockData, int count);      //获取股票盘口数据
	virtual float GetCurrent(STOCK_MARK nMark, LPCSTR szCode, PSTOCK_DATA_SIMPLE pSD);
	virtual float GetOpen(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
	virtual float GetHigh(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
	virtual float GetLow(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
	virtual float GetClose(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
	virtual DWORD GetVolume(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
	virtual double GetAmount(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
protected:
	BOOL GetStockPKFromTencen(_Inout_ PSTOCK_DATA_PK* pStockData, int count);  //腾讯网盘口数据
	BOOL GetStockPKFromSina(_Inout_ PSTOCK_DATA_PK* pStockData, int count);    //sina网盘口数据
	BOOL GetStockPKFromEase(_Inout_ PSTOCK_DATA_PK* pStockData, int count);    //网易盘口数据
	BOOL GetStockPKFromHexun(_Inout_ PSTOCK_DATA_PK* pStockData, int count);   //和讯网盘口数据
	BOOL GetStockSimple(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD);
private:
	time_t m_lastTime;
};

