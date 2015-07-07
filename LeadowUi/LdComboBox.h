#pragma once
#include "LdEdit.h"
#include "LdListBox.h"
#include "LdWnd.h"


// CLdComboBox

class CLdComboBox : public CLdWnd
{
public:
	CLdComboBox();
	virtual ~CLdComboBox();

	CLdListBox* m_ListWnd;
	void DoDropDown();
	void DoCloseUp();
	void SetText(LPCTSTR szText);
	virtual void Create( HWND pParent );
protected:
	void OnDestroy();
	virtual BOOL OnChildNotify(UINT nCode);

	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	virtual BOOL OnInitWnd();

};


