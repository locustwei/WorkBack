/********************************************************************
	created:	2013/12/07
	created:	7:12:2013   23:09
	author:		博文
	
	purpose:	类似于Toolbar控件。在Panel上放多个按钮。可以设置背景图，按钮图片
*********************************************************************/
#pragma once
#include "LdPanel.h"
#include "LdImage.h"
#include <Commctrl.h>
#include "..\PublicLib\comps\LdList.h"

typedef struct tagBtnBarItem{ //按钮的数据结构
	UINT nImage;
	CString szCaption;
	CONTROLSTATUS csStatus;
	BOOL bVisiabled;
	CRect crRect;
	CRect crCaption;
	CString szHint;
	LPVOID lpData;
}BTNBARITEM, *LPBTNBARITEM;

// CLdBtnBar

class CLdBtnBar : public CLdPanel
{

public:
	CLdBtnBar();
	virtual ~CLdBtnBar();
public:
	CLdImage* m_ImgBackgrd;                    //背景图
	HIMAGELIST m_ImstItem;                  //Btn图标。
	DWORD m_BtnAlign;                    //Btn排列方式，默认水平靠左，垂直居中
	DWORD m_BtnCaptionAlign;             //按钮Caption放的位置。默认水平居中，垂直靠下。
	BOOL m_ShowBtnCaption;                   //是否显示Btn的Caption。默认True
	IItemChange* OnItemClicked;               //发布Item被点击事件


	LPBTNBARITEM InsertItem(LPCTSTR szCaption, UINT nImage, UINT nAffter=-1);//插入Btn默认放在最后
	UINT GetItemCount();                     //返回Btn个数
	LPBTNBARITEM GetItem(int nIdx);
	void DeleteItem(int nIdx);              //
	void UpdateItem(int nIdx);              //重绘Item
	LPBTNBARITEM GetItemFrom(POINT pt);     //获取坐标点的Item

	CSize GetItemSize();                     //获取Btn的大小
	void SetItemSize(CSize szie);                      //设置Btn的大小
protected:
	//void SelectChange(UINT nNew);            //改变当前选中的Item（）
	void DrawItem(LPBTNBARITEM pItem);       //画Btn
	void DrawItem( HDC pDc, LPBTNBARITEM pItem );
	void OnMouseMove(UINT nFlags, POINT point);
	void OnPaint();
	void OnLButtonDown(UINT nFlags, POINT point);
	void OnLButtonUp(UINT nFlags, POINT point);
	void OnSize(UINT nType, int cx, int cy);
	BOOL OnDrawBackgrd(HDC pDC);
private:
	CSize m_ItemSize;
	CLdList<LPBTNBARITEM> m_Items;
	LPBTNBARITEM m_CaptureItem;
	void InitMember();                   //初始化变量
	UINT GetItemIndex(LPBTNBARITEM lpItem);

	void DrawBackgrd( HDC pDc );
};


