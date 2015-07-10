/************************************************************************/
/* 通达信登录窗口
*/
/************************************************************************/

#include "TDXLogin.h"
#include "TDXMain.h"
#include "..\VerifyCode.h"

CTDXLogin* CTDXLogin::WndHooker = NULL;

CTDXLogin::CTDXLogin(HWND hWnd):CWndHook(hWnd)
{
	m_CbAType  = GetDlgItem(hWnd, 0x044B);
	m_CbSever  = GetDlgItem(hWnd, 0x044F);
	m_CbZH     = GetDlgItem(hWnd, 0x044C);  
	m_BtOk     = GetDlgItem(hWnd, 0x0001);   
	m_BtQX     = GetDlgItem(hWnd, 0x0002);   
	m_EdMM     = GetDlgItem(hWnd, 0x0450); 
	m_CbJYFS   = GetDlgItem(hWnd, 0x0457); 
	m_EdYZM    = GetDlgItem(hWnd, 0x0458);  
	if(m_EdYZM!=NULL){
		TCHAR yzm[5] = {0};
		if(getYZM(yzm)){
			SetWindowText(m_EdYZM, yzm);

			//SendMessage(m_BtOk, WM_LBUTTONDOWN, MK_LBUTTON, 0);
			//SendMessage(m_BtOk, WM_LBUTTONUP, MK_LBUTTON, 0);
		}else
			SetWindowText(m_EdYZM, yzm);
	}	
}


CTDXLogin::~CTDXLogin(void)
{
}

LRESULT CTDXLogin::WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam)
{
	/*
	if(nCode==WM_MOVE && hwnd==m_hWnd){
		TCHAR yzm[5] = {0};
		getYZM(yzm);
		SetWindowText(m_EdYZM, yzm);
	}
	*/
	LRESULT ret = CWndHook::WndPROC(hwnd, nCode, wparam, lparam);
	switch(nCode){
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY: //登录完成后通知主窗口刷新
		if(CTDXMain::WndHooker!=NULL) 
			PostMessage(CTDXMain::WndHooker->m_hWnd, MM_LOGINED, 0, 0);
		//delete CTDXLogin::WndHooker;
		//CTDXLogin::WndHooker = NULL;
		break;
	}

	return ret;
}

void CTDXLogin::HookLoginWnd( HWND hwnd )
{
	if(WndHooker==NULL){
		WndHooker = new CTDXLogin(hwnd);
		WndHooker->StartHook();
	}
}

BOOL CTDXLogin::getYZM(LPTSTR szCode)
{
	BOOL result = TRUE;
	RECT r = {0};
	if(GetWindowRect(m_EdYZM, &r)){	
		POINT p = {r.right+7, r.top+1};
		result = getVierifyCode(m_hWnd, p, szCode);		
	}else
		result = FALSE;

	return result;
}

