#include "..\StdAfx.h"
#include "ScriptEng.h"
#include <stdio.h>

#pragma warning( disable : 4244 4305)

#define LUA_GLOBAL_IDATA "IStockData"
#define LUA_GLOBAL_ITRAD "IStockTrad"

/********************************************************************
嵌入脚本函数：获取股票数据
*********************************************************************/
static int CallDataFunc(lua_State *L)
{
	int nError = lua_getglobal(L, LUA_GLOBAL_IDATA);
	if(!nError)
		return 0;

	int v = lua_tointeger(L, -1);
	IDataInterface* sd = (IDataInterface*)v;

	if(sd==NULL)
		return 0;
	int nParams = lua_gettop(L); /* number of arguments */
	if(nParams==0)
		return 0;
	int nId = lua_tointeger(L, 1); //功能ID

	STOCK_MARK nMark;
	const char* szCode;
	STOCK_DATA_SIMPLE ss = {MARK_SZ,NULL,12.1,12.2,12.3,12.4,12.5,12.6,12.7};

	switch(nId){	
	case 100:
		nMark = (STOCK_MARK)lua_tointeger(L, 2);
		szCode = lua_tostring(L, 3);
		
		sd->GetCurrent(nMark, szCode, &ss);

		lua_pushnumber(L, ss.fClose);
		lua_pushnumber(L, ss.fOpen);
		lua_pushnumber(L, ss.fHigh);
		lua_pushnumber(L, ss.fLow);
		lua_pushnumber(L, ss.fCurrent);
		lua_pushnumber(L, ss.dwVolume);
		lua_pushnumber(L, ss.fAmount);
		return 7;
	}

	return 0;
}

/********************************************************************
嵌入脚本函数：执行股票交易
*********************************************************************/
static int CallTradFunc(lua_State *L)
{
	int nError = lua_getglobal(L, LUA_GLOBAL_ITRAD);
	if(!nError)
		return 0;

	int v = lua_tointeger(L, -1);
	ITradInterface* iTrad = (ITradInterface*)v;

	if(iTrad==NULL)
		return 0;
	int nParams = lua_gettop(L); /* number of arguments */
	if(nParams==0)
		return 0;
	int nId = lua_tointeger(L, 1); //功能ID

	STOCK_MARK nMark;
	const char* szCode;
	BOOL bRet;
	switch(nId){	
	case 200:
		nMark = (STOCK_MARK)lua_tointeger(L, 2);
		szCode = lua_tostring(L, 3);
		bRet = iTrad->StockBy(nMark, szCode, 0, 0);

		lua_pushboolean(L, bRet);
		return 1;
	}

	return 0;
}

/************************************************************************
枚举脚本资源，加载已定义脚本(库函数）
/************************************************************************/
BOOL CALLBACK EnumResScript(
	__in  HMODULE hModule,
	__in  LPCTSTR lpszType,
	__in  LPTSTR lpszName,
	__in  LONG_PTR lParam
	)
{
	HRSRC hRes = FindResource(hModule, lpszName, lpszType);
	if(hRes){
		HGLOBAL hGlobal = LoadResource(hModule, hRes);
		if(hGlobal){
			LPVOID lpResource = LockResource(hGlobal);
			if(lpResource){
				DWORD nSize = SizeofResource(hModule, hRes);
				LPSTR szScript = new char[nSize + 1];
				CopyMemory((LPVOID)szScript, lpResource, nSize);
				szScript[nSize] = 0;
				((CScriptEng*)lParam)->LoadScriptLib(szScript);
				UnlockResource(hGlobal);
				delete szScript;
			}
			//GlobalFree(hGlobal);
		}
	}
	return TRUE;
}

/************************************************************************
枚举脚本资源，加载已定义脚本(交易函数）
/************************************************************************/
BOOL CALLBACK EnumResTrad(
	__in  HMODULE hModule,
	__in  LPCTSTR lpszType,
	__in  LPTSTR lpszName,
	__in  LONG_PTR lParam
	)
{
	HRSRC hRes = FindResource(hModule, lpszName, lpszType);
	if(hRes){
		HGLOBAL hGlobal = LoadResource(hModule, hRes);
		if(hGlobal){
			LPVOID lpResource = LockResource(hGlobal);
			if(lpResource){
				DWORD nSize = SizeofResource(hModule, hRes);
				LPSTR szScript = new char[nSize + 1];
				CopyMemory((LPVOID)szScript, lpResource, nSize);
				szScript[nSize] = 0;
				((CScriptEng*)lParam)->LoadScriptTrad(szScript);
				UnlockResource(hGlobal);
				delete szScript;
			}
		}
	}
	return TRUE;
}

