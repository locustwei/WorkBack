#pragma once

#include "../../StockDataAPI/IDataInterface.h"
#include "../ITradInterface.h"
#include "../../PublicLib/comps/LdList.h"

extern "C"{
	#include "../../LuaLib/lua.h"
	#include "../../LuaLib/lauxlib.h"
	#include "../../LuaLib/lualib.h"
};

//脚本注释部分关键字（）。
#define S_COMMENT_B "--[["  
#define S_COMMENT_E "--]]"
#define S_TRAD_NAME "[名称]:"
#define	S_TRAD_MEMO "[说明]:"

#define S_TARD_PARAM_ID "This"    //默认给脚本增加的参数。通过此参数脚本可以获得用户的设置信息

typedef struct _TRAD_STRCPIT
{
	LPCSTR szName;         //显示名
	LPSTR szFunction;      //动态分配的函数名
	LPCSTR szComment;      //注释
	int nParamCount;       //参数数目（）
	LPSTR* szParams;       //参数名称列表
}TRAD_STRCPIT, *PTRAD_STRCPIT;

class CScriptEng
{
	friend BOOL CALLBACK EnumResStrategy(__in  HMODULE hModule,__in  LPCTSTR lpszType,__in  LPTSTR lpszName,__in  LONG_PTR lParam);
	friend BOOL CALLBACK EnumResLib(__in  HMODULE hModule,__in  LPCTSTR lpszType,__in  LPTSTR lpszName,__in  LONG_PTR lParam);
public:
	CScriptEng(void);
	~CScriptEng(void);

	BOOL SetDataInterface(IDataInterface* iInt);   //取数接口
	BOOL SetTradInterface(ITradInterface* iInt);


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

	BOOL AddLib(LPCSTR szLib);   //加载函数库
	PTRAD_STRCPIT AddFunction( LPSTR szScript);  //加载策略函数
	BOOL RunScript(LPCSTR szScript);
};

