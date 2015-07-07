// LdBtnBar.cpp : 实现文件
//

#include "stdafx.h"
#include "LdBtnBar.h"
#include "LdGraphicFuncs.h"


// CLdBtnBar

CLdBtnBar::CLdBtnBar():m_ItemSize(48, 60)
{
	InitMember();
}

CLdBtnBar::~CLdBtnBar()
{
	for(int i = 0; i<m_Items.GetCount(); i++)
		delete m_Items[i];
	m_Items.Clear();
}


void CLdBtnBar::InitMember()
{
	m_ImgBackgrd=NULL;                    //背景图
	m_ImstItem=NULL;                  //Btn图标。
	m_BtnAlign=taLeft|taVCenter;
	m_BtnCaptionAlign=taHCenter|taBottom;
	m_ShowBtnCaption=TRUE;                   //是否显示Btn的Caption。默认True
	OnItemClicked=NULL;               //发布Item被点击事件
}

void CLdBtnBar::OnMouseMove( UINT nFlags, POINT point )
{
	if(GetCapture()!=m_hWnd)
		SetCapture(m_hWnd);
	else{
		CRect cr;
		GetClientRect(m_hWnd, &cr);
	
		if(!PtInRect(cr, point)){
			ReleaseCapture();
			if(m_CaptureItem){
				m_CaptureItem->csStatus=CS_NORMAL;
				DrawItem(m_CaptureItem);
				m_CaptureItem=NULL;
			}
			return;
		}else{
			LPBTNBARITEM lpItem=GetItemFrom(point);
			if(lpItem!=m_CaptureItem){
				if(m_CaptureItem){
					m_CaptureItem->csStatus=CS_NORMAL;
					DrawItem(m_CaptureItem);
				}
				m_CaptureItem=lpItem;
				if(lpItem){
					lpItem->csStatus=CS_MOUSEMOVE;
					DrawItem(lpItem);
				}
			}
		}
	}
}

void CLdBtnBar::OnPaint()
{
	PAINTSTRUCT ps;
	HDC pDc=BeginPaint(m_hWnd, &ps);

	if(m_ImgBackgrd==NULL){
		m_ImgBackgrd=new CLdImage();
		m_ImgBackgrd->Load(_T("..\\Images\\ToolbarBackgrd.png"));
		m_ImgBackgrd->m_ImgSplit.nFixLeft=53;
		m_ImgBackgrd->m_ImgSplit.nFixRight=120;
		m_ImgBackgrd->m_ImgSplit.nFixTop=10;
		m_ImgBackgrd->m_ImgSplit.nFixBottm=100;
		PngImgTransparency(m_ImgBackgrd);
	}

	CRect cr;
	GetClientRect(m_hWnd, &cr);
	DrawCtrlBackgnd(pDc, cr, m_ImgBackgrd);
	//DrawBackgrd(pDc);

	if(GetItemCount()==0)
		return;

	int nTop=0;                       //设置有错误时靠上
	if(cr.Height()<=m_ItemSize.cy)
		nTop=0;
	else{
		WORD vAlign=m_BtnAlign & 0xF0;         //垂直方向的排列方式
		switch(vAlign){
		case taVCenter:
			nTop=(cr.Height()-m_ItemSize.cy)/2;
			break;
		case taBottom:
			nTop=cr.Height()-m_ItemSize.cy;
		}
	}

	int nBtnWidth=m_ItemSize.cx*GetItemCount();
	int nLeft=0;                               //设置有错误时靠左
	if(nBtnWidth>=cr.Width())
		nLeft=0;
	else{
		WORD hAlign=m_BtnAlign & 0xF;          //水平方向的排列方式
		switch(hAlign){
		case taHCenter:
			nLeft=(cr.Width()-m_ItemSize.cx)/2;
			break;
		case taRight:
			nLeft=cr.Width()-m_ItemSize.cx;
			break;
		}
	}

	for(int i=0; i<m_Items.GetCount(); i++){
		LPBTNBARITEM lpItem=m_Items[i];
		lpItem->crRect=CRect(CPoint(i*nLeft, nTop), m_ItemSize);
		DrawItem(pDc, lpItem);
	}

	EndPaint(m_hWnd, &ps);
}

