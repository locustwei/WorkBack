/************************************************************************/
/* 股票买入窗口
*/
/************************************************************************/
#include "TDXStockBuy.h"
#include <stdio.h>
#include "..\..\PublicLib\Utils_Wnd.h"
#include "..\stdafx.h"

#define STOCKBY_TIMERID 0xEA6

CTDXStockBuy::CTDXStockBuy(HWND hWnd):CWndHook(hWnd)
{
	m_Inited = FALSE;

	m_edCode   = GetDlgItem(hWnd, 0x2EE5);     //股票代码（Edit）
	m_edValue  = GetDlgItem(hWnd, 0x2EE6);	   //买入价格（Edit）
	m_edCount  = GetDlgItem(hWnd, 0x2EE7);     //买入数量（Edit）
	m_btnOk    = GetDlgItem(hWnd, 0x07DA);     //买入下单（Button）
	m_lbJe     = GetDlgItem(hWnd, 0x2EFC);     //可用金额（Static）
	m_lbZdsl   = GetDlgItem(hWnd, 0x07E6); 	   //最大可买数量（Static）
	m_lstGp    = GetDlgItem(hWnd, 0x0810); 	   //持有股票List（SysListView32）
	m_cbBjfs   = GetDlgItem(hWnd, 0x046D);     //报价方式（ComboBox）
	m_btnAll   = GetDlgItem(hWnd, 0x05D7); 	   //全部（Button）
}


CTDXStockBuy::~CTDXStockBuy(void)
{
}

LRESULT CTDXStockBuy::WndPROC( HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam )
{
	LRESULT result = CWndHook::WndPROC(hwnd, nCode, wparam, lparam);
	switch(nCode){
	case WM_SHOWWINDOW:

		m_Inited = TRUE;
		break;
	case WM_TIMER:
		if(wparam==STOCKBY_TIMERID){
			if(GetWindowTextLength(m_edCode)!=6 || GetWindowTextLength(m_edValue)==0 || GetWindowTextLength(m_edCount)==0)
				SetControlsText();
			else{
				KillTimer(m_hWnd, m_timerId);
				ClickBy();
			}
		}
		break;
	}
	return result;
}

BOOL CTDXStockBuy::DoBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume )
{
	BOOL result = FALSE;

	m_Mark = mark;
	strcpy_s(m_Code, szCode);
	m_Price = fPrice;
	m_Volume = dwVolume;
	
	SetControlsText();

	m_timerId = SetTimer(m_hWnd, STOCKBY_TIMERID, 300, NULL);  //由于界面忙碌，一次不成功设置定时器重试。

	return result;
}

void CTDXStockBuy::SetControlsText()
{
	SetWindowTextA(m_edCode, m_Code);
	char szPrice[10] = {0};
	sprintf_s(szPrice, "%.2f", m_Price);
	SetWindowTextA(m_edValue, szPrice);
	sprintf_s(szPrice, "%d", m_Volume);
	SetWindowTextA(m_edCount, szPrice);
}

void FindByComfirmDlg(UINT uMsg, LPARAM lparam)
{
	Sleep(300);
	HWND hwnd = FindWindowEx(NULL, NULL, CN_Dialog, L"买入交易确认");
	if(hwnd){

	}
}
//点击下单按钮
void CTDXStockBuy::ClickBy()
{
	WaitTimeNotBlock(300);

	RECT r = {0};
	GetWindowRect(m_btnOk, &r);
	SetCursorPos(r.left+ 5, r.top + 5);
	PostThreadMessage(dwViceThreadId, 0, (WPARAM)&FindByComfirmDlg, (LPARAM)this);
	SendMessage(m_btnOk, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(r.left+5, r.top+5));
	WaitTimeNotBlock(100); //需要有时间间隔
	SendMessage(m_btnOk, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(r.left+5, r.top+5));
	//正常此处弹出交易确认模态对话框，中断程序执行等待确认

	//买入交易确认
}
