#pragma once

#include "stdafx.h"

#define MM_LDWND_MESSAGE WM_USER + 0x34
#define MM_ANCHORS MM_LDWND_MESSAGE + 1  //窗口sizing重新排列子窗口
#define MM_NOTIFY MM_LDWND_MESSAGE + 2   //WM_NOTIFY发送到子窗口

class CLdUiObject
{
public:
	static HINSTANCE hInstance;
};

/************************************************************************/
/* 一般的事件回调函数定义。（Button中引用）。
Sender：为事件发布者本身*/
/************************************************************************/
struct INotifyEvent
{
	virtual void OnNotify(CLdUiObject* Sender) = 0;
};

struct IDrawEvent
{
	virtual BOOL OnDraw(CLdUiObject* Sender, HDC pDc, RECT rect) = 0;
};

struct IItemChange
{
	virtual BOOL OnItemChang(CLdUiObject* Sender, int nNew) = 0;
};

struct IMouseEvent
{
	virtual void OnMouseEvent(CLdUiObject* Sender, POINT pt) = 0;
};

struct IItemMouseEvent
{
	virtual void OnItemMouseEvent(CLdUiObject* Sender, CLdUiObject* item, POINT pt) = 0;
};
/************************************************************************/
/* 控件状态枚举。
CS_NORMAL：正常、CS_MOUSEMOVE：鼠标经过、CS_MOUSEDOWN：鼠标按下、CS_DISABLEED：不可用、CS_SELECED：选中（获取焦点）。*/
/************************************************************************/
typedef enum CONTROLSTATUS{CS_NORMAL, CS_MOUSEMOVE, CS_MOUSEDOWN, CS_DISABLEED, CS_SELECTED};

/************************************************************************/
/* 窗口标题栏可选的项目                                                 */
/************************************************************************/
typedef enum LDFORMTITLEBOX{
	LFT_NONE = 0,                                          //啥也不放
	LFT_CPTION = 1,                                        //标头字符串
	LFT_MENUBOX =  2,                                      //窗口菜单
	LFT_MINBOX = 4,                                        //最小化按钮
	LFT_MAXBOX = 8,                                        //最大化按钮
	LFT_CLOSEBOX = 16,                                     //关闭按钮
	LFT_SIZEBOX =32,                                       //改变大小边框
	LFT_ICON = 64,                                         //窗口图标
	LFT_DEFAULT = LFT_ICON|LFT_CPTION|LFT_CLOSEBOX         //窗口标题默认样式
};

/************************************************************************/
/* 控件相对父Wnd的贴近方式
当父窗口大小发生改变时，控件贴近那一边发生改变。可组合*/
/************************************************************************/
typedef enum ANCHORS{
	akLeft = 1,
	akTop = 2,
	akRight = 4,
	akBottom = 8
};

/************************************************************************/
/* 字符串或控件排放方式， 分水平和垂直两个方向，水平和垂直方向可以组合使用*/
/************************************************************************/
typedef enum ALIGNMENT{
	taLeft=1,
	taRight=2,
	taHCenter=4,

	taTop=16,
	taBottom=32,
	taVCenter=64
};

class CLdWnd: public CLdUiObject
{
	friend LRESULT CALLBACK LdWindowProc(
		_In_ HWND   hwnd,
		_In_ UINT   uMsg,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam
		);
public:
	CLdWnd(void);
	virtual ~CLdWnd(void);
	virtual void Create( HWND pParent);
	virtual void AttchWindow(HWND hwnd);
	void Invalidate();
	void ShowWindow(int nCmdShow);
	void MoveWindow(RECT rc, BOOL update = FALSE);

	HWND m_hWnd;
	PVOID m_Data;
	int m_Tag;
	BYTE m_Anchors;
protected:
	WNDPROC m_WndProc;
	LPCTSTR m_ClassName;
	DWORD m_ExStyle;
	DWORD m_Style;
	virtual LRESULT WindowProc(HWND   hwnd, UINT   uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitWnd();
private:
	void OnParentSizing(UINT nSide, int nx, int ny);
	void OnSizing( UINT nSide, LPRECT lpRect );
	void ResizeChilds(UINT nSide, int nx, int ny);
};

