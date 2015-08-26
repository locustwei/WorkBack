/**
窗口消息钩子
**/
#pragma once

#include "..\stdafx.h"

//window class name
#define CN_Dialog            L"#32770"
#define CN_AfxWnd42          L"AfxWnd42"
#define CN_Afx               L"Afx"
#define CN_AfxFrameOrView42  L"AfxFrameOrView42"
#define CN_AfxMDIFrame42     L"AfxMDIFrame42"
#define CN_SysTreeView32     L"SysTreeView32"

#define HOOKPOINT L"LD_HOOK_PROC"

class CWndHook{
	friend LRESULT WINAPI WndHookPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
public:
	CWndHook(HWND hWnd);
	~CWndHook();

	BOOL StartHook();
	BOOL StopHook();

	HWND GetWindowHandle(){ return m_hWnd;};
	BOOL IsHook(){return m_nextHook!=NULL;};

	TCHAR m_ClassName[MAX_PATH];
	TCHAR m_Text[MAX_PATH];
	HWND m_hWnd;

protected:
	WNDPROC m_nextHook;

	virtual LRESULT WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
	
};
