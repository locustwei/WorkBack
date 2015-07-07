#include "..\stdafx.h"
#include "MDIClientHook.h"


HWND CreateMdiWnd11()
{
	HINSTANCE  hInstance = GetModuleHandle(0);//获取当前程序实例句柄
    //创建窗口
    WNDCLASS wndclass;
	memset(&wndclass, 0, sizeof(WNDCLASS));
	
    wndclass.hInstance =hInstance;
	
    //wndclass.lpfnWndProc = DefFrameProc;
    //wndclass.lpszClassName = "MDIWndCopy";
    wndclass.style = CS_OWNDC;
	wndclass.hbrBackground    = (HBRUSH)(COLOR_BACKGROUND+1);
    ATOM atom=RegisterClass(&wndclass);

	HWND hwnd = CreateWindow( (LPTSTR)atom, NULL, WS_OVERLAPPED | WS_CAPTION | WS_BORDER | WS_THICKFRAME | 
      WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_VISIBLE | WS_SYSMENU, 0, 0, 400, 400, NULL, NULL, hInstance, 0);
	if(hwnd){
		ATOM atom2=GlobalFindAtom(L"MDIClient");

   
		//CreateWindow("MDIClient", NULL, WS_CHILDWINDOW|WS_DLGFRAME,0, 0, 200, 200, hwnd, NULL, hInstance, 0);
	}
	return hwnd;
}

CMdiClientHook::CMdiClientHook(HWND hwnd):CWndHook(hwnd){
}

HWND CreateCopyWnd(LPARAM lp ){
	MDICREATESTRUCT mcs  = *(LPMDICREATESTRUCT)lp;
	HWND m_CopyWnd = CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST, 
		mcs.szClass, 
		mcs.szTitle, 
		WS_POPUPWINDOW, 
		mcs.x, 
		mcs.y, 
		mcs.cx,
		mcs.cy,
		NULL,
		NULL,
		NULL,
		0);
	if(m_CopyWnd){
		ShowWindow(m_CopyWnd, SW_SHOW);
	}

	return m_CopyWnd;
}

LRESULT CMdiClientHook::WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam)
{
	switch(nCode){
	case WM_MDICREATE:
		 SendMessage(m_hMainWnd, nCode, wparam, lparam);
		break;
	}
    return CallWindowProc(m_nextHook,hwnd, nCode,wparam,lparam);
}
