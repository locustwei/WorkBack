#include "stdafx.h"
#include "LdDialog.h"

CLdDialog::CLdDialog()   
{   
	m_hWnd = NULL;
	m_IsModal = FALSE;

	//m_Caption=" ";
	//m_FontName="MS Sans Serif";
}   

INT_PTR CALLBACK LdDialogProc(
	__in  HWND hwndDlg,
	__in  UINT uMsg,
	__in  WPARAM wParam,
	__in  LPARAM lParam
	)
{
	CLdDialog* dlg;
	switch(uMsg){
	case WM_INITDIALOG:
		dlg = (CLdDialog*)lParam;
		if(dlg){
			dlg->m_hWnd = hwndDlg;
			SetWindowLong(hwndDlg, DWL_USER, lParam);
			return dlg->OnInitDialog();
		}
		break;
	default:
		dlg = (CLdDialog*)GetWindowLong(hwndDlg, DWL_USER);
		if(dlg)
			return dlg->WindowProc(hwndDlg, uMsg, wParam, lParam);
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL CLdDialog::OnInitDialog()    
{   
	CenterWindow();   

	InitDlgMembers(NULL);
	
	return TRUE; 
}   

int CLdDialog::ShowDialog(LPTSTR lpTemplate)   
{   
	m_hWnd = CreateDialogParam(CLdUiObject::hInstance, lpTemplate, NULL, &LdDialogProc, (LPARAM)this);
	::ShowWindow(m_hWnd, SW_SHOW);
	m_IsModal = FALSE;
	return 0;
} 

void CLdDialog::CenterWindow()
{
}

int CLdDialog::DoModal(LPTSTR lpTemplate)
{
	m_IsModal = TRUE;
	return 0;
}

LRESULT CLdDialog::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg){
	case WM_COMMAND:
		DoCommand(wParam, lParam);
		break;
	case WM_NOTIFY:
		SendMessage(GetDlgItem(hwnd, (((LPNMHDR)lParam)->idFrom)), WM_NOTIFY, wParam, lParam);
		break;
	case WM_DRAWITEM:
		DoCommand(wParam, lParam);
		break;
	}
	return (INT_PTR)CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}

CLdDialog::~CLdDialog()
{

}

INT_PTR CLdDialog::OnMenu( WORD mid )
{
	return FALSE;
}

INT_PTR CLdDialog::OnAccelerator( WORD aid )
{
	return FALSE;
}

INT_PTR CLdDialog::OnCommand( WORD ctrlid, HWND hwnd )
{
	return FALSE;
}

void CLdDialog::InitDlgMembers( PDLGITEM_MAP maps )
{
	if(maps){
		PDLGITEM_MAP itMap = maps;
		while(itMap->ItemId!=-1){
			itMap->wnd->AttchWindow(GetDlgItem(m_hWnd, itMap->ItemId));
			itMap++;
		}
	}
}

void CLdDialog::DoCommand( WPARAM wParam, LPARAM lParam )
{
	UINT nID = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int nCode = HIWORD(wParam);
	if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL){
		if(m_IsModal)
			EndDialog(m_hWnd, LOWORD(wParam));
		else
			DestroyWindow(m_hWnd);
	}else if(hWndCtrl==NULL || nCode==0){
		OnCommand(nID, NULL);
	}else{
		SendMessage(hWndCtrl, MM_NOTIFY, nCode, 0);
	}


}
