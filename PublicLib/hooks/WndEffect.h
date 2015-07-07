/*
给窗口添加效果
1、置顶
2、自动隐藏
*/

#pragma once

#include "..\stdAfx.h"
#include "WindowHook.h"

#define WNDEFCT_HM_NONE     0   //不收缩
#define WNDEFCT_HM_TOP      1   //向上收缩
#define WNDEFCT_HM_BOTTOM   2   //向下收缩
#define WNDEFCT_HM_LEFT     3   //向左收缩
#define WNDEFCT_HM_RIGHT    4   //向右收缩

#define WNDEFCT_CM_ELAPSE   200 //检测鼠标是否离开窗口的时间间隔
#define WNDEFCT_HS_ELAPSE   5   //隐藏或显示过程每步的时间间隔
#define WNDEFCT_HS_STEPS    10  //隐藏或显示过程分成多少步

#define WNDEFCT_INTERVAL    20  //触发粘附时鼠标与屏幕边界的最小间隔,单位为象素
#define WNDEFCT_INFALTE     10  //触发收缩时鼠标与窗口边界的最小间隔,单位为象素
#define WNDEFCT_MINCX       200 //窗口最小宽度
#define WNDEFCT_MINCY       400 //窗口最小高度

class CWndEffect : public CWndHook
{
public:
	CWndEffect(HWND hwnd);	
	BOOL StartHook();
	
protected:
	//修正移动时窗口的大小
	void FixMoving(UINT_PTR fwSide, LPRECT pRect);
	//修正改改变窗口大小时窗口的大小
	void FixSizing(UINT_PTR fwSide, LPRECT pRect);
	//从收缩状态显示窗口
	void DoShow();
	void DoHide();
	//重载函数,只是为了方便调用
	BOOL MySetWindowPos(LPCRECT pRect, UINT nFlags= SWP_SHOWWINDOW|SWP_NOSIZE);
	//BOOL OnInitDialog();
	//void OnPaint();
	//HCURSOR OnQueryDragIcon();
	void OnNcHitTest(int x, int y);
	void OnTimer(UINT_PTR nIDEvent);
	void OnSizing(UINT_PTR fwSide, LPRECT pRect);
	//virtual int DoInitWnd();
	void OnMoving(UINT_PTR fwSide, LPRECT pRect);
	virtual LRESULT WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
	
private:
	HICON m_hIcon;
	//size 别变了
	//BOOL m_isSizeChanged;   //窗口大小是否改变了 
	BOOL m_isSetTimer;      //是否设置了检测鼠标的Timer
	
	//INT  m_oldWndHeight;    //旧的窗口宽度
	INT  m_taskBarHeight;   //任务栏高度
	INT  m_edgeHeight;      //边缘高度
	INT  m_edgeWidth;       //边缘宽度

	INT  m_hideMode;        //隐藏模式
	BOOL m_hsFinished;      //隐藏或显示过程是否完成
    BOOL m_hiding;          //该参数只有在!m_hsFinished才有效
	                        //真:正在隐藏,假:正在显示
	BOOL m_topMost;         //是否使用TopMost风格
	BOOL m_useSteps;        //是否使用抽屉效果
	BOOL m_toolWnd;         //是否使用Tool Window 风格
	
public:
};
