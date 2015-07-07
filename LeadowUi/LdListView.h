/********************************************************************
	created:	2014/01/08
	author:		博文
	
	purpose:	Virtual ListCtrl。虚数据ListView控件。使用控件不要Insert（or Add）
	            任何数据。只需要SetItemCount（Ex）告诉控件数据有多少行即可。这样
				可显示大量数据而不会带来刷新困难，当然也带来麻烦：在每次Draw都需要
				处理OnBeforeItemDraw填充CListCtrlItem信息。
*********************************************************************/
#pragma once
#include "LdListItem.h"
#include "LdWnd.h"
#include <Commctrl.h>
#include "..\PublicLib\comps\LdList.h"

class CListCtrlItem: public CLdListItem
{
public:
	class CSubItem
	{
	public:
		CString Caption;
		CRect rcItem;
	};
public:
	DWORD dwId;
	CLdList<CSubItem> SubItems;
private:
};

// CLdListView 视图

class CLdListCtrl : public CLdWnd
{

public:
	CLdListCtrl();           // 动态创建所使用的受保护的构造函数
	virtual ~CLdListCtrl();
	virtual void Create(HWND pParent);
public:
	BOOL m_ShowIcon;                   //画图标
	BOOL m_MuiltSelect;                //是否多选(默认true)
	int m_nFixLeftPix;                 //左边空几个像素开始画（默认2像素）；
	BOOL m_DrawItemLine;               //是否画Item间的分隔写
	CSize m_IconSize;                  //图标大小（默认16*16）

	IDrawEvent* OnBeforeItemDraw;          //发布ItemDraw事件
	IDrawEvent* OnAfterItemDraw;           //	
	IItemMouseEvent* OnWndMouseMove;       //发布鼠标移动事件（item可能为空）
	INotifyEvent* OnEnter;          //发布鼠标进入事件
	INotifyEvent* OnLeave;          //发布鼠标离开事件
	IItemMouseEvent* OnClick;          //鼠标点击（item可能为空）
	IItemMouseEvent* OnItemChecked;    //Item的CheckBox点击事件

	BOOL IsCheckBoxesVisible();
	void GetCheckBoxRect( CRect* crBound, CRect& crCheck );
	int GetColumnCount();
	int InsertColumn(int nCol, LPCTSTR szHead, int nFormat, int nWidth=-1, int nSubItem=-1);
protected:
	virtual BOOL OnChildNotify(WPARAM, LPARAM, LRESULT*);
	BOOL CustomDrawItem(LPNMLVCUSTOMDRAW nmcd, LRESULT* result);
	void OnSize(UINT nType, int cx, int cy);
private:
	CListCtrlItem* m_drawItem;                    //存储当前要画Item的信息（OnBeforeItemDraw回调）
	HIMAGELIST m_ImageList;                      //这个List用于改变ItemHeight
	void GetDispInfo( NMLVDISPINFO* plvdi );
	int OdFinditem(LPNMLVFINDITEM pFindInfo);

	void DoDrawItem( DWORD dwItemId, HDC pDc, RECT cr );

	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};


