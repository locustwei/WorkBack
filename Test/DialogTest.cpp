#include "StdAfx.h"
#include "DialogTest.h"
#include "..\PublicLib\LdXml.h"
#include "..\TdxPlugin\TDXDataStruct.h"
#include "..\PublicLib\Utils_Wnd.h"
#include "..\StockDataAPI\HttpStockData.h"


CDialogTest::CDialogTest(void)
{
	m_btnOk.m_Anchors = akBottom | akRight;
	m_btnCancel.m_Anchors = akBottom | akRight;
}


CDialogTest::~CDialogTest(void)
{
}

CLdTreeItem* item1;
CLdTreeItem* item2;
CLdTreeItem* item3;
CLdTreeItem* item4;
CLdTreeItem* item5;
CLdTreeItem* item6;
CLdTreeItem* item7;

BOOL CDialogTest::OnInitDialog()
{
	
	CLdDialog::OnInitDialog();
	int nCol = 0;
	nCol = m_lvMyStock.InsertColumn(nCol, L"股票代码", LVCFMT_LEFT, 50);
	nCol = m_lvMyStock.InsertColumn(nCol+1, L"股票名称", LVCFMT_LEFT, 150);

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");
	
	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	item1 = m_tree.InsetItem(NULL, NULL, L"tree1");
	item2 = m_tree.InsetItem(NULL, NULL, L"tree2");
	item3 = m_tree.InsetItem(NULL, NULL, L"tree3");
	item4 = m_tree.InsetItem(NULL, NULL, L"tree4");
	item5 = m_tree.InsetItem(item4, NULL, L"tree5");
	item6 = m_tree.InsetItem(item4, NULL, L"tree6");
	item7 = m_tree.InsetItem(item4, NULL, L"tree7");

	HINSTANCE hInstance = LoadLibrary(L"..\\通用版通达信独立交易\\TCPlugins\\TdxPlugin.dll");
	typedef int (* Addin_)();
	Addin_ p = (Addin_)GetProcAddress(hInstance, "PlugTest");
	p();

	return TRUE;
}

INT_PTR CDialogTest::OnCommand(WORD ctrlid, HWND hwnd1)
{
	CHttpStockData HttpStockData ;
	PSTOCK_DATA_PK pk[2];
	pk[0] = new STOCK_DATA_PK;
	pk[0]->mark = MARK_SH;
	pk[0]->szCode = "601006";

	pk[1] = new STOCK_DATA_PK;
	pk[1]->mark = MARK_SZ;
	pk[1]->szCode = "000858";
	HttpStockData.GetStockPK(pk, 2);
	return TRUE;
}

LRESULT CDialogTest::WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if(uMsg==WM_CLOSE)
		PostQuitMessage(0);
	return CLdDialog::WindowProc(hwnd, uMsg, wParam, lParam);
}

void CDialogTest::InitDlgMembers( PDLGITEM_MAP maps )
{
	DLGITEM_MAP a[] = {
		{IDOK, &m_btnOk},
		{IDC_LIST_MY, &m_lvMyStock},
		{IDC_TREE1,&m_tree},
		{-1}
	};
	CLdDialog::InitDlgMembers(a);
}
