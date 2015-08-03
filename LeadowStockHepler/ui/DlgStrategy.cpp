
#include "stdafx.h"
#include "DlgStrategy.h"

IMPLEMENT_DYNAMIC( CDlgStrategy, CLdDialog )

CDlgStrategy::CDlgStrategy()
{
	m_nResId = IDD_DLG_STRATEGY;
}

CDlgStrategy::~CDlgStrategy()
{

}

void CDlgStrategy::DoDataExchange(CDataExchange* pDX)
{
	CLdDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_edName);
}

void CDlgStrategy::OnClose()
{

}

BEGIN_MESSAGE_MAP(CDlgStrategy, CLdDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CDlgStrategy::OnInitDialog()
{
	return CLdDialog::OnInitDialog();
}
