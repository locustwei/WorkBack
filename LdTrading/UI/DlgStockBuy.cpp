#include "..\StdAfx.h"
#include "DlgStockBuy.h"


CDlgStockBuy::CDlgStockBuy(void)
{
	
}


CDlgStockBuy::~CDlgStockBuy(void)
{
}

BOOL CDlgStockBuy::OnInitDialog()
{
	CLdDialog::OnInitDialog();

	return TRUE;
}

LRESULT CDlgStockBuy::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CLdDialog::WindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CDlgStockBuy::OnCommand( WORD ctrlid, HWND hwnd )
{
	return FALSE;
}

void CDlgStockBuy::InitDlgMembers( PDLGITEM_MAP maps )
{
	DLGITEM_MAP a[] = {
		{IDOK, &m_btnOk},
		{IDCANCEL, &m_btnCancel},
		{IDC_EDIT_VALUE_1,&m_edValue1},
		{IDC_EDIT_VALUE_2,&m_edValue2},
		{IDC_COMBO_PRICE_1,&m_cbPrice1},
		{IDC_COMBO_CON_1,&m_cbCon1},
		{IDC_COMBO_P1,&m_cbUnit1},
		{IDC_COMBO_P2,&m_cbUnit2},
		{-1}
	};
	CLdDialog::InitDlgMembers(a);
}
