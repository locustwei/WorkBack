#pragma once
#include "LdDialog.h"
#include "..\ScriptEng\ScriptEng.h"

class CDlgStrategy: public CLdDialog
{
public:
	CDlgStrategy();
	~CDlgStrategy();

public:
	void SetStrategy( PSTRATEGY_STRCPIT pStrategy );
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
private:
	PSTRATEGY_STRCPIT m_pStrategy;
	
	CEdit m_edName;
	CEdit m_edDesc;
	CMFCButton m_btnOk;
	CStatic m_pnlParam;
	afx_msg void OnClose();

	virtual BOOL OnInitDialog();
	void CreateControls();
	void ClearWindows();
};