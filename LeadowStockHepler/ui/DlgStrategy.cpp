
#include "stdafx.h"
#include "DlgStrategy.h"
#include "..\LeadowStockHepler.h"

CDlgStrategy::CDlgStrategy():CLdDialog(IDD_DLG_STRATEGY)
{
	m_pStrategy = NULL;
}

CDlgStrategy::~CDlgStrategy()
{
	POSITION p = m_ParamWnds.GetStartPosition();
	while(p){
		CInputParamWnd* wnd;
		LPCSTR sz;
		m_ParamWnds.GetNextAssoc(p, sz, wnd);
		delete wnd;
	}
	m_ParamWnds.RemoveAll();
}

void CDlgStrategy::DoDataExchange(CDataExchange* pDX)
{
	CLdDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NAME, m_edName);
	DDX_Control(pDX, IDC_EDIT_DESC, m_edDesc);
	DDX_Control(pDX, IDC_BTN_USE, m_btnOk);
	DDX_Control(pDX, IDC_PANEL_PARAM, m_pnlParam);
}

void CDlgStrategy::OnClose()
{

}

BEGIN_MESSAGE_MAP(CDlgStrategy, CLdDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_TEST, OnBtnTest)
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
	int top = 10, width=0;

	//计算长度
	CDC* dc = GetDC();
	for(int i=0; i<m_pStrategy->nParamCount; i++){
		CSize size;
		GetTextExtentPoint32A(dc->m_hDC, m_pStrategy->szParamComments[i], strlen(m_pStrategy->szParamComments[i]), &size);
		if(size.cx>width)
			width = size.cx;
	}
	ReleaseDC(dc);

	for(int i=0; i<m_pStrategy->nParamCount; i++){
		CInputParamWnd* wnd = new CInputParamWnd();
		wnd->Create(&m_pnlParam, 10, top, width, m_pStrategy->szParamComments[i]);
		m_ParamWnds.SetAt(m_pStrategy->szParams[i], wnd);
		top += 30;
	}
}

void CDlgStrategy::ClearWindows()
{
	m_edName.SetWindowText(NULL);
	m_edDesc.SetWindowText(NULL);
}

void CDlgStrategy::OnBtnTest()
{
	for(int i=0; i<m_pStrategy->nParamCount; i++){
		CString s = GetInputParamValue(m_pStrategy->szParams[i]);
		if(&s==NULL || s.IsEmpty()){
			s.Format(L"请输入参数：【%s】", CString(m_pStrategy->szParamComments[i]));
			AfxMessageBox(s);
			return;
		}
	}
	theApp.m_ScriptEng->TestStrategy(m_pStrategy, CString(""), CString(""));
}

CString CDlgStrategy::GetInputParamValue( LPSTR szParams )
{
	CInputParamWnd* wnd;
	if(!m_ParamWnds.Lookup(szParams, wnd))
		return NULL;
	return wnd->GetValue();
}
