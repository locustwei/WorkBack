/************************************************************************
股份查询窗口
************************************************************************/

#include "TDXGfcx.h"
#include <string.h>
#include <stdlib.h>


CTDXGfcx::CTDXGfcx(HWND hWnd):CWndHook(hWnd)
{
	m_l628   = GetDlgItem(hWnd, 0x0628);
	m_lst61F  = GetDlgItem(hWnd, 0x061F);
}


CTDXGfcx::~CTDXGfcx(void)
{
}

BOOL CTDXGfcx::GetStatisticsValue(float& ye, float& ky, float& sz, float& yk)
{
	if(m_l628==NULL)
		return FALSE;

	TCHAR szText[200] = {0};
	GetWindowText(m_l628, szText, 200);

	TCHAR szValue[20] = {0};
	LPCTSTR tmp = wcsstr(szText, L"余额:");
	if(!tmp)
		return FALSE;
	tmp += 3;	
	for (int i=0; tmp[i] != ' ' && tmp[i] != 0; i++){
		szText[i] = tmp[i];
	}
	ye = _wtof(szValue);

	ZeroMemory(szValue, sizeof(szValue));
	tmp = wcsstr(szText, L"可用:");
	if(!tmp)
		return FALSE;
	tmp += 3;	
	for (int i=0; tmp[i] != ' ' && tmp[i] != 0; i++){
		szText[i] = tmp[i];
	}
	ky = _wtof(szValue);

	ZeroMemory(szValue, sizeof(szValue));
	tmp = wcsstr(szText, L"参考市值:");
	if(!tmp)
		return FALSE;
	tmp += 5;	
	for (int i=0; tmp[i] != ' ' && tmp[i] != 0; i++){
		szText[i] = tmp[i];
	}
	sz = _wtof(szValue);

	ZeroMemory(szValue, sizeof(szValue));
	tmp = wcsstr(szText, L"盈亏:");
	if(!tmp)
		return FALSE;
	tmp += 3;	
	for (int i=0; tmp[i] != ' ' && tmp[i] != 0; i++){
		szText[i] = tmp[i];
	}
	yk = _wtof(szValue);

	return TRUE;
}
