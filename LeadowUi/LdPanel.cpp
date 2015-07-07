// LdPanel.cpp : 实现文件
//

#include "stdafx.h"
#include "LdPanel.h"


// CLdPanel


CLdPanel::CLdPanel()
{
	m_ClassName = L"STATIC";
	m_Style = WS_CHILD|WS_VISIBLE|WS_BORDER;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdPanel::~CLdPanel()
{
}



// CLdPanel 消息处理程序


