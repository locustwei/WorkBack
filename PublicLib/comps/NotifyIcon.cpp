/**
系统托盘按放图标
右键弹出菜单，并处理弹出菜单事件
**/
#include "..\stdafx.h" 
#include "NotifyIcon.h" 
#include "stdio.h"
///////////////////////////////////////////////////////////////////////////// 

CNotifyIcon::CNotifyIcon(INotifyIconListener* listener){
	m_Listener = listener;
	listener->m_Icon = this;

	memset(&m_tnd, 0, sizeof(m_tnd));
	m_tnd.cbSize = sizeof(NOTIFYICONDATA); 

	m_bShowing = FALSE; 
	m_hWnd=CreateNotifyWindow();
}

CNotifyIcon::~CNotifyIcon() {
	
	DestroyWindow(m_hWnd); 
}

LRESULT CNotifyIcon::OnIconNotification(HWND hwnd, UINT wParam, LONG lParam) {

	POINT pos; 
	//单击右键弹出菜单 
	switch(LOWORD(lParam)){
	case WM_RBUTTONUP:
		GetCursorPos(&pos); 
		if(m_Listener)
			m_Listener->OnRightClick(pos);
		break;
	case WM_LBUTTONDBLCLK:
		if(m_Listener)
			m_Listener->OnDbClick();
		break;
	case WM_LBUTTONUP:
		GetCursorPos(&pos); 
		if(m_Listener)
			m_Listener->OnLeftClick(pos);
		break;
	}
	return 1; 
}


LRESULT CALLBACK CNotifyIcon::WndProc(HWND hwnd,UINT uMsg,WPARAM wparam,LPARAM lparam)
{
	CNotifyIcon* icon = (CNotifyIcon*)GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg){
	case WM_COMMAND:
		icon->NotifyMenuItemClicked(LOWORD(wparam));
		break;
	case NOTIFYICON_MESSAGE:
		icon->OnIconNotification(hwnd, wparam, lparam);
		break;
	case WM_DESTROY:
		icon->HideIcon();
	}
	return DefWindowProc(hwnd,uMsg,wparam,lparam);
}

HWND CNotifyIcon::CreateNotifyWindow()
{
    //MSG msg;
	HINSTANCE  hInstance = GetModuleHandle(0);//获取当前程序实例句柄
    //创建窗口
    WNDCLASS wndclass;
	memset(&wndclass, 0, sizeof(WNDCLASS));
	
    wndclass.hInstance =hInstance;
	
    wndclass.lpfnWndProc = WndProc;
    wndclass.lpszClassName = L"PlugNotifyMessageWindow";
    wndclass.style = CS_HREDRAW;
	HWND hwnd=NULL;
    ATOM atom=RegisterClass(&wndclass);
	if(atom){
		hwnd = CreateWindow(wndclass.lpszClassName, NULL, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
		if(!hwnd)
			DWORD e=GetLastError();
		//隐藏窗口
		ShowWindow(hwnd,SW_HIDE);

		SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
	}
    return hwnd;
}

BOOL CNotifyIcon::ShowIcon() {
	//确认通知窗口有效 
	if (!IsWindow(m_hWnd) || m_Listener==NULL) 
		return FALSE; 
	if(m_bShowing)
		return TRUE;

	m_tnd.hWnd = m_hWnd; 
	m_tnd.uID = 0; 
	m_tnd.hIcon = m_Listener->GetIcon(); 
	m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
	m_tnd.uCallbackMessage = NOTIFYICON_MESSAGE; 
	wcscpy_s(m_tnd.szTip, m_Listener->GetTipText()); 
	m_bShowing = Shell_NotifyIcon(NIM_ADD, &m_tnd); 
	return m_bShowing; 
}
///////////////////////////////////////////////////////////////////////////// 
void CNotifyIcon::HideIcon() {
	if (!m_bShowing) 
		return; 
	m_tnd.uFlags = 0; 
	Shell_NotifyIcon(NIM_DELETE, &m_tnd); 
	m_bShowing = FALSE; 
}

void CNotifyIcon::NotifyMenuItemClicked(int menuId)
{
	if(m_Listener)
		m_Listener->OnMenuClick(menuId);
}

void CNotifyIcon::PopupMenu( HMENU hmenu, POINT pos )
{
	TrackPopupMenu(hmenu, 0, pos.x, pos.y, 0, m_hWnd, NULL); 
}
