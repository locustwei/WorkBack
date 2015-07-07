// LdEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "LdEdit.h"


// CLdEdit

CLdEdit::CLdEdit()
{
	m_ClassName = L"EDIT";
	m_Style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER |ES_AUTOHSCROLL;
	m_ExStyle = WS_EX_TRANSPARENT;

	m_HintStr = NULL;
	m_Text = NULL;
	m_TextLen = 0;
}

CLdEdit::~CLdEdit()
{
	if(m_Text)
		FREEMEM_NIL(&m_Text);
}

LRESULT CLdEdit::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
	return result;
}

void CLdEdit::SetReadOnly( BOOL value )
{
	Edit_SetReadOnly(m_hWnd, value);
}

void CLdEdit::setHintStr( LPCTSTR szHint )
{
	m_HintStr = szHint;
}

void CLdEdit::OnPaint()
{
	if(!m_HintStr)
		return;
	RECT r;
	GetClientRect(m_hWnd, &r);
	PAINTSTRUCT ps = {0};
	HDC dc = BeginPaint(m_hWnd, &ps);
	INT color = SetTextColor(dc, 0x121212);
	
	DrawText(dc, m_HintStr, -1, &r, DT_LEFT);
	SetTextColor(dc, color);
	EndPaint(m_hWnd, &ps);
}

LPCTSTR CLdEdit::GetText()
{
	int len = GetTextLen() + 1;
	if(len==0)
		return NULL;

	if(!m_Text || (len>m_TextLen && m_Text)){
		FREEMEM_NIL(&m_Text);
		m_Text = (LPTSTR)GETMEM((len)*sizeof(TCHAR));
		m_TextLen = len;
	}
	ZeroMemory(m_Text, (m_TextLen)*sizeof(TCHAR));
	GetWindowText(m_hWnd, m_Text, len);
	return (LPCTSTR)m_Text;
}

int CLdEdit::GetTextLen()
{
	return GetWindowTextLength(m_hWnd);
}

void CLdEdit::SetText(LPCSTR szRet)
{
	SetWindowTextA(m_hWnd, szRet);
}

// CLdEdit 消息处理程序


