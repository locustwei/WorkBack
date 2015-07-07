#pragma once
#include "..\script\ScriptEng.h"
#include "..\..\LeadowUi\LdButton.h"
#include "..\..\LeadowUi\LdEdit.h"
#include "..\..\LeadowUi\LdDialog.h"
#include "..\..\StockDataAPI\IDataInterface.h"
#include "..\ITradInterface.h"
#include "..\..\PublicLib\comps\NotifyIcon.h"

class CMainDlg :public CLdDialog
{
public:
	CMainDlg(void);
	~CMainDlg(void);

protected:
	virtual LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnInitDialog();

	virtual INT_PTR OnCommand(WORD ctrlid, HWND hwnd);

	virtual void InitDlgMembers(PDLGITEM_MAP maps);

private:
	CNotifyIcon* m_Notifyicon;
	CScriptEng* m_ScriptEng;
	CLdButton m_btnOk;
	CLdButton m_btn;
	CLdEdit m_ed1;
	CLdEdit m_ed2;
	IDataInterface* m_DateInterface;
	ITradInterface* m_TradInterface;
};

