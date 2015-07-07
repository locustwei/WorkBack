// LdComboBox.cpp : 实现文件
//

#include "stdafx.h"
#include "LdComboBox.h"


// CLdComboBox

CLdComboBox::CLdComboBox()
{
	m_ClassName = L"COMBOBOX";
	m_Style = WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED;
	m_ExStyle = WS_EX_TRANSPARENT;

	m_ListWnd=new CLdListBox();
}

CLdComboBox::~CLdComboBox()
{
	OnDestroy();
}

void CLdComboBox::Create( HWND pParent )
{
	CLdWnd::Create(pParent);
}

BOOL CLdComboBox::OnChildNotify( UINT nCode)
{
	switch( nCode){
	case CBN_DROPDOWN:          //DROPDOWN
		DoDropDown();
		break;
	case CBN_CLOSEUP:
		DoCloseUp();
		break;
	}
	return TRUE;
}

void CLdComboBox::SetText( LPCTSTR szText )
{
	ComboBox_SetText(m_hWnd, szText);
}

void CLdComboBox::DoCloseUp()
{
	if(m_ListWnd->m_ShowCheckBox){
		CLdList< CLdListBoxItem* > items;
		m_ListWnd->GetCheckedItems(items);
		CString s;
		for(int i=0; i<items.GetCount(); i++)
			s+=items[i]->Caption+';';
		if(!s.IsEmpty())
			s.Delete(s.GetLength()-1, 1);
		SetText(s);
	}
}

void CLdComboBox::DoDropDown()
{
	int nCount=m_ListWnd->GetCount();
	if(nCount<3)
		nCount=3;
	if(nCount>6)
		nCount=6;

	int nHeight=nCount*m_ListWnd->GetItemHeight(0);
	CRect cr;
	GetWindowRect(m_hWnd, &cr);
	if(cr.Height()==nHeight)
		return;

	cr.bottom+=cr.top+nHeight;
	MapWindowPoints(NULL, GetParent(m_hWnd), (LPPOINT)&cr, 2);
	::MoveWindow(m_hWnd, cr.left, cr.top, cr.right-cr.left, cr.bottom-cr.top, 0);
}

void CLdComboBox::OnDestroy()
{
	if(m_ListWnd->m_hWnd!=NULL)
		DestroyWindow(m_ListWnd->m_hWnd);
}

LRESULT CLdComboBox::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg){
	case MM_NOTIFY:
		OnChildNotify(wParam);
		break;
	}
	return CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL CLdComboBox::OnInitWnd()
{
	COMBOBOXINFO cbInfo={0};
	cbInfo.cbSize=sizeof(COMBOBOXINFO);	
	if(GetComboBoxInfo(m_hWnd, &cbInfo)){
		m_ListWnd->AttchWindow(cbInfo.hwndList);
		m_ListWnd->m_IsComboBoxList=TRUE;
		m_ListWnd->SetItemHeight(0, 20);
	};
	return CLdWnd::OnInitWnd();
}

