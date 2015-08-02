#include "stdafx.h"
#include "LdDialog.h"

IMPLEMENT_DYNAMIC( CLdDialog, CDialogEx )

CLdDialog::CLdDialog()
{

}

CLdDialog::CLdDialog( UINT nIDTemplate, CWnd *pParent /*= NULL*/ )
{
	CDialogEx::CDialogEx(nIDTemplate, pParent);
}

CLdDialog::CLdDialog( LPCTSTR lpszTemplateName, CWnd *pParentWnd /*= NULL*/ )
{
	CDialogEx(lpszTemplateName, pParentWnd);
}

CLdDialog::~CLdDialog()
{

}

int CLdDialog::ShowDialog()
{
	return 0;
}

int CLdDialog::DoModal()
{
	return 0;
}

BOOL CLdDialog::OnInitDialog()
{
	return CDialogEx::OnInitDialog();
}

BOOL CLdDialog::Create( CWnd* pParentWnd /*= NULL */ )
{
	return CDialogEx::Create(IDD, pParentWnd);
}
