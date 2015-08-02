// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

#include "stdafx.h"
#include "mainfrm.h"
#include "Navigator.h"
#include "Resource.h"
#include "LeadowStockHepler.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define IDC_TREEVIEW 4
/////////////////////////////////////////////////////////////////////////////
// CFileView

CNavigator::CNavigator()
{
}

CNavigator::~CNavigator()
{
}

BEGIN_MESSAGE_MAP(CNavigator, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(ID_DUMMY_COMPILE, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_PAINT()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREEVIEW, OnSelectItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_TREEVIEW, OnDbClickItem)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CNavigator::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建视图:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_NavigateTree.Create(dwViewStyle, rectDummy, this, IDC_TREEVIEW))
	{
		TRACE0("未能创建导航视图\n");
		return -1;      // 未能创建
	}

	// 加载视图图像:
	m_TreeImages.Create(IDB_NAV, 16, 0, RGB(255, 0, 255));
	m_NavigateTree.SetImageList(&m_TreeImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* 已锁定*/);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 所有命令将通过此控件路由，而不是通过主框架路由:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	// 填入一些静态树视图数据(此处只需填入虚拟代码，而不是复杂的数据)
	InitNavigateTree();
	AdjustLayout();

	return 0;
}

void CNavigator::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CNavigator::InitNavigateTree()
{
	HTREEITEM hQuotation = m_NavigateTree.InsertItem(_T("市场行情"), 0, 0);
	m_NavigateTree.SetItemState(hQuotation, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hUser = m_NavigateTree.InsertItem(_T("账户信息"), 1, 1);
	m_NavigateTree.SetItemState(hUser, TVIS_BOLD, TVIS_BOLD);

	m_NavigateTree.InsertItem(_T("资金股份"), 0, 0, hUser);
	m_NavigateTree.InsertItem(_T("交易记录"), 0, 0, hUser);
	m_NavigateTree.InsertItem(_T("关注股票"), 0, 0, hUser);

	HTREEITEM hStrategy = m_NavigateTree.InsertItem(_T("自动交易"), 2, 2);
	m_NavigateTree.SetItemState(hStrategy, TVIS_BOLD, TVIS_BOLD);
	LoadStrategyScript(hStrategy);

	m_NavigateTree.Expand(hUser, TVE_EXPAND);
	m_NavigateTree.Expand(hStrategy, TVE_EXPAND);
}

void CNavigator::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_NavigateTree;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 选择已单击的项:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CNavigator::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_NavigateTree.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CNavigator::OnProperties()
{
	AfxMessageBox(_T("属性...."));

}

void CNavigator::OnFileOpen()
{
	// TODO: 在此处添加命令处理程序代码
}

void CNavigator::OnFileOpenWith()
{
	// TODO: 在此处添加命令处理程序代码
}

void CNavigator::OnDummyCompile()
{
	// TODO: 在此处添加命令处理程序代码
}

void CNavigator::OnEditCut()
{
	// TODO: 在此处添加命令处理程序代码
}

void CNavigator::OnEditCopy()
{
	// TODO: 在此处添加命令处理程序代码
}

void CNavigator::OnEditClear()
{
	// TODO: 在此处添加命令处理程序代码
}

void CNavigator::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_NavigateTree.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CNavigator::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_NavigateTree.SetFocus();
}

void CNavigator::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE /* 锁定*/);

	m_TreeImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_NAV_32 : IDB_NAV;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("无法加载位图: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_TreeImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_TreeImages.Add(&bmp, RGB(255, 0, 255));

	m_NavigateTree.SetImageList(&m_TreeImages, TVSIL_NORMAL);
}

void CNavigator::OnSelectItemChanged( NMHDR *pNMHDR, LRESULT *pResult )
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CNavigator::OnDbClickItem( NMHDR *pNMHDR, LRESULT *pResult )
{
	HTREEITEM selItem=m_NavigateTree.GetSelectedItem();
	if(selItem==NULL)
		return;
	HTREEITEM hParent = m_NavigateTree.GetParentItem(selItem);
	if(hParent==NULL)
		return;

	CString txt=m_NavigateTree.GetItemText(selItem);
	//((CMainFrame*)theApp.m_pMainWnd)->CreateNewChild(RUNTIME_CLASS(CChildFrame), txt);
	((CMainFrame*)theApp.m_pMainWnd)->CreateDlgChild(RUNTIME_CLASS(CLdDialog));

	*pResult = 0;
}

void CNavigator::LoadStrategyScript(HTREEITEM hStrategy)
{
	POSITION pos = theApp.m_ScriptEng->m_Strategy.GetStartPosition();
	CStringA strKey;
	PSTRATEGY_STRCPIT pScript;
	while(pos)
	{
		theApp.m_ScriptEng->m_Strategy.GetNextAssoc(pos, strKey, pScript);
		HTREEITEM hItem = m_NavigateTree.InsertItem(CString(pScript->szName), 0, 0, hStrategy);
		m_NavigateTree.SetItemData(hItem, (DWORD)pScript);
	}
}


