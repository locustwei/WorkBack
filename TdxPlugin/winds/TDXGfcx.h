/************************************************************************
股份查询窗口
************************************************************************/
#pragma once
#include "..\..\PublicLib\hooks\WindowHook.h"

class CTDXGfcx :public CWndHook
{
public:
	CTDXGfcx(HWND hWnd);
	~CTDXGfcx(void);
private:
	HWND m_l628;         // 资产统计信息：余额:*  可用:*  参考市值:*  资产:*  盈亏:* 
	HWND m_lst61F;       // 持有股票列表

	BOOL GetStatisticsValue(float& ye, float& ky, float& sz, float& yk);

};

