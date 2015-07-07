// LdGroupBox.cpp : 实现文件
//

#include "stdafx.h"
#include "LdGroupBox.h"


// CLdGroupBox


CLdGroupBox::CLdGroupBox():CLdWnd()
{
	m_ClassName = L"STATIC";
	m_Style = WS_VISIBLE|BS_GROUPBOX|SS_LEFT|SS_ETCHEDFRAME;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdGroupBox::~CLdGroupBox()
{
}

LRESULT CLdGroupBox::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}

// CLdGroupBox 消息处理程序


