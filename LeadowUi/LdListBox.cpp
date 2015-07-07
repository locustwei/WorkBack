// LdListBox.cpp : 实现文件
//

#include "stdafx.h"
#include "LdListBox.h"
#include <Commctrl.h>


// CLdListBox

CLdListBox::CLdListBox()
{
	m_ClassName = L"LISTBOX";
	m_Style = WS_CHILD|WS_VISIBLE|WS_HSCROLL|LBS_OWNERDRAWFIXED|LBS_STANDARD|LBS_EXTENDEDSEL;
	m_ExStyle = LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT|WS_EX_TRANSPARENT |LVS_EX_HEADERDRAGDROP;

	InitMember();
}

CLdListBox::~CLdListBox()
{
}

void CLdListBox::DrawItemCaption(HDC pDc, CLdListBoxItem* item, UINT itemAction, UINT itemState)
{
	
	//COLORREF clBk=pDc->GetBkColor();
	COLORREF clTxt=GetTextColor(pDc);

	if ((itemAction | ODA_SELECT) && (itemState & ODS_SELECTED)){
		SetTextColor(pDc, ::GetSysColor(COLOR_HIGHLIGHT));
		//pDc->FillSolidRect(&item->rcCaption, ::GetSysColor(COLOR_HIGHLIGHT));
	}else{
		//pDc->FillSolidRect(&item->rcCaption, clBk);
	}

	if ((itemAction | ODA_FOCUS) && (itemState & ODS_FOCUS)){
		//HGDIOBJ br = CreateSolidBrush(RGB(255, 255, 0));
		//pDc->FrameRect(&item->rcCaption, &br);
	}
	SetBkMode(pDc, TRANSPARENT);
	DrawText(pDc, item->Caption, -1, &item->rcCaption,DT_LEFT|DT_SINGLELINE|DT_VCENTER);

	//pDc->SetBkColor(clBk);
	SetTextColor(pDc, clTxt);
}

void CLdListBox::InitMember()
{
	ImgBackgrdNoneItem=NULL;
	ImgBackgrd=NULL;
	ImgItemNormal=NULL;
	ImgItemMouseDown=NULL;
	ImgItemMouseOver=NULL;

	m_IsComboBoxList=FALSE;
	m_ShowIcon=TRUE;
	m_ShowCheckBox=FALSE;
	m_MuiltSelect=TRUE;
	m_nFixLeftPix=2;
	m_MouseMoveItem=NULL;
	m_MouseDownItem=NULL;

	OnBeforeItemDraw=NULL;
	OnAfterItemDraw=NULL;
	OnEnter=NULL;
	OnLeave=NULL;
	OnClick=NULL;
	OnWndMouseMove=NULL;
	OnItemChecked=NULL;
}

void CLdListBox::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if((lpDrawItemStruct->itemData==NULL)||(lpDrawItemStruct->itemData==0xFFFFFFFF))
		return;
	CLdListBoxItem* item=(CLdListBoxItem*)lpDrawItemStruct->itemData;

	item->rcItem=lpDrawItemStruct->rcItem;
	int nLeft=lpDrawItemStruct->rcItem.left+m_nFixLeftPix;
	if(m_ShowCheckBox){
		item->rcCheckBox.left=nLeft;
		item->rcCheckBox.top=(lpDrawItemStruct->rcItem.bottom+lpDrawItemStruct->rcItem.top) / 2-8;
		item->rcCheckBox.right=item->rcCheckBox.left + 16;
		item->rcCheckBox.bottom=item->rcCheckBox.top + 16;

		nLeft+=16+1;
	}
	if(m_ShowIcon){
		item->rcIcon.left=nLeft;
		item->rcIcon.top=(lpDrawItemStruct->rcItem.bottom+lpDrawItemStruct->rcItem.top) / 2-8;
		item->rcIcon.right=item->rcIcon.left+16;
		item->rcIcon.bottom=item->rcIcon.top+16;

		nLeft+=16+1;
	}
	item->rcCaption.left=nLeft;
	item->rcCaption.top=lpDrawItemStruct->rcItem.top;
	item->rcCaption.right=lpDrawItemStruct->rcItem.right;
	item->rcCaption.bottom=lpDrawItemStruct->rcItem.bottom;

	HDC dc = lpDrawItemStruct->hDC;

	DoDrawItem(dc, item, lpDrawItemStruct->itemAction, lpDrawItemStruct->itemState);

	//DefaultDrawItem(lpDrawItemStruct);
}

void CLdListBox::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
	lpMeasureItemStruct->itemHeight=40;
}

