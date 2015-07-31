#include "..\StdAfx.h"
#include "HttpStockData.h"
#include "ApiAddress.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "Wininet.h"  
#pragma comment(lib,"Wininet.lib")  

#pragma warning(disable : 4244)

int HttpRequest(LPCSTR lpHostName, LPCSTR lpUrl, LPSTR lpResult, 
	LPCSTR lpMethod = NULL, LPVOID lpPostData = NULL, int nPostDataLen = 0, short sPort=INTERNET_DEFAULT_HTTP_PORT)
{  
	HINTERNET hInternet = NULL, hConnect = NULL, hRequest = NULL;  
	int len = 0;
	do 
	{
		hInternet = (HINSTANCE)InternetOpen("User-Agent",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);  
		if(!hInternet)  
			break;  

		hConnect = (HINSTANCE)InternetConnect(hInternet,lpHostName,sPort,NULL,"HTTP/1.1",INTERNET_SERVICE_HTTP,0,0);  
		if(!hConnect)  
			break;

		hRequest = (HINSTANCE)HttpOpenRequest(hConnect,lpMethod,lpUrl,"HTTP/1.1",NULL,NULL,INTERNET_FLAG_RELOAD,0);  
		if(!hRequest)  
			break;

		//bRet = HttpAddRequestHeaders(hRequest,"Content-Type: application/x-www-form-urlencoded",Len(FORMHEADERS),HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);  
		//if(!bRet)  
		//goto Ret0;  

		BOOL bRet = HttpSendRequest(hRequest,NULL,0,lpPostData,nPostDataLen);  
		while(bRet && lpResult)  
		{  
			BYTE cReadBuffer[1024] = {0};  
			unsigned long lNumberOfBytesRead;  
			bRet = InternetReadFile(hRequest,cReadBuffer,sizeof(cReadBuffer) - 1,&lNumberOfBytesRead);  
			if(!bRet || !lNumberOfBytesRead)  
				break;  
			CopyMemory(lpResult+len, cReadBuffer, lNumberOfBytesRead);  
			len += lNumberOfBytesRead;
		}  

	} while (FALSE);

	if(hRequest)  
		InternetCloseHandle(hRequest);  
	if(hConnect)  
		InternetCloseHandle(hConnect);  
	if(hInternet)  
		InternetCloseHandle(hInternet);  

	return len;  
}  

CHttpStockData::CHttpStockData(void)
{
	//time(&m_lastTime);
}


CHttpStockData::~CHttpStockData(void)
{
}

int SplitStr(LPSTR szSource, int nPos, char spi, char end, LPCSTR* list, int max)
{
	int result = 0;
	list[result] = (szSource + nPos);
	while(szSource[nPos]!=0 && szSource[nPos]!=end){
		if(szSource[nPos]==spi){
			szSource[nPos++] = 0;
			list[++result] = szSource + nPos;
			if(result==max)
				break;
		}else
			nPos++;
	}
	szSource[nPos]=0;

	return result;
}

LPSTR GetStockPKFromUrl(LPCSTR szHost, LPCSTR szUrlT, PSTOCK_DATA_PK* pStockData, int count)
{
	LPSTR szCodes = NULL;  //股票代码拼接
	LPSTR szUrl = NULL;  //获取数据的网络地址
	LPSTR szResult = NULL;  //Http返回数据

	//生成获取数据的网址Url
	//一次获取多个股票的数据，股票代码拼接起来“，”分开
	int memSize = count*10;   //sz000858,sz000058,
	szCodes = (LPSTR)malloc(memSize); 
	memset(szCodes, 0, memSize);
	for (int i=0; i<count; i++){
		strcat_s(szCodes, memSize, QQ_MARK_FH[pStockData[i]->mark]);
		strcat_s(szCodes, memSize, pStockData[i]->szSymbol);
		strcat_s(szCodes, memSize, ",");
	}
	szCodes[memSize-1] = 0;
	memSize += strlen(szUrlT);
	szUrl = (LPSTR)malloc(memSize);
	memset(szUrl, 0, memSize);
	sprintf_s(szUrl, memSize, szUrlT, szCodes);

	//发生Http请求
	memSize = count * 1024;
	szResult = (LPSTR)malloc(memSize);
	memset(szResult, 0, memSize);
	int len = HttpRequest(szHost, szUrl, szResult);

	if(szCodes)
		free(szCodes);
	if(szUrl)
		free(szUrl);

	if(len==0){
		free(szResult);
		szResult = NULL;
	}

	return szResult;
}