CScriptEng::CScriptEng(void):m_TradScripts()
{
	m_hLua = luaL_newstate();
	luaopen_base(m_hLua);
	luaL_openlibs(m_hLua);

	lua_pushcfunction(m_hLua, CallDataFunc);
	lua_setglobal(m_hLua, "CallDataFunc");	

	lua_pushcfunction(m_hLua, CallTradFunc);
	lua_setglobal(m_hLua, "CallTradFunc");	
	//加载脚本资源
	EnumResourceNames(hInstance, L"SCRIPT", &EnumResScript, (LONG_PTR)this);
	EnumResourceNames(hInstance, L"S_TRAD", &EnumResTrad, (LONG_PTR)this);
}


CScriptEng::~CScriptEng(void)
{
	if(m_hLua)
		lua_close(m_hLua);
}

BOOL CScriptEng::RunScript(LPCSTR szScript, float* fResult)
{
	if(!RunScript(szScript))
		return FALSE;
	if(fResult)
		*fResult = lua_tonumber(m_hLua, -1);
	return TRUE;
}

BOOL CScriptEng::RunScript(LPCSTR szScript)
{
	char script[2048] = {0};
	sprintf_s(script, "function TempFunction() \n %s \n end", szScript);

	int nError = luaL_dostring(m_hLua, script);
	if(nError)
		return FALSE;
	nError = lua_getglobal(m_hLua, "TempFunction");  
	nError = lua_pcall(m_hLua, 0, 1, 0);
	if(nError)
		return FALSE;
	else
		return TRUE;
}

BOOL CScriptEng::RunScript(LPCSTR szScript, double* fResult)
{
	if(!RunScript(szScript))
		return FALSE;
	if(fResult)
		*fResult = lua_tonumber(m_hLua, -1);
	return TRUE;
}

BOOL CScriptEng::RunScript(LPCSTR szScript, DWORD* dwResult)
{
	if(!RunScript(szScript))
		return FALSE;
	if(dwResult)
		*dwResult = lua_tointeger(m_hLua, -1);
	return TRUE;
}

BOOL CScriptEng::RunScript(LPCSTR szScript, boolean* bResult)
{
	if(!RunScript(szScript))
		return FALSE;
	if(bResult)
		*bResult = lua_toboolean(m_hLua, -1);
	return TRUE;
}

BOOL CScriptEng::RunScript(LPCSTR szScript, LPCSTR* szResult)
{
	if(!RunScript(szScript))
		return FALSE;
	if(szResult)
		*szResult = lua_tostring(m_hLua, -1);
	return TRUE;
}

BOOL CScriptEng::LoadScriptLib( LPCSTR szLib )
{	
	int nError = luaL_dostring(m_hLua, szLib);
	return nError == 0;
}

BOOL CScriptEng::SetTradInterface( ITradInterface* iInt )
{
	m_IStockTrad = iInt;
	if(m_IStockTrad==NULL)
		lua_pushnil(m_hLua);
	else
		lua_pushinteger(m_hLua, (int)m_IStockTrad);
	lua_setglobal(m_hLua, LUA_GLOBAL_ITRAD);
	return TRUE;
}

BOOL CScriptEng::SetDataInterface( IDataInterface* iInt )
{
	m_IStockData = iInt;
	if(m_IStockData==NULL)
		lua_pushnil(m_hLua);
	else
		lua_pushinteger(m_hLua, (int)m_IStockData);
	lua_setglobal(m_hLua, LUA_GLOBAL_IDATA);
	return TRUE;
}

