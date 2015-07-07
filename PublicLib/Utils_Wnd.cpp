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