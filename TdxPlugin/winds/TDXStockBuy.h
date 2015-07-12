/************************************************************************/
/* ¹ÉÆ±ÂòÈë´°¿Ú
*/
/************************************************************************/
#pragma once
#include "..\..\PublicLib\hooks\WindowHook.h"
#include "..\..\StockDataAPI\IDataInterface.h"

class CTDXStockBuy :public CWndHook
{
public:
	CTDXStockBuy(HWND hWnd);
	~CTDXStockBuy(void);

	BOOL DoBy( STOCK_MARK mark, LPCSTR szCode, float fPrice, DWORD dwVolume );
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
	BOOL m_Inited;
};

