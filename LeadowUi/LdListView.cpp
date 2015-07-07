/********************************************************************
	created:	2015/06/14
	created:	14:6:2015   18:14
	file base:	LdListView
	author:		博文
	
	purpose:	ListView 不拥有数据（即不InsertItem）而是Draw的时候给出相应信息
*********************************************************************/

#include "stdafx.h"
#include "LdListView.h"
#pragma comment(lib, "Comctl32.lib")

// CLdListView


CLdListCtrl::CLdListCtrl():m_IconSize(16, 16)
{
	m_ClassName = L"LISTCTRL";
	m_Style = WS_VISIBLE|WS_BORDER|WS_TABSTOP| LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL;
	m_ExStyle = LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;

	m_drawItem = new CListCtrlItem();

	m_ImageList = ImageList_Create(32, 32, ILC_COLOR16 | ILC_MASK, 0, 0 );

	m_ShowIcon = TRUE;                   //画图标
	m_MuiltSelect = FALSE;                //是否多选(默认true)
	m_nFixLeftPix = 2;                 //；
	m_DrawItemLine = FALSE;               //是否画Item间的分隔写

	OnBeforeItemDraw = NULL;          //发布ItemDraw事件
	OnAfterItemDraw = NULL;           //	
	OnWndMouseMove = NULL;       //发布鼠标移动事件（item可能为空）
	OnEnter = NULL;          //发布鼠标进入事件
	OnLeave = NULL;          //发布鼠标离开事件
	OnClick = NULL;          //鼠标点击（item可能为空）
	OnItemChecked = NULL;    //Item的CheckBox点击事件
}


CLdListCtrl::~CLdListCtrl()
{
}

void CLdListCtrl::Create( HWND pParent )
{
	CLdWnd::Create(pParent);
	ListView_SetImageList(m_hWnd, m_ImageList, LVSIL_SMALL);
	//SetImageList(&m_ImageList, LVSIL_SMALL);
}

BOOL CLdListCtrl::OnChildNotify(WPARAM wParam, LPARAM lParam, LRESULT* result )
{
	switch(((NMHDR*)lParam)->code){
	case NM_CUSTOMDRAW:
		if(CustomDrawItem((LPNMLVCUSTOMDRAW)lParam, result))
			return TRUE;
		break;
	case LVN_GETDISPINFO:
		GetDispInfo((NMLVDISPINFO*)lParam);
		*result=0;
		break;
	case LVN_ODFINDITEM:
		*result=OdFinditem((LPNMLVFINDITEM)lParam);
		break;
	}
	return TRUE;
}

BOOL CLdListCtrl::CustomDrawItem( LPNMLVCUSTOMDRAW nmcd, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW pLVCD = nmcd;
	*pResult = 0;
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage ){
		*pResult = CDRF_NOTIFYITEMDRAW;
	}else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage ){
		HDC pDc = pLVCD->nmcd.hdc;
		RECT cr;
		ListView_GetItemRect(m_hWnd, pLVCD->nmcd.dwItemSpec, &cr, LVIR_BOUNDS);
		
		DoDrawItem(pLVCD->nmcd.dwItemSpec, pDc, cr);

		*pResult = CDRF_SKIPDEFAULT;
	}
	return TRUE;
}

void CLdListCtrl::GetDispInfo( NMLVDISPINFO* plvdi )
{
}

BOOL CLdListCtrl::IsCheckBoxesVisible()
{
	DWORD style = GetWindowLong(m_hWnd, GWL_EXSTYLE);

	return (style & LVS_EX_CHECKBOXES) != 0;
}

int CLdListCtrl::OdFinditem(LPNMLVFINDITEM pFindInfo)
{
	return -1;
}


