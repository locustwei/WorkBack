#include "..\StdAfx.h"
#include "DlgStockSell.h"


CDlgStockSell::CDlgStockSell(void)
{
	
}


CDlgStockSell::~CDlgStockSell(void)
{
}

BOOL CDlgStockSell::OnInitDialog()
{
	CLdDialog::OnInitDialog();

	return TRUE;
}

LRESULT CDlgStockSell::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CLdDialog::WindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CDlgStockSell::OnCommand( WORD ctrlid, HWND hwnd )
{
	return FALSE;
}

void CDlgStockSell::InitDlgMembers( PDLGITEM_MAP maps )
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
