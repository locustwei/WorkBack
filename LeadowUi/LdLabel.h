#pragma once
#include "LdWnd.h"


// CLdLabel

class CLdLabel : public CLdWnd
{

public:
	CLdLabel();
	virtual ~CLdLabel();


protected:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


