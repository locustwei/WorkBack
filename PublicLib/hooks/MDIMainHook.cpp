#include "..\StdAfx.h"
#include "MDIMainHook.h"


CMDIMainHook::CMDIMainHook(HWND hWnd):CWndHook(hWnd)
{
	HWND hClient = FindWindowEx(hWnd, NULL, L"MDIClient", NULL);
	if(hClient!=NULL){
		m_ClientHook = new CMdiClientHook(hClient);
		m_ClientHook->m_hMainWnd = hWnd;
	}

}


CMDIMainHook::~CMDIMainHook(void)
{
}

LRESULT CMDIMainHook::WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam)
{
	return CWndHook::WndPROC(hwnd, nCode, wparam, lparam);
}
