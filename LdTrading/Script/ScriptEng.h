#pragma once

#include "../../StockDataAPI/IDataInterface.h"
#include "../ITradInterface.h"

extern "C"{
	#include "../../LuaLib/lua.h"
	#include "../../LuaLib/lauxlib.h"
	#include "../../LuaLib/lualib.h"
};

class CScriptEng
{
public:
	CScriptEng(void);
	~CScriptEng(void);

	BOOL SetDataInterface(IDataInterface* iInt);
	BOOL SetTradInterface(ITradInterface* iInt);

	BOOL LoadScriptLib(LPCSTR szLib);   //加载函数库

	BOOL RunScript(LPCSTR szScript, float* fResult);      //执行脚本
	BOOL RunScript(LPCSTR szScript, DWORD* dwResult);
	BOOL RunScript(LPCSTR szScript, double* fResult);
	BOOL RunScript(LPCSTR szScript, boolean* bResult);
	BOOL RunScript(LPCSTR szScript, LPCSTR* szResult);
private:
	lua_State* m_hLua;
	IDataInterface* m_IStockData;
	ITradInterface* m_IStockTrad;

	BOOL RunScript(LPCSTR szScript);
};

