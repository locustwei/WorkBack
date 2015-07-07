#include "..\stdafx.h"
#include "WndEffect.h"

CWndEffect::CWndEffect(HWND hwnd):CWndHook(hwnd)
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
	//m_isSizeChanged = FALSE;
	m_isSetTimer = FALSE;
	m_hsFinished = TRUE;
    m_hiding = FALSE;
	
	//m_oldWndHeight = WNDEFCT_MINCY;
	m_taskBarHeight = 30;
	m_edgeHeight = 0;
	m_edgeWidth  =0;
	m_hideMode = WNDEFCT_HM_NONE;

		//获得任务栏高度
    HWND p;
	p = FindWindowEx(NULL, NULL, L"Shell_TrayWnd",NULL);
	if(p != NULL)
	{
		RECT tRect;
		GetWindowRect(p, &tRect);
		m_taskBarHeight = tRect.bottom-tRect.top;
	}

		//获得边缘高度和宽度
	m_edgeHeight = GetSystemMetrics(SM_CYEDGE);
	m_edgeWidth  = GetSystemMetrics(SM_CXFRAME);

}

LRESULT CWndEffect::WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam)
{
	switch(nCode){
	case WM_NCHITTEST:
		
		OnNcHitTest(LOWORD(lparam), HIWORD(lparam));
		break;
	case WM_TIMER:
		OnTimer(wparam);
		break;
	case WM_MOVING:
		OnMoving(wparam, (LPRECT)lparam);
		break;
	}

    return CWndHook::WndPROC(hwnd, nCode, wparam, lparam);
}

BOOL CWndEffect::StartHook()
{	
	if(CWndHook::StartHook()) return false;

    return !m_nextHook;

}

void CWndEffect::OnMoving(UINT_PTR fwSide, LPRECT pRect) 
{
	RECT r=*pRect;
    FixMoving(fwSide,&r); //修正pRect
	SHORT ks = GetKeyState(VK_LBUTTON);
	if(!HIBYTE(ks)) //如果鼠标还按着就别改了。
		*pRect = r;
}

void CWndEffect::OnNcHitTest(int x, int y) 
{
	if(m_hideMode != WNDEFCT_HM_NONE && !m_isSetTimer &&
		//防止鼠标超出屏幕右边时向右边收缩造成闪烁
		x < GetSystemMetrics(SM_CXSCREEN) + WNDEFCT_INFALTE)
	{   //鼠标进入时,如果是从收缩状态到显示状态则开启Timer
		SetTimer(m_hWnd, 1,WNDEFCT_CM_ELAPSE,NULL);
		m_isSetTimer = TRUE;
		
		m_hsFinished = FALSE;
		m_hiding = FALSE;
		SetTimer(m_hWnd, 2,WNDEFCT_HS_ELAPSE,NULL); //开启显示过程
	}
	//return CDialog::OnNcHitTest(point);
}

void CWndEffect::OnTimer(UINT_PTR nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1 )
	{
        POINT curPos;
	    GetCursorPos(&curPos);
		
		RECT tRect;
		//获取此时窗口大小
		GetWindowRect(m_hWnd, &tRect);
		//膨胀tRect,以达到鼠标离开窗口边沿一定距离才触发事件
		
		InflateRect(&tRect, WNDEFCT_INFALTE,WNDEFCT_INFALTE); 
		
		if(!PtInRect(&tRect, curPos)) //如果鼠标离开了这个区域
		{
			KillTimer(m_hWnd, 1); //关闭检测鼠标Timer
			m_isSetTimer = FALSE;


			m_hsFinished = FALSE;			
			m_hiding = TRUE;
			SetTimer(m_hWnd, 2,WNDEFCT_HS_ELAPSE,NULL); //开启收缩过程
		}
	}

	if(nIDEvent == 2)
	{
		if(m_hsFinished) //如果收缩或显示过程完毕则关闭Timer	  
			KillTimer(m_hWnd, 2);
		else
            m_hiding ? DoHide() : DoShow();
	}

	//CDialog::OnTimer(nIDEvent);
}

