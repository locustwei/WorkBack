#include "stdafx.h"
#include "LdDialog.h"

IMPLEMENT_DYNAMIC( CLdDialog, CDialogEx )

CLdDialog::CLdDialog():CDialogEx()
{
	m_nResId = 0;
}

CLdDialog::~CLdDialog()
{

}

BOOL CLdDialog::Create( CWnd* pParentWnd /*= NULL */ )
{
	return CDialogEx::Create(m_nResId, pParentWnd);
}

void CLdDialog::OnOK()
{
	
}

void CLdDialog::OnCancel()
{
	
}

