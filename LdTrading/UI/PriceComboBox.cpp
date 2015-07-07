#include "PriceComboBox.h"


CPriceComboBox::CPriceComboBox(void)
{
}


CPriceComboBox::~CPriceComboBox(void)
{
}

BOOL CPriceComboBox::OnInitWnd()
{
	CLdComboBox::OnInitWnd();
	m_ListWnd->AddString(L"当前价");
	m_ListWnd->AddString(L"昨收价");
	m_ListWnd->AddString(L"最高价");
	m_ListWnd->AddString(L"最低价");
	return TRUE;
}