void CWndEffect::DoHide()
{
	if(m_hideMode == WNDEFCT_HM_NONE)
		return;
	
	RECT tRect;
	GetWindowRect(m_hWnd, &tRect);
	
	INT height = tRect.bottom-tRect.top;
	INT width  = tRect.right-tRect.left;

	INT steps = 0;

	switch(m_hideMode)
	{
	case WNDEFCT_HM_TOP:
		steps = height/WNDEFCT_HS_STEPS;
		tRect.bottom -= steps;
		if(tRect.bottom <= m_edgeWidth)
		{   //你可以把下面一句替换上面的 ...+=|-=steps 达到取消抽屉效果
			//更好的办法是添加个BOOL值来控制,其他case同样.
            tRect.bottom = m_edgeWidth;
			m_hsFinished = TRUE;  //完成隐藏过程
		}
		tRect.top = tRect.bottom - height; 
		break;
	case WNDEFCT_HM_BOTTOM:
		steps = height/WNDEFCT_HS_STEPS;
		tRect.top += steps;
		if(tRect.top >= (GetSystemMetrics(SM_CYSCREEN) - m_edgeWidth))
		{
            tRect.top = GetSystemMetrics(SM_CYSCREEN) - m_edgeWidth;
			m_hsFinished = TRUE;
		}
		tRect.bottom = tRect.top + height;
		break;
	case WNDEFCT_HM_LEFT:
		steps = width/WNDEFCT_HS_STEPS;
		tRect.right -= steps;
		if(tRect.right <= m_edgeWidth)
		{
            tRect.right = m_edgeWidth;
			m_hsFinished = TRUE;
		}
		tRect.left = tRect.right - width;
		//tRect.top = -m_edgeHeight;
		//tRect.bottom = GetSystemMetrics(SM_CYSCREEN) - m_taskBarHeight;
		break;
	case WNDEFCT_HM_RIGHT:
		steps = width/WNDEFCT_HS_STEPS;
		tRect.left += steps;
		if(tRect.left >= (GetSystemMetrics(SM_CXSCREEN) - m_edgeWidth))
		{
            tRect.left = GetSystemMetrics(SM_CXSCREEN) - m_edgeWidth;
			m_hsFinished = TRUE;
		}		
		tRect.right = tRect.left + width;
		//tRect.top = -m_edgeHeight;
		//tRect.bottom = GetSystemMetrics(SM_CYSCREEN) - m_taskBarHeight;
		break;
	default:
		break;
	}

	MySetWindowPos(&tRect);
}

void CWndEffect::DoShow()
{
    if(m_hideMode == WNDEFCT_HM_NONE)
		return;
	
	RECT tRect;
	GetWindowRect(m_hWnd, &tRect);
	INT height = tRect.bottom-tRect.top;
	INT width  = tRect.right-tRect.left;

	INT steps = 0;

	switch(m_hideMode)
	{
	case WNDEFCT_HM_TOP:
		steps = height/WNDEFCT_HS_STEPS;
		tRect.top += steps;
		if(tRect.top >= -m_edgeHeight)
		{   //你可以把下面一句替换上面的 ...+=|-=steps 达到取消抽屉效果
			//更好的办法是添加个BOOL值来控制,其他case同样.
            tRect.top = -m_edgeHeight;
			m_hsFinished = TRUE;  //完成显示过程
		}
		tRect.bottom = tRect.top + height;
		break;
	case WNDEFCT_HM_BOTTOM:
		steps = height/WNDEFCT_HS_STEPS;
		tRect.top -= steps;
		if(tRect.top <= (GetSystemMetrics(SM_CYSCREEN) - height))
		{
            tRect.top = GetSystemMetrics(SM_CYSCREEN) - height;
			m_hsFinished = TRUE;
		}
        tRect.bottom = tRect.top + height;
		break;
	case WNDEFCT_HM_LEFT:
		steps = width/WNDEFCT_HS_STEPS;
		tRect.right += steps;
		if(tRect.right >= width)
		{
            tRect.right = width;
			m_hsFinished = TRUE;
		}
		tRect.left = tRect.right - width;
		//tRect.top = -m_edgeHeight;
		//tRect.bottom = GetSystemMetrics(SM_CYSCREEN) - m_taskBarHeight;
		break;
	case WNDEFCT_HM_RIGHT:
		steps = width/WNDEFCT_HS_STEPS;
		tRect.left -= steps;
		if(tRect.left <= (GetSystemMetrics(SM_CXSCREEN) - width))
		{
            tRect.left = GetSystemMetrics(SM_CXSCREEN) - width;
			m_hsFinished = TRUE;
		}
		tRect.right = tRect.left + width;
		//tRect.top = -m_edgeHeight;
		//tRect.bottom = GetSystemMetrics(SM_CYSCREEN) - m_taskBarHeight;
		break;
	default:
		break;
	}
	
	MySetWindowPos(&tRect);
}

