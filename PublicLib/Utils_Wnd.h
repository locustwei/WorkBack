#pragma once

#include "stdafx.h"
#include <commctrl.h>

int EnumChildWnds(HWND hWnd, HWND* wnds, int maxcount);
BOOL WndClassNameIs(HWND hwnd, LPCTSTR clsname);
BOOL TreeView_GetItemText(HWND hwnd, HTREEITEM hItem, LPTSTR text);
LPARAM TreeView_GetItemParam(HWND hwnd, HTREEITEM hItem);
BOOL HandleMessssage(HWND hwnd);
void WaitTimeNotBlock(int nMillisecond);
void SendClickMessage(HWND hwnd, BOOL bMoveCursor);