#pragma once
#include "LdDialog.h"

class CDlgStrategy: public CLdDialog
{
	DECLARE_DYNAMIC(CDlgStrategy)
public:
	CDlgStrategy();
	~CDlgStrategy();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
private:
	CEdit m_edName;
	afx_msg void OnClose();

	virtual BOOL OnInitDialog();

};