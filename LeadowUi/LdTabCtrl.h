/********************************************************************
	created:	2013/12/07
	created:	7:12:2013   23:35
	author:		博文
	
	purpose:	页面控件。为了省去自绘CTabCtrl的麻烦，采用CLdButton和CWnd组合实现。
*********************************************************************/
#pragma once
#include "LdButton.h"
#include "LdPanel.h"
#include "..\PublicLib\comps\LdList.h"

class CLdSheet : public CLdPanel
{
};

// CLdTabCtrl

class CLdTabSheet : public CLdPanel
{
public:
	CLdTabSheet();
	virtual ~CLdTabSheet();
public:
	IItemChange* OnBeforeSheetChange;              //发布sheet改变事件（前）
	IItemChange* OnAfterSheetChange;               //发布sheet改变事件（后）

	UINT AddSheet( LPCTSTR lpCaption);              //增加sheet
	void SetBtnSize(CSize size);                    //设置按钮大小
	CSize GetBtnSize();                             //获取按钮大小
	int GetCurSheetId(CLdSheet** tabSheet);      //获取当前选中sheet的id，tabSheet为接收选中sheet，不需要传入NULL
	CLdButton* GetSheetBtn(int nId);                //获取按钮指针
	CLdSheet* GetSheet( int nId );                  //获取Sheet指针
	int SetCurSheet(int nId);                       //设置选中的Sheet
protected:
//BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,LRESULT* pResult);
	void OnSize(UINT nType, int cx, int cy); //
	CRect GetSheetRect();                            //获取Sheet区域（）
private:
	int m_CurSheetId;
	CSize m_BtnSize;
	CLdList<CLdSheet*> m_Sheets;          //已创建的sheet
	CLdList<CLdButton*> m_Btns;           //已创建的btn

	void ChangeSheet(int newId);                      //改变选中的sheet
	void SheetBtnClick( CLdButton* Sender );
private:
	INotifyEvent* m_SheetBtnClickHandler;   //类函数指针

};


