/********************************************************************
	file base:	IconListener
	file ext:	h
	author:		博文
	
	purpose:	任务栏图标事件处理
*********************************************************************/
#pragma once
#include "..\..\PublicLib\comps\NotifyIcon.h"

class CIconListener :public INotifyIconListener
{

public:
	HMENU m_hMenu;
	HICON m_hIcon;

	CIconListener()
	{
		m_hIcon = (HICON)LoadImage(hInstance, (LPCTSTR)IDI_ICON_NOTIFY, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
		m_hMenu = LoadMenu(hInstance, (LPCTSTR)IDR_MENU_MENU);
		m_hMenu = GetSubMenu(m_hMenu, 0);
	}

	~CIconListener(void);

	virtual void OnRightClick( POINT pos )
	{
		m_Icon->PopupMenu(m_hMenu, pos);
	}

	virtual void OnLeftClick( POINT pos )
	{


	}

	virtual void OnDbClick()
	{
	}

	virtual HICON GetIcon()
	{
		return m_hIcon;
	}

	virtual LPCTSTR GetTipText()
	{
		return L"的地方公司的分公司";
	}

	virtual void OnMenuClick( int menuid )
	{

	}
};

