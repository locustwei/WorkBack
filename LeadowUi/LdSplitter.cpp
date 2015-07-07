#include "LdSplitter.h"


CLdSplitter::CLdSplitter(void)
{
	m_ClassName = L"STATIC";
	m_Style = WS_CHILD|WS_VISIBLE|WS_GROUP;
	m_ExStyle = WS_EX_TRANSPARENT;

	m_dwSplitterStyle   = SPS_VERTICAL;
	m_bMouseIsDown      = FALSE;
	ZeroMemory(&m_ptCurrent, sizeof(m_ptCurrent));
	ZeroMemory(&m_ptOrigin, sizeof(m_ptOrigin));
	m_hCursor           = NULL;
	m_iMinPos           = 0;
	m_iMaxPos           = 0;
	m_clrSplitterColor  = RGB(120, 120, 120);
	m_lSplitterWidth    = 1;
}


CLdSplitter::~CLdSplitter(void)
{
}

void CLdSplitter::Create(HWND pParent, DWORD dwStyle, DWORD dwColor)
{
	m_dwSplitterStyle = dwStyle;

	//  Sace splitter color
	m_clrSplitterColor = dwColor;

	//  load the cursor
	m_hCursor = ::LoadCursor(NULL, m_dwSplitterStyle&SPS_VERTICAL?IDC_SIZEWE:IDC_SIZENS);

	return CLdWnd::Create(pParent);
}

void CLdSplitter::AttchWindow(HWND hwnd)
{
	return CLdWnd::AttchWindow(hwnd);
}

LRESULT CLdSplitter::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		OnLButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		OnLButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	}
	return CLdWnd::WindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL CLdSplitter::OnInitWnd()
{
	return CLdWnd::OnInitWnd();
}

void CLdSplitter::OnMouseMove(UINT nFlags, WORD x, WORD y)
{
	if (m_bMouseIsDown)
	{
		//  erase the old splitter
		HDC dc = GetWindowDC(0);
		this->DrawSplitterLine(dc);

		//  calc the new pos of the splitter
		POINT pt = {x, y};
		ClientToScreen(m_hWnd, &pt);

		HWND hParent = GetParent(m_hWnd);
		if(hParent==NULL || !IsWindow(hParent))
			return;
		ScreenToClient(hParent, &pt);

		//  split position limit
		pt.x = (pt.x < m_iMinPos)?m_iMinPos:pt.x;
		pt.y = (pt.y < m_iMinPos)?m_iMinPos:pt.y;
		pt.x = (pt.x > m_iMaxPos)?m_iMaxPos:pt.x;
		pt.y = (pt.y > m_iMaxPos)?m_iMaxPos:pt.y;

		//  save the current pos, this value will be used when the mouse up
		ClientToScreen(hParent, &pt);
		m_ptCurrent = pt;

		//  repaint the splitter
		DrawSplitterLine(dc);

		ReleaseDC(0, dc);
	}
}

void CLdSplitter::DrawSplitterLine(HDC pDC)
{
	POINT      pt          = this->m_ptCurrent;
	COLORREF    clrSplitter = this->m_clrSplitterColor;


	int nRop = SetROP2(pDC, R2_NOTXORPEN);

	RECT rcWnd;
	GetWindowRect(m_hWnd, &rcWnd);

	HPEN  pen = CreatePen(0, 1, clrSplitter);
	HPEN pOldPen = (HPEN)SelectObject(pDC, pen);

	POINT outPt;
	if (m_dwSplitterStyle&SPS_VERTICAL)
	{
		MoveToEx(pDC, pt.x - m_lSplitterWidth, rcWnd.top, &outPt);
		LineTo(pDC, pt.x - m_lSplitterWidth, rcWnd.bottom);

		MoveToEx(pDC, pt.x + m_lSplitterWidth, rcWnd.top, &outPt);
		LineTo(pDC, pt.x + m_lSplitterWidth, rcWnd.bottom);
	}
	else // m_dwSplitterStyle == SPS_HORIZONTAL
	{
		MoveToEx(pDC, rcWnd.left,  pt.y - m_lSplitterWidth, &outPt);
		LineTo(pDC, rcWnd.right, pt.y - m_lSplitterWidth);

		MoveToEx(pDC, rcWnd.left,  pt.y + m_lSplitterWidth, &outPt);
		LineTo(pDC, rcWnd.right, pt.y + m_lSplitterWidth);
	}

	SetROP2(pDC, nRop);
	SelectObject(pDC, pOldPen);
}

void CLdSplitter::OnLButtonDown(UINT nFlags, WORD x, WORD y)
{
	GetMaxMinPos(this->m_iMaxPos, this->m_iMinPos);

	//  Record the mouse status
	m_bMouseIsDown = TRUE;

	SetCapture(m_hWnd);


	//  Get the move split start pos
	RECT rcWnd;
	GetWindowRect(m_hWnd, &rcWnd);
	m_ptOrigin.x = (rcWnd.right+rcWnd.left)/2;
	m_ptOrigin.y = (rcWnd.bottom+rcWnd.top)/2;

	m_ptCurrent = m_ptOrigin;

	//  draw the splitter
	HDC dc = GetWindowDC(NULL);
	DrawSplitterLine(dc);
	ReleaseDC(NULL, dc);
}

