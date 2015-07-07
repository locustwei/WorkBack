#include "..\stdafx.h"
#include "WindowHook.h"

class CMdiClientHook : public CWndHook{
	friend class CMDIMainHook;

public:
	CMdiClientHook(HWND hwnd);
private:

	virtual LRESULT WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
	HWND m_hMainWnd;
};