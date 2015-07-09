#pragma once

#pragma warning(disable : 4091)

typedef enum STOCK_MARK  //股市编号
{
	MARK_SZ = 0,   //深市
	MARK_SH = 1    //沪市
};

typedef struct _STOCK_DATA_SIMPLE
{
	STOCK_MARK  mark;        //市场
	LPCSTR      szCode;
	float       fClose;      //昨收盘价
	float       fOpen;       //开盘价
	float       fHigh;
	float       fLow;
	float       fCurrent;     //现价
	DWORD       dwVolume;     //成交量(手)
	float       fAmount;      //成交金额（元）
}STOCK_DATA_SIMPLE, *PSTOCK_DATA_SIMPLE;

typedef struct _STOCK_DATA_PK   // 股票盘口数据
{
	union{  //为了使用方便
		STOCK_DATA_SIMPLE smple;
		struct{
			STOCK_MARK  mark;        //市场
			LPCSTR      szCode;
			float       fClose;      //昨收盘价
			float       fOpen;       //开盘价
			float       fHigh;
			float       fLow;
			float       fCurrent;     //现价
			DWORD       dwVolume;     //成交量(手)
			double      fAmount;      //成交金额（元）
		};
	};
	
	float       fBuyp[5];     //五个叫买价(元）    
	DWORD       dwBuyv[5];    //对应五个叫买价的五个买盘（手）
	float       fSellp[5];    //五个叫卖价
	DWORD       dwSellv[5];   //对应五个叫卖价的五个卖盘
}STOCK_DATA_PK, *PSTOCK_DATA_PK;

struct IDataInterface
{
	virtual BOOL GetStockPK(_Inout_ PSTOCK_DATA_PK* pStockData, int count) = 0;                        //获取股票盘口数据
	virtual float GetCurrent(STOCK_MARK nMark, LPCSTR szCode, PSTOCK_DATA_SIMPLE pSD) = 0;             //当前价
	virtual float GetOpen(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD) = 0;           //开盘价
	virtual float GetHigh(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD) = 0;           //最高价
	virtual float GetLow(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD) = 0;            //最低价
	virtual float GetClose(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD) = 0;          //昨收盘
	virtual DWORD GetVolume(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD) = 0;         //成交手
	virtual double GetAmount(STOCK_MARK nMark, const char* szCode, PSTOCK_DATA_SIMPLE pSD) = 0;        //成交额
};