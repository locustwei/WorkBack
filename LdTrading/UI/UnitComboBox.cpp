#include "UnitComboBox.h"


CUnitComboBox::CUnitComboBox(void)
{

}


CUnitComboBox::~CUnitComboBox(void)
{
}

BOOL CUnitComboBox::OnInitWnd()
{
	CLdComboBox::OnInitWnd();
	m_ListWnd->AddString(L"%");
	m_ListWnd->AddString(L"Ԫ");
	ComboBox_SetCurSel(m_hWnd, 0);
	return TRUE;
}
