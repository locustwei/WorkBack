/************************************************************************/
/* 股票买入窗口
*/
/************************************************************************/
#include "TDXStockBuy.h"


CTDXStockBuy::CTDXStockBuy(HWND hWnd):CWndHook(hWnd)
{
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
	return CWndHook::WndPROC(hwnd, nCode, wparam, lparam);
}
