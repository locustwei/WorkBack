#pragma once
#include "LdDialog.h"
#include "..\ScriptEng\ScriptEng.h"

class CInputParamWnd
{
public:
	CInputParamWnd(){
		name = NULL;
		edit = NULL;
	}
	~CInputParamWnd(){
		if(name)
			delete name;
		if(edit)
			delete edit;
	}
	inline void Create(CWnd* pParent, int left, int top, int nWidth, LPCSTR szCaption){
		name = new CStatic();
		CString s(szCaption);

		CRect r(left, top, left + nWidth, top + 18); 
		name->Create(s, WS_CHILD|WS_VISIBLE|WS_GROUP, r, pParent);
		edit = new CEdit();
		r.left = r.right + 10;
		r.right = r.left + 200;
		edit->Create(WS_CHILD|WS_VISIBLE, r, pParent, 0);
	}

	CString GetValue(){
		if(edit){
			CString s;
			edit->GetWindowText(s);
			return s;
		}else
			return NULL;
	}
private:
	CStatic* name;
	CEdit* edit;
};

class CDlgStrategy: public CLdDialog
{
public:
	CDlgStrategy();
	~CDlgStrategy();

public:
	void SetStrategy( PSTRATEGY_STRCPIT pStrategy );
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
private:
	PSTRATEGY_STRCPIT m_pStrategy;
	CMap<LPCSTR, LPCSTR, CInputParamWnd*, CInputParamWnd*> m_ParamWnds;
	
	CEdit m_edName;
	CEdit m_edDesc;
	CMFCButton m_btnOk;
	CStatic m_pnlParam;
	afx_msg void OnClose();
	afx_msg void OnBtnTest();

	virtual BOOL OnInitDialog();
	void CreateControls();
	void ClearWindows();
	CString GetInputParamValue( LPSTR szParams );
};