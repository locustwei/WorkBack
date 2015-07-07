#pragma once
#include "windowhook.h"
#include "MDIClientHook.h"

class CMDIMainHook :public CWndHook
{
protected:
	virtual LRESULT WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
public:
	CMDIMainHook(HWND hWnd);
	~CMDIMainHook(void);

	CMdiClientHook* m_ClientHook;
};

