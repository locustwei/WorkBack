#pragma once
#include "..\stdafx.h"
#include "..\..\PublicLib\hooks\WindowHook.h"
#include "..\..\StockDataAPI\IDataInterface.h"

class CTDXStockSell :public CWndHook
{
	friend void SellContolInViceThread(LPARAM lparam);
public:
	CTDXStockSell(HWND hWnd);
	~CTDXStockSell(void);

	BOOL DoSell(STOCK_MARK mark, LPCSTR szSymbol, float fPrice, DWORD dwVolume);
protected:
	virtual LRESULT WndPROC( HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam );
	void SetControlsText();
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

	STOCK_MARK m_Mark;
	char m_Code[7];
	float m_Price;
	DWORD m_Volume;
	DWORD m_HtId; //下单后生成的合同号
	BOOL m_btnClicked;
};

