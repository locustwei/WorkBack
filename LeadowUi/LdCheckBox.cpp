// CldCheckBox.cpp : 实现文件
//

#include "stdafx.h"
#include "LdCheckBox.h"


// CldCheckBox

CLdCheckBox::CLdCheckBox()
{
	m_ClassName = L"BUTTON";
	m_Style = WS_VISIBLE|WS_CHILD|BS_AUTOCHECKBOX;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdCheckBox::~CLdCheckBox()
{
}

LRESULT CLdCheckBox::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}



// CldCheckBox 消息处理程序


