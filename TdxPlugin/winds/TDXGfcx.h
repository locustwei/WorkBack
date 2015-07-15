/************************************************************************
股份查询窗口
************************************************************************/
#pragma once
#include "..\..\PublicLib\hooks\WindowHook.h"
#include "..\TDXSocketData.h"

class CTDXZjgf :public CWndHook
{
public:
	CTDXZjgf(HWND hWnd);
	~CTDXZjgf(void);
	int GetZjgf(_Out_ PTDX_STOCK_ZJGF* result);
private:
	HWND m_l628;         // 资产统计信息：余额:*  可用:*  参考市值:*  资产:*  盈亏:* 
	HWND m_lst61F;       // 持有股票列表
	int GetGf(PTDX_STOCK_GF pGf);
	BOOL GetStatisticsValue(float& ye, float& ky, float& sz, float& yk);
};

