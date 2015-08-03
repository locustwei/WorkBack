
#include "stdafx.h"
#include "DlgStrategy.h"

CDlgStrategy::CDlgStrategy():CLdDialog(IDD_DLG_STRATEGY)
{
	m_pStrategy = NULL;
}

CDlgStrategy::~CDlgStrategy()
{

}

void CDlgStrategy::DoDataExchange(CDataExchange* pDX)
{
	CLdDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_edName);
	DDX_Control(pDX, IDC_EDIT_DESC, m_edDesc);
	DDX_Control(pDX, IDC_BTN_OK, m_btnOk);
	DDX_Control(pDX, IDC_PANEL_PARAM, m_pnlParam);
}

void CDlgStrategy::OnClose()
{

}

BEGIN_MESSAGE_MAP(CDlgStrategy, CLdDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CDlgStrategy::OnInitDialog()
{
	CLdDialog::OnInitDialog();

	if(m_pStrategy)
		CreateControls();
	return TRUE;
}

void CDlgStrategy::SetStrategy( PSTRATEGY_STRCPIT pStrategy )
{
	if(pStrategy!=m_pStrategy){
		m_pStrategy = pStrategy;
		if(GetSafeHwnd()!=NULL)
			CreateControls();
	}
}

void CDlgStrategy::CreateControls()
{
	ClearWindows();
	if(m_pStrategy==NULL)
		return;

	::SetWindowTextA(m_edName.GetSafeHwnd(), m_pStrategy->szName);
	::SetWindowTextA(m_edDesc.GetSafeHwnd(), m_pStrategy->szComment);
	int top = 10;
	for(int i=0; i<m_pStrategy->nParamCount; i++){
		CStatic* name = new CStatic();
		name->Create(CString(m_pStrategy->szParams[i]), WS_CHILD|WS_VISIBLE|WS_GROUP, CRect(10, top, 100, top + 18), &m_pnlParam);
		top += 30;
	}
}

void CDlgStrategy::ClearWindows()
{
	m_edName.SetWindowText(NULL);
	m_edDesc.SetWindowText(NULL);
}