void CLdListCtrl::DoDrawItem( DWORD dwItemId, HDC pDc, RECT cr )
{
	cr.left += m_nFixLeftPix;

	m_drawItem->dwId = dwItemId;
	m_drawItem->rcItem=cr;
	//计算Item各个部分的Rect 为Draw做准备-----------------------------------------------------------------------------------------
	if(IsCheckBoxesVisible()){   
		GetCheckBoxRect(&m_drawItem->rcItem, m_drawItem->rcCheckBox);
	}else
		m_drawItem->rcCheckBox.SetRect(0, 0, 0, 0);
	if(m_ShowIcon){
		m_drawItem->rcIcon.top = m_drawItem->rcItem.top + (m_drawItem->rcItem.Height()-m_IconSize.cy) / 2;
		m_drawItem->rcIcon.bottom = m_drawItem->rcIcon.top + m_IconSize.cy;
		m_drawItem->rcIcon.left = m_drawItem->rcItem.left + m_drawItem->rcCheckBox.Width();
		m_drawItem->rcIcon.right = m_drawItem->rcIcon.left + m_IconSize.cx;
	}else
		m_drawItem->rcIcon.SetRect(0, 0, 0, 0);
	int nWidth = ListView_GetColumnWidth(m_hWnd, 0);
	m_drawItem->rcCaption = m_drawItem->rcItem;
	m_drawItem->rcCaption.left = m_drawItem->rcCaption.left + m_drawItem->rcCheckBox.Width()+m_drawItem->rcIcon.Width();
	m_drawItem->rcCaption.right = m_drawItem->rcItem.left + nWidth;

	//m_drawItem->SubItems.SetSize(GetColumnCount()-1);
	int nLeft = m_drawItem->rcCaption.right;
	for(int i=0; i<m_drawItem->SubItems.GetCount()-1; i++){
		m_drawItem->SubItems[i].rcItem.SetRect(nLeft, cr.top, nLeft + ListView_GetColumnWidth(m_hWnd, i+1), cr.bottom);
		nLeft = m_drawItem->SubItems[i].rcItem.right;
	}
	
	//开始draw-----------------------------------------------------------------------------------------------------------------------
	if(OnBeforeItemDraw!=NULL){
		if(OnBeforeItemDraw->OnDraw(m_drawItem, pDc, CRect(cr)))
			return;
	}else  //如果没有这个事件得不到Item的相关信息就别画了。
		return; 

	if(IsCheckBoxesVisible()){
		DWORD dwState=DFCS_TRANSPARENT | DFCS_BUTTONCHECK;
		if(m_drawItem->Checked)
			dwState|=DFCS_CHECKED;
		DrawFrameControl(pDc, &m_drawItem->rcCheckBox, DFC_BUTTON, dwState);
	}
	if((m_ShowIcon)&&(m_drawItem->imgIcon!=NULL)){
		m_drawItem->imgIcon->Draw(pDc, m_drawItem->rcIcon);
	}
	if(m_drawItem->Caption){
		DrawText(pDc, m_drawItem->Caption, -1, &m_drawItem->rcCaption, DT_LEFT | DT_VCENTER);
	}
	for(int i=0; i<m_drawItem->SubItems.GetCount(); i++){
		DrawText(pDc, m_drawItem->SubItems[i].Caption, -1, &m_drawItem->SubItems[i].rcItem, DT_LEFT | DT_VCENTER);
	}

	if(OnAfterItemDraw!=NULL)
		OnAfterItemDraw->OnDraw(m_drawItem, pDc, cr);
}

void CLdListCtrl::GetCheckBoxRect( CRect* crBound, CRect& crCheck )
{
	if(IsCheckBoxesVisible()){
		crCheck.top = crBound->top + (crBound->Height()-16)/2;
		crCheck.bottom = crCheck.top + 16;
		crCheck.left = crBound->left;
		crCheck.right = crCheck.left + 16;
	}
}

int CLdListCtrl::GetColumnCount()
{
	HWND pHeaderCtrl = ListView_GetHeader(m_hWnd);
	return Header_GetItemCount(pHeaderCtrl);
}

int CLdListCtrl::InsertColumn( int nCol, LPCTSTR szHead, int nFormat, int nWidth, int nSubItem )
{
	LVCOLUMN column;
	column.mask = LVCF_TEXT|LVCF_FMT;
	column.pszText = (LPTSTR)szHead;
	column.fmt = nFormat;
	if (nWidth != -1)
	{
		column.mask |= LVCF_WIDTH;
		column.cx = nWidth;
	}
	if (nSubItem != -1)
	{
		column.mask |= LVCF_SUBITEM;
		column.iSubItem = nSubItem;
	}
	return ListView_InsertColumn(m_hWnd, nCol, &column);
}

LRESULT CLdListCtrl::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	LRESULT result;
	switch(uMsg){
	case WM_NOTIFY:
		if(OnChildNotify(wParam, lParam, &result))
			return result;
		break;
	}
	return CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}

