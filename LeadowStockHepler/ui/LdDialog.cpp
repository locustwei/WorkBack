#include "stdafx.h"
#include "LdDialog.h"


CLdDialog::CLdDialog():CDialogEx()
{
}

CLdDialog::CLdDialog( UINT nIDTemplate, CWnd *pParent /*= NULL*/ ):CDialogEx(nIDTemplate, pParent)
{

}

CLdDialog::~CLdDialog()
{

}

BOOL CLdDialog::Create( CWnd* pParentWnd /*= NULL */ )
{
	return CDialogEx::Create(m_nIDHelp, pParentWnd);
}

void CLdDialog::OnOK()
{
	
}

void CLdDialog::OnCancel()
{
	
}

