// LdRadioBox.cpp : 实现文件
//

#include "stdafx.h"
#include "LdRadioBox.h"


// CLdRadioBox

CLdRadioBox::CLdRadioBox()
{
	m_ClassName = L"BUTTON";
	m_Style = WS_VISIBLE|WS_CHILD|BS_AUTORADIOBUTTON;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdRadioBox::~CLdRadioBox()
{
}




