#pragma once
#include "LdWnd.h"


// CLdGroupBox

class CLdGroupBox : public CLdWnd
{

public:
	CLdGroupBox();
	virtual ~CLdGroupBox();


protected:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


