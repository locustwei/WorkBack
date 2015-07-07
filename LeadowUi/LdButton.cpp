// LdButton.cpp : 实现文件
//

#include "stdafx.h"
#include "LdButton.h"
#include "LdGraphicFuncs.h"


// CLdButton


CLdButton::CLdButton()
{
	m_ClassName = L"BUTTON";
	m_Style = WS_CHILD|WS_VISIBLE|BS_OWNERDRAW;
	m_ExStyle = WS_EX_TRANSPARENT;

	OnClick=NULL;
	OnBeforeDraw=NULL;
	OnAfterDraw=NULL;
	OnEnter=NULL;
	OnLeave=NULL;

	m_CurStatus=CS_NORMAL;
	m_Selected=FALSE;

	ImgNormal=NULL;
	ImgDown=NULL;
	ImgEnter=NULL;
	ImgDisable=NULL;
}

CLdButton::~CLdButton()
{
}

void CLdButton::OnLButtonDown( UINT x, UINT y )
{
	SetCapture(m_hWnd);
	ChangStatus(CS_MOUSEDOWN);
}

void CLdButton::OnLButtonUp( UINT x, UINT y )
{
	POINT point = {x, y};
	if((GetCapture()==this->m_hWnd)&&(m_CurStatus==CS_MOUSEDOWN)){
		if(PtInThis(point))
			DoClick();
	}
	ChangStatus(CS_MOUSEMOVE);
}

LRESULT CLdButton::WindowProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch(message){
	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;;
	case WM_LBUTTONUP:
		OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	case WM_MOUSEMOVE:
		OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	}
	return CLdWnd::WindowProc(hwnd, message, wParam, lParam);
}

void CLdButton::DoClick()
{
	if(OnClick!=NULL)
		OnClick->OnNotify(this);
}

void CLdButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct)
{

	CRect rect=CRect(lpDrawItemStruct->rcItem);

	if(DoBeforeDraw(lpDrawItemStruct->hDC, rect)){
		return;
	}
	
	if(!DoDrawControl(lpDrawItemStruct->hDC, rect))
		DefaultDraw(lpDrawItemStruct);

	DoAfterDraw(lpDrawItemStruct->hDC, rect);
}

void CLdButton::DefaultDraw( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	UINT uStyle = DFCS_BUTTONPUSH;


	// If drawing selected, add the pushed style to DrawFrameControl.
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		uStyle |= DFCS_PUSHED;

	// Draw the button frame.
	::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, 
		DFC_BUTTON, uStyle);

	// Get the button's text.
	TCHAR strText[255] = {0};
	GetWindowText(m_hWnd, strText, 255);

	// Draw the button text using the text color red.
	//COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, RGB(255,0,0));
	::DrawText(lpDrawItemStruct->hDC, strText, -1, 
		&lpDrawItemStruct->rcItem, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	//::SetTextColor(lpDrawItemStruct->hDC, crOldColor);
}

BOOL CLdButton::DoBeforeDraw(HDC pDc, CRect rect)
{
	if(OnBeforeDraw!=NULL)
		return OnBeforeDraw->OnDraw(this, pDc, rect);
	return FALSE;
}

void CLdButton::DoAfterDraw( HDC pDc, CRect rect )
{
	if(OnAfterDraw!=NULL)
		OnAfterDraw->OnDraw(this, pDc, rect);
}

void CLdButton::OnMouseMove( UINT x, UINT y )
{
	POINT point = {x, y};
	if(GetCapture()!=this->m_hWnd){
		SetCapture(m_hWnd);
		DoEnter();
		ChangStatus(CS_MOUSEMOVE);
	}else if(!PtInThis(point)){
		ReleaseCapture();
		DoLeave();
		ChangStatus(CS_NORMAL);
	}
}

BOOL CLdButton::PtInThis( POINT pt, BOOL bScreen )
{
	if(bScreen)
		ScreenToClient(m_hWnd, &pt);
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	return PtInRect(&rect, pt);
}

void CLdButton::ChangStatus( CONTROLSTATUS newStatus )
{
	if((m_CurStatus==CS_DISABLEED)||(m_CurStatus==newStatus))  //EnableWindow改变CS_DISABLEED状态
		return;
	m_CurStatus=newStatus;
	UpdateWindow(m_hWnd);
}

void CLdButton::DoEnter()
{
	if(OnEnter!=NULL)
		OnEnter->OnNotify(this);
}

void CLdButton::DoLeave()
{
	if(OnLeave!=NULL)
		OnLeave->OnNotify(this);
}

BOOL CLdButton::EnableWindow( BOOL bEnable /*= TRUE*/ )
{
	BOOL ret=::EnableWindow(m_hWnd, bEnable);
	if(!ret)
		return ret;
	if(bEnable)
		ChangStatus(CS_NORMAL);
	else
		ChangStatus(CS_DISABLEED);
	return TRUE;
}

BOOL CLdButton::DoDrawControl( HDC hDc, CRect rect )
{
	CLdImage* pImg=GetCurstatusImage();
	if((pImg==NULL)||pImg->IsNull())
		return FALSE;
	//pImg->DrawSplit(hDc, rect);
	HDC pDc=GetDC(m_hWnd);
	DrawCtrlBackgnd(pDc, rect, pImg);
	ReleaseDC(m_hWnd, pDc);
	//delete pDc;

	TCHAR szTxt[255] = {0};
	GetWindowText(m_hWnd, szTxt, 255);
	::SetBkMode(hDc, TRANSPARENT);
	::DrawText(hDc, szTxt, -1, rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
	return TRUE;
}

CLdImage* CLdButton::GetCurstatusImage()
{
	CONTROLSTATUS status=m_CurStatus;
	
	if((m_Selected)&&(status==CS_NORMAL))
		status=CS_MOUSEMOVE;

	CLdImage* Ret=NULL;
	switch(status){
	case CS_DISABLEED:
		if(ImgDisable!=NULL)
			Ret=ImgDisable;
		break;
	case CS_NORMAL:
		if(ImgNormal!=NULL)
			Ret=ImgNormal;
		break;
	case CS_MOUSEDOWN:
		if(ImgDown!=NULL)
			Ret=ImgDown;
		break;
	case CS_MOUSEMOVE:
		if(ImgEnter!=NULL)
			Ret=ImgEnter;
		break;
	}
	return Ret;
}

/*
void CLdButton::InitStaticImages()
{
	CLdImage tmp;
	tmp.Load(L"d:\\button.png");
	//tmp.LoadFromResource(AfxGetInstanceHandle(), IDB_BUTTON);
	if(tmp.IsNull())
		return;
	for(int i=0; i<4; i++){
		CLdImage* img=new CLdImage();
		img->Create(187, 36, tmp.GetBPP(), AC_SRC_ALPHA);
		for(int x=0; x<img->GetWidth(); x++)
			for(int y=0; y<img->GetHeight(); y++){
				PBYTE pDes=(PBYTE)img->GetPixelAddress(x, y);
				PBYTE pScr=(PBYTE)tmp.GetPixelAddress(i*187+x, y);
				pDes[0]=pScr[0]*pScr[3]/0xFF;
				pDes[1]=pScr[1]*pScr[3]/0xFF;
				pDes[2]=pScr[2]*pScr[3]/0xFF;
				pDes[3]=pScr[3];
			}
		switch(i){
		case 0:
			static_ImgNormal=img;
			break;
		case 1:
			static_ImgEnter=img;
			break;
		case 2:
			static_ImgDown=img;
			break;
		case 3:
			static_ImgDisable=img;
			break;
		}
	}
	tmp.Destroy();
}
*/
