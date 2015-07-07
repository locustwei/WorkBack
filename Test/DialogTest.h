#pragma once
#include "..\LeadowUi\LdDialog.h"
#include "..\LeadowUi\LdButton.h"
#include "..\LeadowUi\LdListView.h"
#include "..\LeadowUi\LdTreeCtrl.h"

class CDialogTest : public CLdDialog
{
public:
	CDialogTest(void);
	~CDialogTest(void);

private:
	virtual BOOL OnInitDialog();

	virtual INT_PTR OnCommand(WORD ctrlid, HWND hwnd1);

	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	virtual void InitDlgMembers( PDLGITEM_MAP maps );

	CLdButton m_btnOk;
	CLdButton m_btnCancel;
	CLdListCtrl m_lvMyStock;
	CLdTreeCtrl m_tree;
};

