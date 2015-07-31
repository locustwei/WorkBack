#pragma once
#include "..\..\PublicLib\hooks\WindowHook.h"
#include "..\..\PublicLib\comps\LdList.h"
#include "..\TDXDataStruct.h"
#include <commctrl.h>
#include "TDXStockBuy.h"
#include "TDXStockSell.h"
#include "..\..\StockDataAPI\IDataInterface.h"
#include "TDXGfcx.h"


class CTDXMain :public CWndHook
{
	friend class CRightPanelHook;
public:
	CTDXMain(HWND hWnd);
	~CTDXMain(void);

	static CTDXMain* WndHooker;
	static void HookMainWnd(HWND hwnd);
	BOOL DoStockBy(STOCK_MARK mark, LPCSTR szSymbol, float fPrice, DWORD dwVolume);
	BOOL DoStockSell(STOCK_MARK mark, LPCSTR szSymbol, float fPrice, DWORD dwVolume);
	BOOL DoStockZjgf();
protected:
	virtual LRESULT WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
private:
	HWND m_hLeftPanel;
	HWND m_hRightPanel;
	CRightPanelHook* m_RightHook;         //右侧操作窗口面板Hook拦截操作窗口创建消息

	BOOL FindNavTreeViews();             //找到导航菜单TreeView；
	BOOL InitRightWnds();                //在操作窗口面板设置Hook，
	void FindChildWnds();                //找到导航菜单栏、操作窗口面板
	CLdList<Tdx_Tree_HWND> m_NavTrees;   //功能菜单树TreeView数组（视版本不同可能包含：股票功能树、基金功能树、ETF功能树、港股通功能树等）
	CLdList<Tdx_TreeItem_Dlg> m_NavDialogs;                                      //功能菜单项操作窗口Dialog数组（如：股票买入窗口、股票卖出窗口等）

	BOOL Click_NavTreeItem( HWND hwnd, HTREEITEM item );                        //模拟用户点击功能菜单树上的一个节点。
	HWND Find_Dialog( LPCSTR szID, LPCSTR szGroup = NULL);                      //根据szID找到操作窗口
	HTREEITEM Find_Nav_Item( HWND& hwnd, LPCSTR szID, LPCSTR szGroup = NULL );  //根据szID（TcOem.xml中定义）找到TreeView中的Item。
	HTREEITEM FindTreeItemByParam(HWND hwnd,  LPARAM param, HTREEITEM hItem = NULL);
	BOOL Click_TreeItemByID(LPCSTR szID, LPCSTR szGroup = NULL);

	CTDXStockBuy* m_StockBuyDlg;
	CTDXStockSell* m_StockSellDlg;
	CTDXZjgf* m_StockZjgfDlg;
	CTDXStockBuy* GetStockBuyDlg();                                            //查找股票买入窗口
	CTDXStockSell* GetStockSellDlg();
	CTDXZjgf* GetStockZjgfDlg();
};

