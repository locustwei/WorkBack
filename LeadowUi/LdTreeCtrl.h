#pragma once
#include "LdWnd.h"
#include <CommCtrl.h>
#include "LdListItem.h"

class CLdTreeCtrl;

class CLdTreeItem: public CLdListItem
{
	friend class CLdTreeCtrl;
public:
	HTREEITEM hItem;
};

// CLdTreeCtrl

class CLdTreeCtrl : public CLdWnd
{

public:
	CLdTreeCtrl();
	virtual ~CLdTreeCtrl();
	CLdTreeItem* InsetItem(CLdTreeItem* pParent, CLdTreeItem* pAfter, LPCTSTR szText );
	void InsetItem(CLdTreeItem* pParent, CLdTreeItem* pAfter, CLdTreeItem* pItem );
};


