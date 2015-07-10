#pragma once

#include "..\..\PublicLib\hooks\WindowHook.h"

class CTDXStockSell :public CWndHook
{
public:
	CTDXStockSell(HWND hWnd);
	~CTDXStockSell(void);

	BOOL DoSell(LPCSTR szCode, float fPrice);
protected:
	virtual LRESULT WndPROC( HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam );
private:
	HWND m_edCode;
	HWND m_edValue;
	HWND m_edCount;
	HWND m_btnOk;  
	HWND m_lbJe;   
	HWND m_lbZdsl; 
	HWND m_lstGp;  
	HWND m_cbBjfs; 
	HWND m_btnAll; 

};

