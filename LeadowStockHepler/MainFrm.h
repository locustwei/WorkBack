
#pragma once
#include "Navigator.h"
#include "ui\LdDialog.h"

class CMainFrame : public CMDIFrameWndEx
{
	
public:
	CMainFrame();
	virtual ~CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	CMDIChildWnd* CreateNewChild(CRuntimeClass* pClass, CString strTitle);
	CMDIChildWnd* DockDlgChild( CLdDialog* dlg, CString strTitle = NULL );
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	//CMFCRibbonBar     m_wndRibbonBar;
	//CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CNavigator         m_Navigator;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnShowView(UINT id);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};