void CLdSplitter::GetMaxMinPos(int& iMax, int& iMin)
{
	HWND hParent = GetParent(m_hWnd);
	if(!hParent || !IsWindow(hParent))
		return;

	RECT rcParent;
	GetClientRect(hParent, &rcParent);

	//  try to get the max/min pos limit from parent window
	SPC_NM_MAXMINPOS  nmMinmax;
	nmMinmax.hdr.hwndFrom   = m_hWnd;
	nmMinmax.hdr.idFrom     = GetDlgCtrlID(m_hWnd);
	nmMinmax.hdr.code       = SPN_MAXMINPOS;
	nmMinmax.lMax           = (m_dwSplitterStyle&SPS_VERTICAL)?rcParent.right:rcParent.bottom;
	nmMinmax.lMin           = (m_dwSplitterStyle&SPS_VERTICAL)?rcParent.left:rcParent.top;
	SendMessage(hParent, WM_NOTIFY, nmMinmax.hdr.idFrom, (LPARAM)&nmMinmax);

	//  return
	iMax = nmMinmax.lMax;
	iMin = nmMinmax.lMin;
}

void CLdSplitter::OnLButtonUp(UINT nFlags, WORD x, WORD y)
{
	
	if (m_bMouseIsDown)
	{
		//  erase the old picture
		POINT point = {x, y};
		ClientToScreen(m_hWnd, &point);
		HDC dc = GetWindowDC(NULL);
		DrawSplitterLine(dc);
		ReleaseDC(NULL, dc);
		//  move spliter control self to the new pos
		m_bMouseIsDown = FALSE;
		HWND hParent = GetParent(m_hWnd);
		if (hParent && IsWindow(hParent))
		{
			POINT pt = m_ptCurrent;
			ScreenToClient(m_hWnd, &pt);
			MoveSelfWindowToPoint(pt);

		}

		//  relayout all registerd windows
		Relayout();

		//  if need notify the parent
		if (m_dwSplitterStyle & SPS_DELTA_NOTIFY)
		{
			if (hParent && IsWindow(hParent))
			{
				POINT ptDelta = {m_ptCurrent.x - m_ptOrigin.x, m_ptCurrent.y - m_ptOrigin.y};
				SPC_NM_DELTA tNotify;
				tNotify.hdr.hwndFrom = m_hWnd;
				tNotify.hdr.idFrom   = GetDlgCtrlID(m_hWnd);
				tNotify.hdr.code     = SPN_DELTA;
				tNotify.lDelta       = (m_dwSplitterStyle&SPS_VERTICAL)?ptDelta.x:ptDelta.y;
				SendMessage(hParent, WM_NOTIFY, tNotify.hdr.idFrom, (LPARAM)&tNotify);
			}
		}
	}

	ReleaseCapture();
}

void CLdSplitter::MoveSelfWindowToPoint(POINT pt)
{
	HWND hParent = GetParent(m_hWnd);
	if (!hParent || !IsWindow(hParent))
	{
		return;
	}

	RECT rc;
	GetWindowRect(m_hWnd, &rc);
	MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);

	//  calc the new rect
	if (m_dwSplitterStyle&SPS_VERTICAL)
	{
		OffsetRect(&rc, (pt.x - (rc.left + rc.right) / 2), 0);
	}
	else
	{
		OffsetRect(&rc, 0, (pt.y - (rc.top + rc.bottom) / 2));
	}

	MoveWindow(rc);
}

void CLdSplitter::Relayout()
{
	HWND hParent = GetParent(m_hWnd);
	if(!hParent || IsWindow(hParent))
		return;

	RECT rcSelf;
	GetWindowRect(m_hWnd, &rcSelf);
	/*
	for (int i = 0; i < 2; i++)
	{
		for (POSITION pos = m_listLinkedWnds[i].GetHeadPosition(); NULL != pos;)
		{
			//  get the window object
			HWND hWnd = this->m_listLinkedWnds[i].GetNext(pos);
			if (NULL == hWnd)
			{
				continue;
			}

			//  calc the new pos(the code is not very good)
			RECT rcLinked;
			GetWindowRect(hWnd, &rcLinked);
			if (SPS_VERTICAL&m_dwSplitterStyle)
			{
				if (SPLS_LINKED_LEFT == i)
				{
					rcLinked.right  = rcSelf.left;
				}
				else
				{
					rcLinked.left   = rcSelf.right;
				}
			}
			else
			{
				if (SPLS_LINKED_LEFT == i)
				{
					rcLinked.bottom = rcSelf.top;
				}
				else
				{
					rcLinked.top    = rcSelf.bottom;
				}
			}

			//  move it to new pos and then update
			MapWindowPoints(NULL, hParentm, &rcLinked, 2);
			MoveWindow(rcLinked, TRUE);
			Invalidate();
		}
	}
	*/
}
