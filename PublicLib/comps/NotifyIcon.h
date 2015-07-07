/**
系统托盘上方一个ICON
**/ 
#pragma once

#include "..\stdafx.h"
#include <ShellAPI.h>

#define NOTIFYICON_MESSAGE WM_USER+0x894A

class CNotifyIcon;

struct INotifyIconListener{
	CNotifyIcon* m_Icon;
	virtual void OnRightClick(POINT pos) = 0;
	virtual void OnLeftClick(POINT pos) = 0;
	virtual void OnDbClick() = 0;
	virtual HICON GetIcon() = 0;
	virtual LPCTSTR GetTipText() = 0;
	virtual void OnMenuClick(int menuid) = 0;
};

///////////////////////////////////////////////////////////////////////////// 
// CNotifyIcon window 
class CNotifyIcon {
// Construction/destruction 
public: 
	CNotifyIcon(INotifyIconListener* listener); 
	virtual ~CNotifyIcon(); 
// Operations 
public: 
	BOOL IsVisible() { return !m_bShowing; } 

	BOOL ShowIcon(); 
	void HideIcon(); 
	void PopupMenu(HMENU hmenu, POINT pos);
protected: 
	//创建系统图标 

	BOOL m_bShowing; //是否隐藏图标 
	HWND m_hWnd;  //接受消息窗口
	NOTIFYICONDATA m_tnd; //数据结构，请参考在线帮助 
	INotifyIconListener* m_Listener;

	LRESULT OnIconNotification(HWND hwnd, UINT wParam, LONG lParam);
	void NotifyMenuItemClicked(int menuId);

private:
	HWND CreateNotifyWindow();
	static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam);
};
