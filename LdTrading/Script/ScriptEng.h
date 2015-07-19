#pragma once

#include "../../StockDataAPI/IDataInterface.h"
#include "../ITradInterface.h"
#include "../../PublicLib/comps/LdList.h"

extern "C"{
	#include "../../LuaLib/lua.h"
	#include "../../LuaLib/lauxlib.h"
	#include "../../LuaLib/lualib.h"
};

#define S_COMMENT_B "--[["
#define S_COMMENT_E "--]]"

#define S_TRAD_NAME "[名称]:"
#define	S_TRAD_MEMO "[说明]:"
#define	S_TRAD_PARAM "[参数"

typedef struct _TRAD_STRCPIT_PARAM
{

};

typedef struct _TRAD_STRCPIT
{
	LPCSTR szName;
	LPSTR szFunction;
	LPCSTR szComment;
	int nParamCount;
	LPSTR* szParams;
}TRAD_STRCPIT, *PTRAD_STRCPIT;

class CScriptEng
{
public:
	CScriptEng(void);
	~CScriptEng(void);

	BOOL SetDataInterface(IDataInterface* iInt);
	BOOL SetTradInterface(ITradInterface* iInt);

	BOOL LoadScriptLib(LPCSTR szLib);   //加载函数库
	PTRAD_STRCPIT LoadScriptTrad( LPSTR szScript);  //加载交易函数

	BOOL RunScript(LPCSTR szScript, float* fResult);      //执行脚本
	BOOL RunScript(LPCSTR szScript, DWORD* dwResult);
	BOOL RunScript(LPCSTR szScript, double* fResult);
	BOOL RunScript(LPCSTR szScript, boolean* bResult);
	BOOL RunScript(LPCSTR szScript, LPCSTR* szResult);
private:
	lua_State* m_hLua;
	IDataInterface* m_IStockData;
	ITradInterface* m_IStockTrad;
	CLdList<PTRAD_STRCPIT> m_TradScripts;

	BOOL RunScript(LPCSTR szScript);
};

