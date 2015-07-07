#pragma once
#include "ldwnd.h"

/////////////////////////////////////////////////////////////////////////////
// Splitter style
#define SPS_HORIZONTAL      0x00000000          //  Horizontal look splitter
#define SPS_VERTICAL        0x00000001          //  Vertical look splitter(this is the default style)
#define SPS_DELTA_NOTIFY    0x00000002          //  if need notify the parent to handle delta message


//  Linked side(used for Vertical style)
#define SPLS_LINKED_RIGHT   0x00000000          //  linked window at the right of the splitter(right pos change automatic)
#define SPLS_LINKED_LEFT    0x00000001          //  linked window at the left of the splitter(right pos change automatic)


//  Linked side(used for Horizontal style)
#define SPLS_LINKED_UP      SPLS_LINKED_LEFT    //  linked window at the top of the splitter(right pos change automatic)
#define SPLS_LINKED_DOWN    SPLS_LINKED_RIGHT   //  linked window at the bottom of the splitter(right pos change automatic)

//  Notify event : to get the max/min pos limit
//      usualy, the max/min pos depend on the parent window size, so we'd better get it from parent.
#define SPN_MAXMINPOS       (WM_USER + 1)
struct SPC_NM_MAXMINPOS
{
	NMHDR   hdr;
	LONG    lMax;
	LONG    lMin;
};


//  Notify event : tell the parent to do some special things
//      some times, the parent window can not register the child control for reason it does not created yet.
//      so, SPN_DELTA event give the parent window a chance to change the child control's pos.
#define SPN_DELTA           (WM_USER + 267)
struct SPC_NM_DELTA
{
	NMHDR   hdr;
	LONG    lDelta;
};

class CLdSplitter :public CLdWnd
{
public:
	CLdSplitter(void);
	~CLdSplitter(void);

	void Create(HWND pParent, DWORD dwStyle, DWORD dwColor);

	virtual void AttchWindow(HWND hwnd);

protected:
	DWORD           m_dwSplitterStyle;      //  Splitter Style
	BOOL            m_bMouseIsDown;         //  Record the mouse is down or up
	POINT           m_ptCurrent;            //  Current positon
	POINT           m_ptOrigin;             //  Positon when mouse down
	HCURSOR         m_hCursor;              //  Cursor when mouse move
	COLORREF        m_clrSplitterColor;     //  Color of splitter
	LONG            m_lSplitterWidth;       //  line width of splitter
	int             m_iMaxPos;              //  Max pos the splitter
	int             m_iMinPos;              //  Min pos the splitter

	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitWnd();
	void OnMouseMove(UINT nFlags, WORD x, WORD y);
	void DrawSplitterLine(HDC pDC);
	void OnLButtonDown(UINT nFlags, WORD x, WORD y);
	void GetMaxMinPos(int& iMax, int& iMin);
	void OnLButtonUp(UINT nFlags, WORD x, WORD y);
	void MoveSelfWindowToPoint(POINT pt);
	void Relayout();
};

