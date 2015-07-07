// LdTabCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "LdTabCtrl.h"
#include "LdGroupBox.h"


// CLdTabCtrl


CLdTabSheet::CLdTabSheet():m_BtnSize(80, 30)
{
	m_CurSheetId=-1;
	OnAfterSheetChange=NULL;
	OnBeforeSheetChange=NULL;
}

CLdTabSheet::~CLdTabSheet()
{
	for(int i=0; i<m_Sheets.GetCount(); i++)
		delete m_Sheets[i];
	m_Sheets.Clear();

	for(int i=0; i<m_Btns.GetCount(); i++)
		delete m_Btns[i];
	m_Btns.Clear();
	FREEMEM(m_SheetBtnClickHandler);
}

UINT CLdTabSheet::AddSheet( LPCTSTR lpCaption )
{
	CLdSheet* sheet=new CLdSheet();
	CRect rcSheet=GetSheetRect();
	sheet->Create(this->m_hWnd);
	sheet->MoveWindow(rcSheet);
	m_Sheets.Add(sheet);

	CLdButton* btn=new CLdButton();
	btn->Create(this->m_hWnd);
	int n=m_Btns.Add(btn);
	CRect rcBtn(n*m_BtnSize.cx, 0, (n+1)*m_BtnSize.cx, m_BtnSize.cy);
	btn->MoveWindow(rcBtn);
	SetWindowText(btn->m_hWnd, lpCaption);
	btn->OnClick=m_SheetBtnClickHandler;
	btn->m_Tag=n;
	::ShowWindow(btn->m_hWnd, SW_SHOW);
	if(n==0)
		ChangeSheet(n);

	return n;
}

void CLdTabSheet::OnSize( UINT nType, int cx, int cy )
{
	CRect rcSheet=GetSheetRect();
	for (int i=0; i<m_Sheets.GetCount(); i++){
		::MoveWindow(m_Sheets[i]->m_hWnd, rcSheet.left, rcSheet.top, rcSheet.right-rcSheet.left, rcSheet.bottom-rcSheet.top, 0);
	}
}

CLdButton* CLdTabSheet::GetSheetBtn( int nId )
{
	if((nId<0)||(nId>=m_Btns.GetCount()))
		return NULL;
	return m_Btns[nId];
}

int CLdTabSheet::SetCurSheet( int nId )
{
	ChangeSheet(nId);
	return m_CurSheetId;
}

void CLdTabSheet::ChangeSheet( int newId )
{
	if((newId<0)||(newId>=m_Btns.GetCount())||(newId==m_CurSheetId))
		return;

	if(OnBeforeSheetChange!=NULL)
		if(!OnBeforeSheetChange->OnItemChang(this, newId))
			return;

	CLdButton* btn=GetSheetBtn(m_CurSheetId);
	if(btn){
		btn->m_Selected=FALSE;
		btn->Invalidate();
	}
	btn=GetSheetBtn(newId);
	if(btn){
		btn->m_Selected=TRUE;
		btn->Invalidate();
	}

	CLdSheet* sheet=GetSheet(m_CurSheetId);
	if(sheet)
		sheet->ShowWindow(SW_HIDE);
	sheet=GetSheet(newId);
	sheet->ShowWindow(SW_SHOW);

	UINT nOld=m_CurSheetId;
	m_CurSheetId=newId;
	if(OnAfterSheetChange!=NULL)
		OnAfterSheetChange->OnItemChang(this, nOld);

}

CLdSheet* CLdTabSheet::GetSheet( int nId )
{
	if((nId<0)||(nId>=m_Sheets.GetCount()))
		return NULL;
	return m_Sheets[nId];
}

CRect CLdTabSheet::GetSheetRect()
{
	CRect rcClient;
	GetClientRect(m_hWnd, rcClient);
	rcClient.top=m_BtnSize.cy;
	return rcClient;
}

void CLdTabSheet::SetBtnSize( CSize size )
{
	if(m_BtnSize==size)
		return;
	m_BtnSize=size;

	for(int i=0; i<m_Btns.GetCount(); i++){
		CRect rcBtn(i*m_BtnSize.cx, 0, (i+1)*m_BtnSize.cx, m_BtnSize.cy);
		m_Btns[i]->MoveWindow(rcBtn);
	}

	OnSize(0, 0, 0);
}

void CLdTabSheet::SheetBtnClick( CLdButton* Sender )
{
	ChangeSheet(Sender->m_Tag);
}

int CLdTabSheet::GetCurSheetId( CLdSheet** tabSheet )
{
	if(tabSheet!=NULL)
		*tabSheet=GetSheet(m_CurSheetId);
	return m_CurSheetId;
}



// CLdTabCtrl 消息处理程序