int CLdListBox::CompareItem( LPCOMPAREITEMSTRUCT lpCompareItemStruct )
{
	return 0;//CListBox::CompareItem(lpCompareItemStruct);
}

BOOL CLdListBox::OnEraseBkgnd( HDC pDC )
{

	RECT cr;
	GetClientRect(m_hWnd, &cr);
	//FillSolidRect(pDC, &cr, ::GetSysColor(COLOR_WINDOW));
	if(m_IsComboBoxList){
		//COLORREF clBk=GetBkColor();
		return FALSE;
	}

	if((GetCount()>0)&&(ImgBackgrdNoneItem!=NULL))
		ImgBackgrdNoneItem->DrawSplit(pDC, cr);
	else if(ImgBackgrd!=NULL)
		ImgBackgrd->DrawSplit(pDC, cr);
	return TRUE;
}

CLdListBoxItem* CLdListBox::AddString( LPCTSTR lpszItem )
{
	return InsertString(-1, lpszItem);
}

CLdListBoxItem* CLdListBox::InsertString( int nIndex, LPCTSTR lpszItem )
{
	CLdListBoxItem* aItem=new CLdListBoxItem();
	nIndex = ListBox_InsertString(m_hWnd, nIndex, lpszItem);
	ListBox_SelectItemData(m_hWnd, nIndex, aItem);
	return aItem;
}

CLdListBoxItem* CLdListBox::InsertItem( int nIndex/*=-1*/ )
{
	return InsertString(nIndex, NULL_STRING);
}

int CLdListBox::InsertItem( CLdListBoxItem* pItem, int nIndex )
{
	if(!pItem)
		return -1;
	nIndex = ListBox_InsertString(m_hWnd, nIndex, pItem->Caption);
	ListBox_SelectItemData(m_hWnd, nIndex, pItem);
	return nIndex;
}

CLdListBoxItem* CLdListBox::GetItem( int nIdx )
{
	CLdListBoxItem* Ret = (CLdListBoxItem*)ListBox_GetItemData(m_hWnd, nIdx);
	if((DWORD)Ret==0xFFFFFFFF)
		return NULL;
	else
		return Ret;
}

int CLdListBox::DeleteString( UINT nIndex )
{
	CLdListBoxItem* aItem=GetItem(nIndex);
	if(aItem)
		delete aItem;
	return ListBox_DeleteString(m_hWnd, nIndex);
}

void CLdListBox::DoDrawItem( HDC pDc, CLdListBoxItem* item, UINT itemAction, UINT itemState )
{
	if(item==NULL)
		return;

	if(OnBeforeItemDraw!=NULL)
		if(OnBeforeItemDraw->OnDraw(item, pDc, item->rcItem))
			return;

	CLdImage* img=GetItemImage(item);
	if(img!=NULL)
		img->DrawSplit(pDc, item->rcItem);
	if(m_ShowCheckBox){
		DWORD dwState=DFCS_TRANSPARENT;
		if(m_MuiltSelect)
			dwState|=DFCS_BUTTONCHECK;
		else
			dwState|=DFCS_BUTTONRADIO;
		if(item->Checked)
			dwState|=DFCS_CHECKED;
		DrawFrameControl(pDc, &item->rcCheckBox, DFC_BUTTON, dwState);
	}
	if(m_ShowIcon){
		if(item->imgIcon!=NULL)
			item->imgIcon->DrawSplit(pDc, item->rcIcon);
	}
	DrawItemCaption(pDc, item, itemAction, itemState);

	if(OnAfterItemDraw!=NULL)
		OnAfterItemDraw->OnDraw(item, pDc, item->rcItem);

}

void CLdListBox::OnMouseMove( UINT nFlags, CPoint point )
{
	if(m_IsComboBoxList)      //是ComboBox DropDownList，不处理此消息（由ComboBox代劳）
		return ;

	if(GetCapture()!=this->m_hWnd){
		SetCapture(this->m_hWnd);
		if(OnEnter!=NULL)
			OnEnter->OnNotify(this);
		return;
	}
	CRect cr;
	GetWindowRect(m_hWnd, cr);
	if(!PtInRect(cr, point)){
		m_MouseMoveItem=NULL;
		m_MouseDownItem=NULL;
		ReleaseCapture();
		if(OnLeave!=NULL)
			OnLeave->OnNotify(this);
		//return;
	}

	UINT nIndex = ItemFromPoint(point);
	CLdListBoxItem* item=GetItem(nIndex);
	if(item!=m_MouseMoveItem){
		CLdListBoxItem* tmp=m_MouseMoveItem;
		m_MouseMoveItem=item;
		if(tmp!=NULL)
			InvalidateRect(m_hWnd, &tmp->rcItem, 0);
		if(m_MouseMoveItem!=NULL)
			InvalidateRect(m_hWnd, &m_MouseMoveItem->rcItem, 0);
	}
	if(m_MouseMoveItem!=NULL){
		if(OnWndMouseMove!=NULL)
			OnWndMouseMove->OnItemMouseEvent(this, m_MouseMoveItem, point);
	}

}