/************************************************************************
加载交易脚本，脚本头部注释部分定义脚本的意义及参数信息。格式如下
--[[                    --注释开始
[名称]:操底买入         --“[名称]:”为关键字，表示显示函数名称
[说明]:当股票...。      --“[说明]:”为关键字，表示函数的说明，用于描述函数的作业用法等。
[Code]:股票代码。       --“[*]:”为函数参数格式。Code代表某个股票代码在函数中使用。函数调用时由程序赋值。
[nPercent]:回升幅度     --同上
--]]                    --注释结束

以下为脚本代码部分

************************************************************************/
PTRAD_STRCPIT CScriptEng::LoadScriptTrad( LPSTR szScript)
{
	//解析函数注释部分。
	LPSTR szName = NULL,         //函数名称
		szComment = NULL,        //函数说明
		szContent = NULL;        //函数体

	LPSTR* szParams = NULL,      //参数数组
		*szParamComment = NULL;  //参数说明
	int nPs = 0;
	LPSTR tmp = szScript, p = szScript;
	while(*p!=0){
		tmp = strchr(p, '\n');
		if(tmp==NULL)
			goto err;

		int nlen = tmp - p;
		if(strstr(p, S_TRAD_NAME)==p){
			nlen = nlen - strlen(S_TRAD_NAME);
			szName = (LPSTR)malloc(nlen + 1);
			ZeroMemory(szName, nlen + 1);
			memcpy(szName, p + strlen(S_TRAD_NAME), nlen);
		}else if(strstr(p, S_TRAD_MEMO)==p){
			nlen = nlen - strlen(S_TRAD_MEMO);
			szComment = (LPSTR)malloc(nlen + 1);
			ZeroMemory(szComment, nlen + 1);
			memcpy(szComment, p + strlen(S_TRAD_MEMO), nlen);
		}else if(strstr(p, S_COMMENT_E)==p){
			szContent = tmp + 1;
			break;
		}else if(p[0] == '['){
			nPs++;
			LPSTR p1 = strchr(p, ']');
			if(p1){
				int pLen = p1 - p - 1;
				szParams = (LPSTR*)realloc(szParams, nPs*sizeof(LPSTR));
				szParams[nPs-1] = (LPSTR)malloc(pLen + 1);
				memset(szParams[nPs-1], 0, nlen + 1);
				memcpy(szParams[nPs-1], p + 1, pLen);

				szParamComment = (LPSTR*)realloc(szParamComment, nPs*sizeof(LPSTR));
				szParamComment[nPs-1] = (LPSTR)malloc(nlen - pLen + 1);
				memset(szParamComment[nPs-1], 0, nlen -pLen + 1);
				memcpy(szParamComment[nPs-1], p + pLen + 3, nlen - pLen - 4);
			}
				
		}
		p = tmp + 1;
	}

	LPSTR szFunction = NULL,
		szP = NULL,
		szBody = NULL;

	if(szName==NULL || szContent == NULL){ 
		goto err;
	}

	//给函数一个动态的函数名。（避免函数名重复）
	szFunction = (LPSTR)malloc(20);
	memset(szFunction, 0, 20);
	sprintf_s(szFunction, 20, "LdStockTrad%d", m_TradScripts.GetCount()); 

	szP = NULL;
	int nlen = 0;
	for(int i=0; i<nPs; i++){
		nlen += strlen(szParams[i]) + 1;			
	}
	if(nlen){
		szP = (LPSTR)malloc(nlen);
		memset(szP, 0, nlen);
		for(int i=0; i<nPs; i++){
			strcat(szP, szParams[i]);
			if(i!=nPs-1)
				strcat(szP,",");
		}
	}
	nlen = nlen + strlen(szContent) + strlen(szFunction) + 20;

	szBody = (LPSTR)malloc(nlen);
	memset(szBody, 0, nlen);
	if(szP)
		sprintf_s(szBody, nlen, "function %s(%s)\r\n%s\r\nend", szFunction, szP, szContent);
	else
		sprintf_s(szBody, nlen, "function %s()\r\n%s\r\nend", szFunction, szContent);

	PTRAD_STRCPIT pScript = NULL;
	if(LoadScriptLib(szBody)){
		pScript = new TRAD_STRCPIT;
		pScript->szName = szName;
		pScript->nParamCount = nPs;
		pScript->szComment = szComment;
		pScript->szParams = szParams;
		pScript->szFunction = szFunction;
		m_TradScripts.Add(pScript);
	}

	if(szFunction){
		free(szFunction);
	}
	if(szP){
		free(szP);
	}
	if(szBody){
		free(szBody);
	}

	return pScript;

err:
	if(szName)
		free(szName);
	if(szComment)
		free(szComment);
	for(int i=0; i<nPs; i++){
		free(szParams[i]);
		free(szParamComment[i]);
	}
	if(szParams){
		free(szParams);
	}
	if(szParamComment){
		free(szParamComment);
	}
	return NULL;
}
