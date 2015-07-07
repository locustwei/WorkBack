#pragma once
#include "LdWnd.h"


// CldCheckBox

class CLdCheckBox : public CLdWnd
{

public:
	CLdCheckBox();
	virtual ~CLdCheckBox();


protected:
	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};


