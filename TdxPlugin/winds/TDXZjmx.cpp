/************************************************************************/
/* 资金明细（交易数据查询）窗口
*/
/************************************************************************/
#include "TDXZjmx.h"
#include "..\..\PublicLib\hooks\WindowHook.h"

CTDXZjmx::CTDXZjmx(HWND hWnd):CWndHook(hWnd)
{
	m_cbBz     = GetDlgItem(hWnd, 0x05D3);    //币种（ComboBox）
	m_dtBegin  = GetDlgItem(hWnd, 0x0468);	  //开始日期（SysDateTimePick）
	m_dtEnd    = GetDlgItem(hWnd, 0x046B);    //结束日期（SysDateTimePick）
	m_btnOk    = GetDlgItem(hWnd, 0x0474);    //查询（Button）
	m_lstGp    = GetDlgItem(hWnd, 0x061F); 	  //结果List（SysListView32）
}


CTDXZjmx::~CTDXZjmx(void)
{
}
