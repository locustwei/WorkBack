#include "JylsForm.h"
#include <CommCtrl.h>

CJylsForm* CJylsForm::theForm = NULL;

CJylsForm::CJylsForm(void)
{
	theForm = this;
}


CJylsForm::~CJylsForm(void)
{
	theForm = NULL;
}

BOOL CJylsForm::OnInitForm()
{
	m_hList = GetDlgItem(m_hWnd, IDC_LIST_RECORD);
	InitListColumn();
	return TRUE;
}

void CJylsForm::OnCommand( WORD c_id )
{
	
}

void CJylsForm::InitListColumn()
{
	/*
	ctl_anchors = new CONTRL_ANCHORS[4];
	ctl_anchors[0].anchors = akLeft | akTop | akRight | akBottom;
	ctl_anchors[0].DLGITEM_ID = IDC_LIST_RECORD;

	ctl_anchors[1].anchors = akRight | akBottom;
	ctl_anchors[1].DLGITEM_ID = IDCANCEL;

	ctl_anchors[2].anchors = akRight | akBottom;
	ctl_anchors[2].DLGITEM_ID = IDOK;

	ctl_anchors[3].DLGITEM_ID = 0xFFFFFFFF;
	*/
	LVCOLUMN column = {0};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.cx = 150;
	column.fmt = LVCFMT_RIGHT;
	column.pszText = L"市值";
	column.cchTextMax = 255;
	ListView_InsertColumn(m_hList, 0, &column);

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.cx = 100;
	column.fmt = LVCFMT_RIGHT;
	column.pszText = L"持仓成本";
	column.cchTextMax = 255;
	ListView_InsertColumn(m_hList, 0, &column);

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.cx = 100;
	column.fmt = LVCFMT_RIGHT;
	column.pszText = L"买入价";
	column.cchTextMax = 255;
	ListView_InsertColumn(m_hList, 0, &column);

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.cx = 120;
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"股票名称";
	column.cchTextMax = 255;
	ListView_InsertColumn(m_hList, 0, &column);

	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.cx = 100;
	column.fmt = LVCFMT_LEFT;
	column.pszText = L"股票代码";
	column.cchTextMax = 255;
	ListView_InsertColumn(m_hList, 0, &column);


}
