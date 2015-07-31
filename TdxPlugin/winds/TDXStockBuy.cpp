/************************************************************************/
/* 股票买入窗口
*/
/************************************************************************/
#include "TDXStockBuy.h"
#include <stdio.h>
#include "..\..\PublicLib\Utils_Wnd.h"
#include "..\stdafx.h"

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
	case MM_STOCK_BY_END:
		TradSocket->SendStockByResult(m_HtId);
		break;
	}
	return result;
}

BOOL CTDXStockBuy::DoBy( STOCK_MARK mark, LPCSTR szSymbol, float fPrice, DWORD dwVolume )
{
	BOOL result = FALSE;
	m_HtId = 0;
	m_Mark = mark;
	strcpy_s(m_Code, szSymbol);
	m_Price = fPrice;
	m_Volume = dwVolume;
	
	RunOnViceThread(&ByContolInViceThread, (LPARAM)this);//在辅助线程中等待交易确认对话框

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

//在辅助线程中运行，控制下单过程
#define COMFIRM_DLGTITLE L"买入交易确认"
#define PROMPT_DLGTITLE L"提示"
void ByContolInViceThread(LPARAM lparam)
{
	CTDXStockBuy* dlgBy = (CTDXStockBuy*)lparam;
	
	//输入框中赋值------------------------------------------------------------------------------------------------------
	int i = 0;
	do 
	{//
		dlgBy->SetControlsText();
		Sleep(300);
		if(GetWindowTextLength(dlgBy->m_edCode)==6 && GetWindowTextLength(dlgBy->m_edValue)>0 && GetWindowTextLength(dlgBy->m_edCount)>0)
			break;
	} while (i++<3);

	if(GetWindowTextLength(dlgBy->m_edCode)!=6 || GetWindowTextLength(dlgBy->m_edValue)==0 || GetWindowTextLength(dlgBy->m_edCount)==0) //没有成功
		goto exit;
	
	//点击买入按钮------------------------------------------------------------------------------------------------------
	dlgBy->m_btnClicked = FALSE;
	i = 0;
	do 
	{
		Sleep(300);
		SendClickMessage(dlgBy->m_btnOk, TRUE);
	} while (i++<3);
	
	//关闭交易确认对话框--------------------------------------------------------------------------------------------
	i = 0;
	HWND hwnd = NULL; 
	do{
		Sleep(300);
		hwnd = FindWindowEx(NULL, NULL, CN_Dialog, COMFIRM_DLGTITLE);//交易确认窗口
		if(hwnd){
			break;
		}
	}while(i++<3);

	if(hwnd==NULL)
		goto exit;

	HWND hBtn = GetDlgItem(hwnd, 0x1B67);
	i = 0;  //有可能一次点击不成功点3次
	do 
	{
		SendClickMessage(hBtn, TRUE);
		Sleep(100);
		hwnd = FindWindowEx(NULL, NULL, CN_Dialog, COMFIRM_DLGTITLE);
		if(!hwnd)
			break;
	} while (i++<3);
	
	if(hwnd)  //如果窗口还在说明关闭不成功
		goto exit;

	//关闭合同号提示对话框----------------------------------------------------------------------------------------
	i = 0;
	do{
		Sleep(500);
		hwnd = FindWindowEx(NULL, NULL, CN_Dialog, PROMPT_DLGTITLE); //提交委托后，弹出“提示”窗口
		if(hwnd){
			dlgBy->m_HtId = 1;  //todo 获取合同号
			hBtn = GetDlgItem(hwnd, 0x1B67);
			if(hBtn)
				SendClickMessage(hBtn, TRUE);
			break;
		}
	}while (i++<3);	

exit:
	PostMessage(dlgBy->m_hWnd, MM_STOCK_BY_END, 0, 0);

}
