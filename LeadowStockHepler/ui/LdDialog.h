/********************************************************************
	created:	2013/12/07
	created:	7:12:2013   23:54
	author:		博文
	
	purpose:	扩展CDialogEx实现没有资源ID的方式创建弹出窗口（去掉Dialog属性）
*********************************************************************/
#pragma once
#include <afxdialogex.h>
#include "..\Resource.h"

typedef struct _DLGITEM_MAP
{
	int ItemId;
	CWnd* wnd;
}DLGITEM_MAP, *PDLGITEM_MAP;

class CLdDialog: public CDialogEx
{ 
	DECLARE_DYNAMIC(CLdDialog)

public:
	enum { IDD = IDD_DIALOG1 };
public: 
	CLdDialog();
	CLdDialog(UINT nIDTemplate, CWnd *pParent = NULL);
	CLdDialog(LPCTSTR lpszTemplateName, CWnd *pParentWnd = NULL);

	virtual ~CLdDialog();

	int ShowDialog(); 
	int DoModal();

	virtual BOOL Create( CWnd* pParentWnd = NULL );

protected: 
	BOOL m_IsModal;
	virtual BOOL OnInitDialog();

}; 

