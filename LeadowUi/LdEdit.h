#pragma once
#include "LdWnd.h"


// CLdEdit

class CLdEdit : public CLdWnd
{
public:
	CLdEdit();
	virtual ~CLdEdit();
	void SetReadOnly( BOOL value );
	void setHintStr( LPCTSTR szHint );

	LPCTSTR GetText();
	int GetTextLen();
	void SetText(LPCSTR szRet);
protected:
	virtual LRESULT WindowProc(HWND   hwnd, UINT   uMsg, WPARAM wParam, LPARAM lParam);
	void OnPaint();
private:
	LPCTSTR m_HintStr;
	LPTSTR m_Text;
	int m_TextLen;
};


