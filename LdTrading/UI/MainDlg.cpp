#include "..\StdAfx.h"
#include "MainDlg.h"
#include "IconListener.h"
#include "..\..\StockDataAPI\web\HttpStockData.h"


CMainDlg::CMainDlg(void)
{
	m_Notifyicon = NULL;
}


CMainDlg::~CMainDlg(void)
{
	if(m_Notifyicon!=NULL)
		delete m_Notifyicon;
}

LRESULT CMainDlg::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg==WM_DESTROY)
		PostQuitMessage(0);
	switch(uMsg){
	case WM_SETCURSOR:
		if(wParam==(WPARAM)hInstance, IDC_WAIT)
			break;
		break;
	}
	return CLdDialog::WindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL CMainDlg::OnInitDialog()
{
	m_Notifyicon = new CNotifyIcon(new CIconListener());
	m_Notifyicon->ShowIcon();

	
	return CLdDialog::OnInitDialog();
}

INT_PTR CMainDlg::OnCommand(WORD ctrlid, HWND hwnd)
{
	
	LPCSTR szRet = NULL;
	char ss[1024] = {0};
	
	switch(ctrlid){
	case IDC_BUTTON_TEST:
		WideCharToMultiByte(CP_ACP, 0, m_ed1.GetText(), -1, ss, 1024, NULL, NULL);
		if(ScriptEng->RunScript(ss, &szRet))
			m_ed2.SetText(szRet);
		break;
	}

	//TradClient->StockSell(MARK_SZ, "000858", 14, 100);
	//TradClient->GetZjgf();
	//SetCursor(LoadCursor(NULL, IDC_HAND));
	return CLdDialog::OnCommand(ctrlid, hwnd);
}

void CMainDlg::InitDlgMembers(PDLGITEM_MAP maps)
{
	DLGITEM_MAP a[] = {
		{IDOK, &m_btnOk},
		{IDC_BUTTON_TEST, &m_btn},
		{IDC_EDIT1, &m_ed1},
		{IDC_EDIT2, &m_ed2},
		{-1}
	};

	CLdDialog::InitDlgMembers(a);

	m_ed1.SetText("return LdStockTrad0(\"000858\", 5, 30, 100)");
}
