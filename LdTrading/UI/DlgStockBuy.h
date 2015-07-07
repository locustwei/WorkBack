#pragma once
#include "PriceComboBox.h"
#include "ConComboBox.h"
#include "UnitComboBox.h"
#include "..\..\LeadowUi\LdDialog.h"
#include "..\..\LeadowUi\LdButton.h"
#include "..\..\LeadowUi\LdEdit.h"

class CDlgStockBuy :public CLdDialog
{
public:
	CDlgStockBuy(void);
	~CDlgStockBuy(void);
protected:
	virtual BOOL OnInitDialog();

	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	virtual INT_PTR OnCommand( WORD ctrlid, HWND hwnd );

	virtual void InitDlgMembers( PDLGITEM_MAP maps );

private:
	CLdButton m_btnOk;
	CLdButton m_btnCancel;
	CLdEdit m_edValue1;
	CLdEdit m_edValue2;
	CPriceComboBox m_cbPrice1;
	CConComboBox m_cbCon1;
	CUnitComboBox m_cbUnit1;
	CUnitComboBox m_cbUnit2;
};