void CLdBtnBar::OnLButtonDown( UINT nFlags, POINT point )
{
	if((m_CaptureItem)&&(PtInRect(m_CaptureItem->crRect, point))){
		m_CaptureItem->csStatus=CS_MOUSEDOWN;
		DrawItem(m_CaptureItem);
	}
}

void CLdBtnBar::OnLButtonUp( UINT nFlags, POINT point )
{
	if((m_CaptureItem)&&(PtInRect(m_CaptureItem->crRect, point))){
		if(OnItemClicked)
			OnItemClicked->OnItemChang(this, GetItemIndex(m_CaptureItem));
	}
}

void CLdBtnBar::OnSize( UINT nType, int cx, int cy )
{
	UpdateWindow(m_hWnd);
}

LPBTNBARITEM CLdBtnBar::InsertItem( LPCTSTR szCaption, UINT nImage, UINT nAffter/*=-1*/ )
{
	LPBTNBARITEM lpItem=new BTNBARITEM();
	ZeroMemory(lpItem, sizeof(BTNBARITEM));
	lpItem->szCaption=szCaption;
	lpItem->nImage=nImage;
	if (nAffter==-1){
		m_Items.Add(lpItem);
	} 
	else{
		m_Items.Insert(nAffter, lpItem);
	}
	UpdateWindow(m_hWnd);
	return lpItem;
}

UINT CLdBtnBar::GetItemCount()
{
	return m_Items.GetCount();
}

LPBTNBARITEM CLdBtnBar::GetItem( int nIdx )
{
	if((nIdx<0)||(nIdx>=m_Items.GetCount()))
		return NULL;
	return m_Items[nIdx];
}

void CLdBtnBar::DeleteItem( int nIdx )
{
	if(nIdx<0||nIdx>=m_Items.GetCount())
		return;
	LPBTNBARITEM lpItem=m_Items[nIdx];
	m_Items.Remove(nIdx);
	if(lpItem)
		delete lpItem;
	UpdateWindow(m_hWnd);
}

void CLdBtnBar::UpdateItem( int nIdx )
{
	LPBTNBARITEM lpItem=GetItem(nIdx);
	if(!lpItem)
		return;
	DrawItem(lpItem);
}

LPBTNBARITEM CLdBtnBar::GetItemFrom( POINT pt )
{
	for(int i = 0; i < m_Items.GetCount(); i++)
		if(PtInRect(m_Items[i]->crRect, pt))
			return m_Items[i];
	return NULL;
}

CSize CLdBtnBar::GetItemSize()
{
	return m_ItemSize;
}

void CLdBtnBar::SetItemSize(CSize szie)
{
	m_ItemSize=szie;
	UpdateWindow(m_hWnd);
}

void CLdBtnBar::DrawItem( HDC pDc, LPBTNBARITEM pItem )
{
	//m_ImstItem->Draw(pDc, pItem->nImage, pItem->crRect.TopLeft(), ILD_SELECTED);
}

void CLdBtnBar::DrawItem( LPBTNBARITEM pItem )
{
	
}

UINT CLdBtnBar::GetItemIndex( LPBTNBARITEM lpItem )
{
	for(int i=0; i<m_Items.GetCount(); i++)
		if(lpItem==m_Items[i])
			return i;
	return -1;
}

void CLdBtnBar::DrawBackgrd( HDC pDc )
{
	if(m_ImgBackgrd==NULL){
		m_ImgBackgrd=new CLdImage();
		m_ImgBackgrd->Load(_T("..\\Images\\ToolbarBackgrd.png"));
		m_ImgBackgrd->m_ImgSplit.nFixLeft=53;
		m_ImgBackgrd->m_ImgSplit.nFixRight=120;
		m_ImgBackgrd->m_ImgSplit.nFixTop=10;
		m_ImgBackgrd->m_ImgSplit.nFixBottm=100;
		PngImgTransparency(m_ImgBackgrd);
	}
	CRect cr;
	GetClientRect(m_hWnd, &cr);
	m_ImgBackgrd->DrawSplit(pDc, cr);
	//m_ImgBackgrd->DrawIndirect();
}

BOOL CLdBtnBar::OnDrawBackgrd( HDC pDC )
{
	return TRUE;
}



// CLdBtnBar 消息处理程序