BOOL CHttpStockData::GetStockPKFromTencen( _Inout_ PSTOCK_DATA_PK* pStockData, int count )
{
	LPSTR* szDatas = NULL;

	LPSTR szResult = GetStockPKFromUrl(QQ_STOCK_ADDRESS, QQ_STOCK_CODE, pStockData, count);
	//分解Http返回数据
	if(szResult){
		szDatas = (LPSTR*)malloc(sizeof(LPSTR)*count);
		memset(szDatas, 0, sizeof(LPSTR)*count);

		//第一次分解，找到每个股票数据对应起始地址
		char sub[9] = {0};
		for(int i=0; i<count; i++){
			sprintf_s(sub, 9, "%s%s", QQ_MARK_FH[pStockData[i]->mark], pStockData[i]->szSymbol);
			szDatas[i] = strstr(szResult, sub);
		}
		//第二次分解，分解出每个字段。
		for(int i=0; i<count; i++){
			if(!szDatas[i])
				continue;

			LPCSTR szValues[50] = {0};

			if(SplitStr(szDatas[i], 10, '~', '\"', szValues, 49)!=49)
				continue;

			pStockData[i]->fCurrent = atof(szValues[3]);
			pStockData[i]->fClose =   atof(szValues[4]);
			pStockData[i]->fOpen =    atof(szValues[5]);
			pStockData[i]->fHigh =    atof(szValues[33]);
			pStockData[i]->fLow =     atof(szValues[34]);
			pStockData[i]->fAmount =  atof(szValues[37])*10000;
			pStockData[i]->dwVolume = atoi(szValues[6]);

			pStockData[i]->fBuyp[0] = atof(szValues[9]);
			pStockData[i]->fBuyp[1] = atof(szValues[11]);
			pStockData[i]->fBuyp[2] = atof(szValues[13]);
			pStockData[i]->fBuyp[3] = atof(szValues[15]);
			pStockData[i]->fBuyp[4] = atof(szValues[17]);

			pStockData[i]->dwBuyv[0] = atoi(szValues[10]);
			pStockData[i]->dwBuyv[1] = atoi(szValues[12]);
			pStockData[i]->dwBuyv[2] = atoi(szValues[14]);
			pStockData[i]->dwBuyv[3] = atoi(szValues[16]);
			pStockData[i]->dwBuyv[4] = atoi(szValues[18]);

			pStockData[i]->fSellp[0] = atof(szValues[19]);
			pStockData[i]->fSellp[1] = atof(szValues[21]);
			pStockData[i]->fSellp[2] = atof(szValues[23]);
			pStockData[i]->fSellp[3] = atof(szValues[25]);
			pStockData[i]->fSellp[4] = atof(szValues[27]);

			pStockData[i]->dwSellv[0] = atoi(szValues[20]);
			pStockData[i]->dwSellv[1] = atoi(szValues[22]);
			pStockData[i]->dwSellv[2] = atoi(szValues[24]);
			pStockData[i]->dwSellv[3] = atoi(szValues[26]);
			pStockData[i]->dwSellv[4] = atoi(szValues[28]);
		}
	}

	if(szResult)
		free(szResult);
	if(szDatas)
		free(szDatas);

	return szResult!=NULL;
}

