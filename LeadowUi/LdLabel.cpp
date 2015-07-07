// LdLabel.cpp : 实现文件
//

#include "stdafx.h"
#include "LdLabel.h"


// CLdLabel


CLdLabel::CLdLabel()
{
	m_ClassName = L"STATIC";
	m_Style = WS_CHILD|WS_VISIBLE|WS_GROUP;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdLabel::~CLdLabel()
{
}

LRESULT CLdLabel::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}



// CLdLabel 消息处理程序


