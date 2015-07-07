/********************************************************************
	created:	2013/12/07
	created:	7:12:2013   23:54
	author:		博文
	
	purpose:	扩展CDialogEx实现没有资源ID的方式创建弹出窗口（去掉Dialog属性）
*********************************************************************/
#pragma once

#include <atlstr.h>
#include "LdWnd.h"

typedef struct _DLGITEM_MAP
{
	int ItemId;
	CLdWnd* wnd;
}DLGITEM_MAP, *PDLGITEM_MAP;

class CLdDialog: public CLdWnd
{ 
	friend INT_PTR CALLBACK LdDialogProc(
		__in  HWND hwndDlg,
		__in  UINT uMsg,
		__in  WPARAM wParam,
		__in  LPARAM lParam
		);
public: 
	CLdDialog();   // standard constructor 
	virtual ~CLdDialog();

	int ShowDialog(LPTSTR lpTemplate); 
	int DoModal(LPTSTR lpTemplate);


protected: 
	BOOL m_IsModal;
	void OnPaint();

	virtual BOOL OnInitDialog(); 
	virtual void CenterWindow();
	virtual LRESULT WindowProc(HWND   hwnd, UINT   uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR OnMenu( WORD mid );
	virtual INT_PTR OnAccelerator( WORD aid );
	virtual INT_PTR OnCommand( WORD ctrlid, HWND hwnd );
	virtual void InitDlgMembers( PDLGITEM_MAP maps );
	void DoCommand( WPARAM wParam, LPARAM lParam );
private:

}; 

