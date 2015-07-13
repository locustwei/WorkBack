#include "..\StdAfx.h"
#include "ScriptEng.h"
#include <stdio.h>

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
枚举脚本资源，加载已定义脚本
/************************************************************************/
BOOL CALLBACK EnumResNameProc(
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

CScriptEng::CScriptEng(void)
{
	m_hLua = luaL_newstate();
	luaopen_base(m_hLua);
	luaL_openlibs(m_hLua);

	lua_pushcfunction(m_hLua, CallDataFunc);
	lua_setglobal(m_hLua, "CallDataFunc");	

	lua_pushcfunction(m_hLua, CallTradFunc);
	lua_setglobal(m_hLua, "CallTradFunc");	
	//加载脚本资源
	EnumResourceNames(hInstance, L"SCRIPT", &EnumResNameProc, (LONG_PTR)this);
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