BOOL CWndEffect::MySetWindowPos(LPCRECT pRect, UINT nFlags)
{
	return SetWindowPos(m_hWnd, NULL,pRect->left, pRect->top,0, 0, nFlags);
}

void CWndEffect::FixMoving(UINT_PTR fwSide, LPRECT pRect)
{
	POINT pTopLeft = {pRect->left, pRect->top};
	POINT pBottomRight = {pRect->right, pRect->bottom};
	//GetCursorPos(&curPos);
	INT screenHeight = GetSystemMetrics(SM_CYSCREEN);
	INT screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	INT height = pRect->bottom - pRect->top;
	INT width  = pRect->right - pRect->left;
    
	if (pTopLeft.y <= WNDEFCT_INTERVAL)
	{   //粘附在上边
		pRect->bottom = height - m_edgeHeight;
		pRect->top = -m_edgeHeight;
		m_hideMode = WNDEFCT_HM_TOP;
	}
	else if(pBottomRight.y >= (screenHeight - WNDEFCT_INTERVAL - m_taskBarHeight))
	{   //粘附在下边
        pRect->top = screenHeight - m_taskBarHeight - height;
        pRect->bottom = screenHeight - m_taskBarHeight;
		//m_hideMode = WNDEFCT_HM_BOTTOM;
		m_hideMode = WNDEFCT_HM_NONE; //下边不藏
	}
	else if (pTopLeft.x < WNDEFCT_INTERVAL)
	{	//粘附在左边	
		pRect->right = width;
		pRect->left = 0;
		m_hideMode = WNDEFCT_HM_LEFT;
	}
	else if(pBottomRight.x >= (screenWidth - WNDEFCT_INTERVAL))
	{   //粘附在右边
		pRect->left = screenWidth - width;
		pRect->right = screenWidth;
		m_hideMode = WNDEFCT_HM_RIGHT;
	}
	else
	{   //不粘附
		if(m_isSetTimer)
		{   //如果Timer开启了,则关闭之
			if(KillTimer(m_hWnd, 1) == 1)
				m_isSetTimer = FALSE;
		}
		m_hideMode = WNDEFCT_HM_NONE;
	}
}
/*
void CWndEffect::FixSizing(UINT fwSide, LPRECT pRect)
{

	if((pRect->right-pRect->left) < WNDEFCT_MINCX || (pRect->bottom-pRect->top) < WNDEFCT_MINCY)
	{   //小于指定的最小宽度或高度
		switch(fwSide)
		{
		case WMSZ_BOTTOM:
            pRect->bottom = pRect->top + WNDEFCT_MINCY;
			break;
		case WMSZ_BOTTOMLEFT:
			if((pRect->right-pRect->left) <= WNDEFCT_MINCX)
				pRect->left = pRect->right - WNDEFCT_MINCX;
			if((pRect->bottom-pRect->top) <= WNDEFCT_MINCY)
				pRect->bottom = pRect->top + WNDEFCT_MINCY;
			break;
		case WMSZ_BOTTOMRIGHT:
			if((pRect->right-pRect->left) < WNDEFCT_MINCX)
				pRect->right = pRect->left + WNDEFCT_MINCX;
			if((pRect->bottom-pRect->top) < WNDEFCT_MINCY)
				pRect->bottom = pRect->top + WNDEFCT_MINCY;
			break;
		case WMSZ_LEFT:
			pRect->left = pRect->right - WNDEFCT_MINCX;
			break;
		case WMSZ_RIGHT:
			pRect->right = pRect->left + WNDEFCT_MINCX;
			break;
		case WMSZ_TOP:
            pRect->top = pRect->bottom - WNDEFCT_MINCY;
			break;
		case WMSZ_TOPLEFT:
			if((pRect->right-pRect->left) <= WNDEFCT_MINCX)
				pRect->left = pRect->right - WNDEFCT_MINCX;
			if((pRect->bottom-pRect->top) <= WNDEFCT_MINCY)
				pRect->top = pRect->bottom - WNDEFCT_MINCY;
			break;
		case WMSZ_TOPRIGHT:
			if((pRect->right-pRect->left) < WNDEFCT_MINCX)
				pRect->right = pRect->left + WNDEFCT_MINCX;
			if((pRect->bottom-pRect->top) < WNDEFCT_MINCY)
				pRect->top = pRect->bottom - WNDEFCT_MINCY;
			break;
		}
	}
}
*/
