/********************************************************************
	created:	2013/12/07
	created:	7:12:2013   23:21
	author:		博文
	
	purpose:	支持4态图片按钮。
*********************************************************************/
#pragma once

#include "LdImage.h"
#include "LdWnd.h"

// CLdButton

class CLdButton : public CLdWnd
{

public:
	CLdButton();
	virtual ~CLdButton();
	BOOL PtInThis(POINT pt, BOOL bScreen=FALSE);     //判断pt是否在button的区域内。bScreen指示pt是否屏幕坐标
	BOOL EnableWindow(BOOL bEnable = TRUE);

	INotifyEvent* OnClick;          //发布Click事件
	INotifyEvent* OnEnter;          //发布鼠标进入事件
	INotifyEvent* OnLeave;          //发布鼠标离开事件
	//MOUSEEVENT OnMouseMove;
	IDrawEvent* OnBeforeDraw;          //画按钮之前
	IDrawEvent* OnAfterDraw;           //画按钮完成

	//CLdImage* m_BtnImags[CS_DISABLEED+1];    //按钮4态图片数组（见CONTROLSTATUS）
	BOOL m_Selected;                       //使按钮保持选择状态（即CS_SELECED）

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	void ChangStatus(CONTROLSTATUS newStatus);           //按钮状态发生改变（CONTROLSTATUS变换）触发重绘
	BOOL DoDrawControl( HDC hDc, CRect rect );          //画按钮过程
private:
	CONTROLSTATUS m_CurStatus;                           //当前的按钮状态

	void DefaultDraw(LPDRAWITEMSTRUCT lpDrawItemStruct); //系统默认画Button。没有图片时执行。
	void OnLButtonDown(UINT x, UINT y);   
	void OnLButtonUp(UINT x, UINT y);

	CLdImage* GetCurstatusImage();                       //画之前获取可用的图片。

	void OnMouseMove(UINT x, UINT y);
	
	void DoClick();                           //发布事件
	BOOL DoBeforeDraw(HDC pDc, CRect rect);
	void DoAfterDraw(HDC pDc, CRect rect);
	void DoEnter();
	void DoLeave();

public:
	CLdImage* ImgNormal;
	CLdImage* ImgDown;
	CLdImage* ImgEnter;
	CLdImage* ImgDisable;
	//static void InitStaticImages();

};