void CLdListBox::OnLButtonUp( UINT nFlags, CPoint point )
{
	UINT nIndex = ItemFromPoint(point);
	CLdListBoxItem* item=GetItem(nIndex);
	if(item==m_MouseDownItem){
		if((item!=NULL)&&(m_ShowCheckBox)&&(PtInRect(&item->rcCheckBox, point))){
			CheckItem(item, !item->Checked);
			if(OnItemChecked!=NULL)
				OnItemChecked->OnItemMouseEvent(this, item, point);
		}else if(OnClick)
			OnClick->OnItemMouseEvent(this, item, point);
	}
	item=m_MouseDownItem;
	m_MouseDownItem=NULL;
	if(item!=NULL)
		InvalidateRect(m_hWnd, &item->rcItem, 0);
	//if(! ((m_IsComboBoxList)&&(m_ShowCheckBox)&&(m_MuiltSelect)))      //多选的ComboBox DropDownList，把消息吃掉不让他CloseUp
		//CListBox::OnLButtonUp(nFlags, point);
}

void CLdListBox::OnLButtonDown( UINT nFlags, CPoint point )
{
	UINT nIndex = ItemFromPoint(point);
	CLdListBoxItem* item=GetItem(nIndex);
	m_MouseDownItem=item;
	if(m_MouseDownItem)
		InvalidateRect(m_hWnd, &m_MouseDownItem->rcItem, 0);
}

CLdImage* CLdListBox::GetItemImage( CLdListBoxItem* item )
{
	if(item==NULL)
		return NULL;
	CLdImage* ret=ImgItemNormal;
	if(item==m_MouseDownItem)
		ret = ImgItemMouseDown;
	if(item==m_MouseMoveItem)
		ret = ImgItemMouseOver;
	return ret;
}

void CLdListBox::CheckItem( UINT nIndex, BOOL bCheck )
{
	CLdListBoxItem* item=GetItem(nIndex);
	CheckItem(item, bCheck);
}

void CLdListBox::CheckItem( CLdListBoxItem* item, BOOL bCheck )
{
	if((item==NULL)||(item->Disabled)||(item->Checked==bCheck))
		return;

	if(bCheck==FALSE){                    //取消不影响谁
		item->Checked=FALSE;
		InvalidateRect(m_hWnd, &item->rcItem, 0);
		return;
	}
	if(!m_MuiltSelect){                  //bCheck==TRUE
		for(int i=0; i<GetCount(); i++){
			CLdListBoxItem* aItem=GetItem(i);
			if(aItem->Checked){
				aItem->Checked=FALSE;
				InvalidateRect(m_hWnd, &aItem->rcItem, 0);
			}
		}
	}
	item->Checked=TRUE;
	InvalidateRect(m_hWnd, &item->rcItem, 0);
}

UINT CLdListBox::ItemFromPoint( CPoint pt )
{
	return SendMessage(m_hWnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
}

int CLdListBox::GetCheckedItems( CLdList<CLdListBoxItem*>& items )
{
	int nRet=0;
	for(int i=0; i<GetCount(); i++){
		CLdListBoxItem* item=GetItem(i);
		if(item->Checked){
			items.Add(item);
			nRet++;
		}
	}
	return nRet;
}

void CLdListBox::OnDestroy()
{
	while(GetCount()>0){
		DeleteString(0);
	}
}

int CLdListBox::GetCount()
{
	return ListView_GetItemCount(m_hWnd);
}

int CLdListBox::GetCurSel()
{
	return (int)::SendMessage(m_hWnd, CB_GETCURSEL, 0, 0);
}

int CLdListBox::GetItemHeight( int nidx )
{
	return (int)::SendMessage(m_hWnd, CB_GETITEMHEIGHT, nidx, 0L);
}

void CLdListBox::SetItemHeight( int nidx, int nheight )
{
	ListBox_SetItemHeight(m_hWnd, nidx, nheight);
}


// CLdListBox 消息处理程序


