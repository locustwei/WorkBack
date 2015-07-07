#pragma once
#include "LdImage.h"
#include "LdWnd.h"
#include "LdListItem.h"
#include "..\PublicLib\comps\LdList.h"

class CLdListBoxItem: public CLdListItem
{
};
// CLdListBox

class CLdListBox : public CLdWnd
{
public:
	CLdListBox();
	virtual ~CLdListBox();

	CLdListBoxItem* AddString(LPCTSTR lpszItem);                       //替换CListBox函数
	CLdListBoxItem* InsertString(int nIndex, LPCTSTR lpszItem);         
	CLdListBoxItem* InsertItem(int nIndex=-1);                                         //添加项目(nIndex=-1为append）
	int InsertItem(CLdListBoxItem* pItem, int nIndex);                       
	CLdListBoxItem* GetItem(int nIdx);                               
	int DeleteString(UINT nIndex);                               
	void CheckItem(UINT nIndex, BOOL bCheck);                    //勾选Item
	void CheckItem(CLdListBoxItem* item, BOOL bCheck);
	int GetCheckedItems(CLdList<CLdListBoxItem*>& items);                         //获取已勾选的项目数组。返回个数，Item Append到items中
	int GetCount();
	int GetCurSel();
	int GetItemHeight( int nidx );

	BOOL m_ShowIcon;                  //画图标
	BOOL m_ShowCheckBox;              //画选择框，当多选时为checkbox， 单选时为radiobox
	BOOL m_MuiltSelect;               //是否多选(默认true)
	int m_nFixLeftPix;                 //左边空几个像素开始画（默认2像素）；
	BOOL m_DrawItemLine;               //是否画Item间的分隔写
	BOOL m_IsComboBoxList;             //是否控件是作为ComboBox的下拉ListBox。（如果是出Mouse事件是有些不同）

	IDrawEvent* OnBeforeItemDraw;          //发布ItemDraw事件
	IDrawEvent* OnAfterItemDraw;           //	
	IItemMouseEvent* OnWndMouseMove;       //发布鼠标移动事件（item可能为空）
	INotifyEvent* OnEnter;          //发布鼠标进入事件
	INotifyEvent* OnLeave;          //发布鼠标离开事件
	IItemMouseEvent* OnClick;          //鼠标点击（item可能为空）
	IItemMouseEvent* OnItemChecked;    //Item的CheckBox点击事件

	UINT ItemFromPoint(CPoint pt);
	void SetItemHeight( int nidx, int nheight );
protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void DrawItemCaption(HDC pDc, CLdListBoxItem* item, UINT itemAction, UINT itemState);

	BOOL OnEraseBkgnd(HDC pDC);
	void OnSize(UINT nType, int cx, int cy);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnDestroy();
private:
	CLdListBoxItem* m_MouseMoveItem;
	CLdListBoxItem* m_MouseDownItem;

	void InitMember();
	void DoDrawItem( HDC pDc, CLdListBoxItem* item, UINT itemAction, UINT itemState );
	CLdImage* GetItemImage( CLdListBoxItem* item );
public:
	CLdImage* ImgBackgrdNoneItem;
	CLdImage* ImgBackgrd;
	CLdImage* ImgItemNormal;
	CLdImage* ImgItemMouseDown;
	CLdImage* ImgItemMouseOver;

};


