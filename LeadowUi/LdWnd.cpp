#include "LdWnd.h"
#include <atltypes.h>

HINSTANCE CLdUiObject::hInstance = NULL;

CLdWnd::CLdWnd()
{
	m_hWnd=NULL;
	m_WndProc = NULL;
	m_Data=NULL;
	m_Tag=0;
	m_Anchors=akLeft|akTop;
	m_ClassName = NULL;
	m_Style = 0;
	m_ExStyle = 0;
}

void CLdWnd::OnParentSizing(UINT nSide, int nx, int ny)
{
	if(m_Anchors==(akLeft|akTop))
		return;                      //不需要动
	if(!m_hWnd)
		return;
	
	RECT cr = {0};
	GetWindowRect(m_hWnd, &cr);

	MapWindowPoints(NULL, m_hWnd, (LPPOINT)&cr, 2);

	if((m_Anchors&akLeft)==0)
		cr.left += nx;
	if((m_Anchors&akRight))
		cr.right += nx;
	if((m_Anchors&akTop)==0)
		cr.top += ny;
	if(m_Anchors&akBottom)
		cr.bottom += ny;

	MoveWindow(cr, TRUE);

}
/*
void CLdWnd::PrepareSkin(CFormPersistent* pPersistent)
{
	if(pPersistent==NULL)
		return;

	m_Name = pPersistent->m_Name;
	MoveWindow(m_hWnd, pPersistent->Left, pPersistent->Top, pPersistent->Width, pPersistent->Height, 0);
	if(!pPersistent->m_Caption[0])
		SetWindowTextA(m_hWnd, pPersistent->m_Caption.c_str());
	//[akLeft,akTop,akRight,akBottom]-------------------------------------------------------------------------------
	if(!pPersistent->m_Anchors[0]){
		m_Anchors=0;

		if(pPersistent->m_Anchors.find("akTop")!=-1)
			m_Anchors|=akTop;
		if(pPersistent->m_Anchors.find("akLeft")!=-1)
			m_Anchors|=akLeft;
		if(pPersistent->m_Anchors.find("akRight")!=-1)
			m_Anchors|=akRight;
		if(pPersistent->m_Anchors.find("akBottom")!=-1)
			m_Anchors|=akBottom;
	}
}
*/
LRESULT CALLBACK LdWindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	)
{
	CLdWnd* wnd = (CLdWnd*)GetWindowLong(hwnd, GWL_USERDATA);
	if(wnd==NULL)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	else{
		return wnd->WindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void CLdWnd::Create(HWND pParent)
{
	if(m_ClassName==NULL)
		return;

	m_hWnd = CreateWindowEx(m_ExStyle, m_ClassName, NULL, m_Style, 0, 0, 10, 10, pParent, NULL, NULL, 0);
	if(m_hWnd!=NULL){
		SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
		m_WndProc = (WNDPROC)SetWindowLongPtr(m_hWnd, GWL_WNDPROC, (LONG)&LdWindowProc);
		OnInitWnd();
	}
}

LRESULT CLdWnd::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT cr;
	switch(uMsg){
	case MM_ANCHORS:
		OnParentSizing(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_SIZING:
		OnSizing(wParam, (LPRECT)lParam);
		break;
	case WM_SIZE:
		GetWindowRect(m_hWnd, &cr);
		ResizeChilds(wParam, LOWORD(lParam)-(cr.right, cr.left), HIWORD(lParam)-(cr.bottom-cr.top));
		break;
	case WM_DESTROY:
		SetWindowLongPtr(m_hWnd, GWL_WNDPROC, (LONG)m_WndProc);
		m_hWnd = NULL;
		break;
	}
	if(m_WndProc!=NULL)
		return m_WndProc(hwnd, uMsg, wParam, lParam);
	else
		return FALSE;//DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CLdWnd::AttchWindow( HWND hwnd )
{
	m_hWnd = hwnd;
	if(m_hWnd!=NULL){
		SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
		m_WndProc = (WNDPROC)SetWindowLongPtr(m_hWnd, GWL_WNDPROC, (LONG)&LdWindowProc);
		OnInitWnd();
	}
}

CLdWnd::~CLdWnd( void )
{
	if(m_hWnd)
		DestroyWindow(m_hWnd);
	m_hWnd = NULL;
}

void CLdWnd::Invalidate()
{
	UpdateWindow(m_hWnd);
}

void CLdWnd::ShowWindow( int nCmdShow )
{
	::ShowWindow(m_hWnd, nCmdShow);
}

void CLdWnd::MoveWindow( RECT rc, BOOL update )
{
	::MoveWindow(m_hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, update);
}


void CLdWnd::OnSizing( UINT nSide, LPRECT lpRect )
{
	RECT cr;
	GetWindowRect(m_hWnd, &cr);
	int nx = (cr.left - lpRect->left) + (lpRect->right-cr.right);
	int ny = (cr.top - lpRect->top) + (lpRect->bottom-cr.bottom);
	ResizeChilds(nSide, nx, ny);
}

void CLdWnd::ResizeChilds(UINT nSide, int nx, int ny)
{
	if(nx==0&&ny==0)
		return;
	HWND hwnd = FindWindowEx(m_hWnd, NULL, NULL, NULL);
	while(hwnd!=NULL){

		SendMessage(hwnd, MM_ANCHORS, nSide, MAKELPARAM(nx, ny));
		hwnd = FindWindowEx(m_hWnd, hwnd, NULL, NULL);
	}
}

BOOL CLdWnd::OnInitWnd()
{
	return TRUE;
}

