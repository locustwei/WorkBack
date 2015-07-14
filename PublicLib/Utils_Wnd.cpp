#include "stdafx.h"
#include "Utils_Wnd.h"


struct FOR_EnumWnd{
	HWND* p;
	int maxcount;
	int count;
};

BOOL CALLBACK EnumChildProc(
	_In_ HWND   hwnd,
	_In_ LPARAM lParam
	)
{
	FOR_EnumWnd* result = (FOR_EnumWnd*)lParam;
	//result->p = (HWND*)realloc(result->p, ++result->count);
	//if(result->p==NULL)
	//return FALSE;
	result->p[result->count++] = hwnd;
	return result->maxcount==0 || result->count < result->maxcount;
}

int EnumChildWnds(HWND hWnd, HWND* wnds, int maxcount)
{
	FOR_EnumWnd result = {0};
	result.p = wnds;
	result.maxcount = maxcount;
	EnumChildWindows(hWnd, &EnumChildProc, (LPARAM)&result);
	return result.count;
}

BOOL TreeView_GetItemText(HWND hwnd, HTREEITEM hItem, LPTSTR text)
{
	
	TV_ITEM tvi = {0};

	tvi.mask = TVIF_TEXT | TVIF_HANDLE;
	tvi.hItem=hItem;
	tvi.pszText = text;
	tvi.cchTextMax = 255;
	return TreeView_GetItem(hwnd, &tvi);
}

LPARAM TreeView_GetItemParam(HWND hwnd, HTREEITEM hItem)
{
	TV_ITEM tvi = {0};

	tvi.mask = TVIF_PARAM | TVIF_HANDLE;
	tvi.hItem=hItem;
	if(TreeView_GetItem(hwnd, &tvi))
		return tvi.lParam;
	else
		return -1;
}

BOOL WndClassNameIs(HWND hwnd, LPCTSTR clsname)
{
	WCHAR name[MAX_LEN] = {0}; 
	INT len = GetClassName(hwnd, name, MAX_LEN);
	if(len>0){
		return wcscmp(clsname, name)==0;
	}
	return FALSE;
}

BOOL HandleMessssage(HWND hwnd)
{
	MSG msg = {0};
	BOOL MsgExists = PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE);
	if(MsgExists){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return MsgExists;
		/*
	BOOL Unicode;
	MSG Msg = {0};
	BOOL MsgExists = PeekMessage(&Msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE);

	if (MsgExists || PeekMessage(&Msg, 0, 0, 0, PM_NOREMOVE)){
		Unicode = (Msg.hwnd = 0) || IsWindowUnicode(Msg.hwnd);
		if(!MsgExists){
			if (Unicode)
				MsgExists = PeekMessageW(&Msg, 0, 0, 0, PM_REMOVE);
			else
				MsgExists = PeekMessageA(&Msg, 0, 0, 0, PM_REMOVE);
		}

		if (MsgExists){
			TranslateMessage(&Msg);
			if (Unicode)
				DispatchMessageW(&Msg);
			else
				DispatchMessageA(&Msg);
		}
	}
	*/
}

void WaitTimeNotBlock(int nMillisecond)
{
	DWORD dwTime = GetTickCount();
	while(GetTickCount()-dwTime<nMillisecond)
		HandleMessssage(NULL);
}

void SendClickMessage(HWND hwnd, BOOL bMoveCursor)
{
	RECT r = {0};
	GetWindowRect(hwnd, &r);
	POINT p ={(r.left+r.right)/2, (r.top+r.bottom)/2};
	
	if(bMoveCursor){
		SetCursorPos(p.x, p.y);
	}

	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(p.x, p.y));
	WaitTimeNotBlock(100); //需要有时间间隔
	PostMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(p.x, p.y));
}