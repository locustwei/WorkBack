/************************************************************************
股份查询窗口
************************************************************************/

#include "TDXGfcx.h"
#include <string.h>
#include <stdlib.h>
#include <commctrl.h>

#pragma warning( disable : 4244)

CTDXZjgf::CTDXZjgf(HWND hWnd):CWndHook(hWnd)
{
	m_l628   = GetDlgItem(hWnd, 0x0628);
	m_lst61F  = GetDlgItem(hWnd, 0x061F);
}


CTDXZjgf::~CTDXZjgf(void)
{
}

BOOL CTDXZjgf::GetStatisticsValue(float& ye, float& ky, float& sz, float& yk)
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

int CTDXZjgf::GetGf(PTDX_STOCK_GF pGf)
{
	if(!m_lst61F)
		return FALSE;
	int nCount = ListView_GetItemCount(m_lst61F);
	
	for(int i=0; i<nCount; i++){
		ListView_GetItemText(m_lst61F, i, 0, pGf[i].code, 6);  //
		ListView_GetItemText(m_lst61F, i, 1, pGf[i].name, 4);

		TCHAR szText[20] = {0};
		ListView_GetItemText(m_lst61F, i, 2, szText, 20);
		pGf[i].sl = _wtoi(szText);

		ListView_GetItemText(m_lst61F, i, 3, szText, 20);
		pGf[i].kmsl = _wtoi(szText);

		ListView_GetItemText(m_lst61F, i, 4, szText, 20);
		pGf[i].jmsl = _wtoi(szText);

		ListView_GetItemText(m_lst61F, i, 5, szText, 20);
		pGf[i].ckcbj = _wtof(szText);

		ListView_GetItemText(m_lst61F, i, 6, szText, 20);
		pGf[i].mrjj = _wtof(szText);

		ListView_GetItemText(m_lst61F, i, 7, szText, 20);
		pGf[i].dqj = _wtof(szText);

		ListView_GetItemText(m_lst61F, i, 8, szText, 20);
		pGf[i].zxsz = _wtof(szText);

		ListView_GetItemText(m_lst61F, i, 9, szText, 20);
		pGf[i].ccyk = _wtof(szText);

		ListView_GetItemText(m_lst61F, i, 10, szText, 20);
		pGf[i].ykbl = _wtoi(szText);

		ListView_GetItemText(m_lst61F, i, 11, szText, 20);
		pGf[i].djsl = _wtoi(szText);
	}

	return nCount;
}

int CTDXZjgf::GetZjgf(_Out_ PTDX_STOCK_ZJGF* result)
{
	int nSize = 0;
	PTDX_STOCK_ZJGF tmp = NULL;

	int nCount = ListView_GetItemCount(m_lst61F);
	nSize = sizeof(TDX_STOCK_ZJGF) + nCount*sizeof(TDX_STOCK_GF);
	tmp = (PTDX_STOCK_ZJGF)malloc(nSize);
	ZeroMemory(tmp, nSize);

	if(!GetStatisticsValue(tmp->ye, tmp->ky, tmp->sz, tmp->yk)){
		free(tmp);
		return 0;
	}
	tmp->count = GetGf(tmp->gf);
	*result = tmp;

	return nSize;
}
