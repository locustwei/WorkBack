/************************************************************************/
/* 交易历史数据
*/
/************************************************************************/
#pragma once
#include "..\stdafx.h"
#include "..\..\LeadowUi\LdDialog.h"

class CJylsForm :public CLdDialog
{
public:
	CJylsForm(void);
	~CJylsForm(void);

	static CJylsForm* theForm;
protected:
	virtual BOOL OnInitForm();

	virtual void OnCommand( WORD c_id );
	void InitListColumn();
private:
	HWND m_hList;
};

