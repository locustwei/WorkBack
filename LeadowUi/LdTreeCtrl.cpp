// LdTreeCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "LdTreeCtrl.h"


// CLdTreeCtrl

CLdTreeCtrl::CLdTreeCtrl()
{
	m_ClassName = L"LISTBOX";
	m_Style = WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_DISABLEDRAGDROP | TVS_NOTOOLTIPS ;
	m_ExStyle = WS_EX_TRANSPARENT;
}

CLdTreeCtrl::~CLdTreeCtrl()
{
}

CLdTreeItem* CLdTreeCtrl::InsetItem(CLdTreeItem* pParent, CLdTreeItem* pAfter, LPCTSTR szText )
{
	CLdTreeItem* result = new CLdTreeItem();
	result->Caption = (LPTSTR)szText;
	InsetItem(pParent, pAfter, result);
	return result;
}

void CLdTreeCtrl::InsetItem( CLdTreeItem* pParent, CLdTreeItem* pAfter, CLdTreeItem* pItem )
{
	TVINSERTSTRUCT tv = {0};
	if(pParent==NULL)
		tv.hParent = NULL;
	else
		tv.hParent = pParent->hItem;
	if(pAfter==NULL)
		tv.hInsertAfter = NULL;
	else
		tv.hInsertAfter = pAfter->hItem;
	tv.item.mask = TVIF_TEXT | TVIF_PARAM;
	tv.item.pszText = pItem->Caption;
	tv.item.cchTextMax = wcslen(pItem->Caption);
	tv.item.lParam = (LPARAM)pItem;
	
	pItem->hItem = TreeView_InsertItem(m_hWnd, &tv);
}



