// LdMemo.cpp : 实现文件
//

#include "stdafx.h"
#include "LdMemo.h"


// CLdMemo


CLdMemo::CLdMemo()
{
	m_ClassName = L"EDIT";
	m_Style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_MULTILINE | ES_WANTRETURN;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdMemo::~CLdMemo()
{
	CLdWnd::~CLdWnd();
}




