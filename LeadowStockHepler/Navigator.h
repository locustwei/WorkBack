/************************************************************************
导航栏

************************************************************************/
#pragma once

enum NAV_NODE_TYPE
{
	NNT_1_HQ,        //行情
	NNT_1_ZH,        //账户
	NNT_2_ZJGF,      //资金股份
	NNT_2_JYLS,      //交易历史
	NNT_2_GZ,        //关注股票
	NNT_1_CLJY,      //策略交易
	NNT_2_CJX        //策略交易脚本 
};

struct NAV_NODE_DATA
{
	NAV_NODE_TYPE type;
	LPVOID data;
	NAV_NODE_DATA* pParent;
};

inline NAV_NODE_DATA* NewNodeData(NAV_NODE_TYPE type) 
{
	NAV_NODE_DATA* p = new NAV_NODE_DATA; 
	ZeroMemory(p, sizeof(NAV_NODE_DATA));
	p->type = type;
	return p;
};

class CViewTree : public CTreeCtrl
{
public:
	CViewTree(){};
	virtual ~CViewTree(){};

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

		NMHDR* pNMHDR = (NMHDR*)lParam;
		ASSERT(pNMHDR != NULL);

		if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
		{
			GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}

		return bRes;
	};

};

class CNavigatorToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CNavigator : public CDockablePane
{
public:
	CNavigator();
	virtual ~CNavigator();

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSelectItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDbClickItem(NMHDR *pNMHDR, LRESULT *pResult);
private:
	CViewTree m_NavigateTree;
	CImageList m_TreeImages;
	CNavigatorToolBar m_wndToolBar;
	CMap<HTREEITEM, HTREEITEM, CMDIChildWnd*, CMDIChildWnd*> m_NodeWnd;

	void InitNavigateTree();

	void LoadStrategyScript(HTREEITEM hStrategy);



	DECLARE_MESSAGE_MAP()
};

