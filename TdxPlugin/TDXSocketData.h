/*!
 * Socket 通讯数据定义
 *通达信外挂动态库与主控程序使用Socket进行进程间通信。
 * 此文件定义相关数据结构
 */
#pragma once

#include <Windows.h>
#include <malloc.h>
#include "..\StockDataAPI\IDataInterface.h"

#pragma warning( disable : 4200)

#define TDX_SOCKET_PORT 0x3421


enum TDX_TRAD_FUN
{
	TF_REGISTER,        //注册（连接后确认身份）
	TF_STOCKBY,         //股票买入
	TF_STOCKBY_RET,     //股票买入返回
	TF_STOCKSEL,        //股票卖出
	TF_STOCKSEL_RET,    //卖出返回
	TF_GETZJGF,         //获取持有股票信息
	TF_GETZJGF_RET      //获取持有股票信息返回数据
};

//通达信软件与交易主控软件见的socket通信数据结构
typedef struct _TDX_SOCKET_DATA
{
	TDX_TRAD_FUN ID;   //数据标识（指示data的数据类型）
	WORD nLen;          
	unsigned char data[0];  //根据具体情况而定
}TDX_SOCKET_DATA, *PTDX_SOCKET_DATA;

//买入或卖出
typedef struct _TDX_STOCK_BY
{
	STOCK_MARK mark;
	char Code[7];
	float fPrice;
	DWORD dwVolume;
}TDX_STOCK_BY, *PTDX_STOCK_BY;

//持有股票信息
typedef struct _TDX_STOCK_GF
{
	TCHAR code[7];       //股票代码
	TCHAR name[5];      //证券名称
	DWORD sl;           //证券数量
	DWORD kmsl;         //可卖数量
	DWORD jmsl;         //今买数量
	float ckcbj;        //持仓成本价
	float mrjj;         //买入均价
	float dqj;          //当前价
	float zxsz;         //最新市值
	float ccyk;         //持仓盈亏
	WORD ykbl;          //盈亏比例
	DWORD djsl;         //冻结数量
	//股东代码
	//交易所名称
}TDX_STOCK_GF, *PTDX_STOCK_GF;

//资金总量及持有股份
typedef struct _TDX_STOCK_ZJGF
{
	float ye;     //余额
	float ky;     //可用
	float sz;     //市值
	float yk;     //盈亏
	int count;    //持有股票数
	TDX_STOCK_GF gf[0]; //股份信息 
}TDX_STOCK_ZJGF, *PTDX_STOCK_ZJGF;

inline PTDX_SOCKET_DATA MakeStockData(LPVOID p, TDX_TRAD_FUN id, _Inout_ int& nSize)
{
	int nLen = sizeof(TDX_SOCKET_DATA)+nSize;
	PTDX_SOCKET_DATA result = (PTDX_SOCKET_DATA)malloc(nLen);
	ZeroMemory(result, nLen);
	result->ID = id;
	result->nLen = nSize;
	if(p)
		CopyMemory(result->data, p, nSize);
	nSize = nLen;
	return result;
};