BOOL CHttpStockData::GetStockPKFromSina( _Inout_ PSTOCK_DATA_PK* pStockData, int count )
{
	LPSTR* szDatas = NULL;
	LPSTR szResult = GetStockPKFromUrl(SINA_STOCK_ADDRESS, SINA_STOCK_CODE, pStockData, count);

	if(szResult){
		szDatas = (LPSTR*)malloc(sizeof(LPSTR)*count);
		memset(szDatas, 0, sizeof(LPSTR)*count);

		//第一次分解，找到每个股票数据对应起始地址
		char sub[9] = {0};
		for(int i=0; i<count; i++){
			sprintf_s(sub, 9, "%s%s", QQ_MARK_FH[pStockData[i]->mark], pStockData[i]->szSymbol);
			szDatas[i] = strstr(szResult, sub);
		}
		//第二次分解，分解出每个字段。
		for(int i=0; i<count; i++){
			LPCSTR szValues[33] = {0};
			if(SplitStr(szDatas[i], 10, ',', '\"', szValues, 32)!=32)
				continue;

			pStockData[i]->fCurrent = atof(szValues[3]);
			pStockData[i]->fClose =   atof(szValues[2]);
			pStockData[i]->fOpen =    atof(szValues[1]);
			pStockData[i]->fHigh =    atof(szValues[4]);
			pStockData[i]->fLow =     atof(szValues[5]);
			pStockData[i]->fAmount =  atof(szValues[9]);
			pStockData[i]->dwVolume = atoi(szValues[8]);

			pStockData[i]->fBuyp[0] = atof(szValues[11]);
			pStockData[i]->fBuyp[1] = atof(szValues[13]);
			pStockData[i]->fBuyp[2] = atof(szValues[15]);
			pStockData[i]->fBuyp[3] = atof(szValues[17]);
			pStockData[i]->fBuyp[4] = atof(szValues[19]);

			pStockData[i]->dwBuyv[0] = atoi(szValues[10]);
			pStockData[i]->dwBuyv[1] = atoi(szValues[12]);
			pStockData[i]->dwBuyv[2] = atoi(szValues[14]);
			pStockData[i]->dwBuyv[3] = atoi(szValues[16]);
			pStockData[i]->dwBuyv[4] = atoi(szValues[18]);

			pStockData[i]->fSellp[0] = atof(szValues[21]);
			pStockData[i]->fSellp[1] = atof(szValues[23]);
			pStockData[i]->fSellp[2] = atof(szValues[25]);
			pStockData[i]->fSellp[3] = atof(szValues[27]);
			pStockData[i]->fSellp[4] = atof(szValues[29]);

			pStockData[i]->dwSellv[0] = atoi(szValues[20]);
			pStockData[i]->dwSellv[1] = atoi(szValues[22]);
			pStockData[i]->dwSellv[2] = atoi(szValues[24]);
			pStockData[i]->dwSellv[3] = atoi(szValues[26]);
			pStockData[i]->dwSellv[4] = atoi(szValues[28]);
		}
	}

	if(szResult)
		free(szResult);
	if(szDatas)
		free(szDatas);

	return szResult != NULL;
}

BOOL CHttpStockData::GetStockPKFromEase( _Inout_ PSTOCK_DATA_PK* pStockData, int count )
{
	return FALSE;
}

BOOL CHttpStockData::GetStockPKFromHexun( _Inout_ PSTOCK_DATA_PK* pStockData, int count )
{
	return FALSE;
}

BOOL CHttpStockData::GetStockPK( _Inout_ PSTOCK_DATA_PK* pStockData, int count )
{
	if(!pStockData || !count)
		return FALSE;

	time(&m_lastTime);
	return GetStockPKFromTencen(pStockData, count);
}

float CHttpStockData::GetCurrent(STOCK_MARK nMark, LPCSTR szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.fCurrent;
	}else
		return -1;
}

float CHttpStockData::GetOpen(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.fOpen;
	}else
		return -1;
}

float CHttpStockData::GetHigh(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.fHigh;
	}else
		return -1;
}

float CHttpStockData::GetLow(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.fLow;
	}else
		return -1;
}

float CHttpStockData::GetClose(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.fClose;
	}else
		return -1;
}

DWORD CHttpStockData::GetVolume(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.dwVolume;
	}else
		return -1;
}

double CHttpStockData::GetAmount(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	STOCK_DATA_SIMPLE SD;;

	if(GetStockSimple(nMark, szSymbol, &SD)){
		if(pSD)
			*pSD = SD;
		return SD.fAmount;
	}else
		return -1;
}

BOOL CHttpStockData::GetStockSimple(STOCK_MARK nMark, const char* szSymbol, PSTOCK_DATA_SIMPLE pSD)
{
	PSTOCK_DATA_PK pk[1];
	pk[0] = new STOCK_DATA_PK;
	pk[0]->mark = nMark;
	pk[0]->szSymbol = szSymbol;

	if(GetStockPK(pk, 1)){
		if(pSD)
			*pSD = pk[0]->smple;
		return TRUE;
	}else
		return FALSE;
}

BOOL CHttpStockData::GetStockDay(_Out_ PSTOCK_DATA_SIMPLE* pStockData, STOCK_MARK nMark, const char* szSymbol, int count, int y, int m, int d)
{
	return FALSE;
}

BOOL CHttpStockData::GetStockWeek(_Out_ PSTOCK_DATA_SIMPLE* pStockData, STOCK_MARK nMark, const char* szSymbol, int count, int y, int w)
{
	return FALSE;
}

BOOL CHttpStockData::GetStockMonth(_Out_ PSTOCK_DATA_SIMPLE* pStockData, STOCK_MARK nMark, const char* szSymbol, int count, int y, int m)
{
	return FALSE;
}

BOOL CHttpStockData::GetStockYear(_Out_ PSTOCK_DATA_SIMPLE* pStockData, STOCK_MARK nMark, const char* szSymbol, int count, int y)
{
	return FALSE;
}
