// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// LeadowStockHepler.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "LeadowStockHepler.h"
#include "MainFrm.h"
#include "..\StockDataAPI\web\HttpStockData.h"
#include "tdxinterface\TdxTrading.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLeadowStockHeplerApp

BEGIN_MESSAGE_MAP(CLeadowStockHeplerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CLeadowStockHeplerApp::OnAppAbout)
END_MESSAGE_MAP()


// CLeadowStockHeplerApp 构造

CLeadowStockHeplerApp::CLeadowStockHeplerApp()
{
	m_bHiColorIcons = TRUE;

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Leadow.StockHepler.V0.1"));

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	m_bSaveState = FALSE;
}

// 唯一的一个 CLeadowStockHeplerApp 对象

CLeadowStockHeplerApp theApp;

// CLeadowStockHeplerApp 初始化

BOOL CLeadowStockHeplerApp::InitInstance()
{
	CWinAppEx::InitInstance();
	// 初始化 OLE 库
// 	if (!AfxOleInit())
// 	{
// 		AfxMessageBox(IDP_OLE_INIT_FAILED);
// 		return FALSE;
// 	}

	EnableTaskbarInteraction(FALSE);

	// 使用 RichEdit 控件需要  AfxInitRichEdit2()	
	// AfxInitRichEdit2();

	SetRegistryKey(_T("LeadowStockHepler"));

	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();

	InitScriptEng();

	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 若要创建主窗口，此代码将创建新的框架窗口
	// 对象，然后将其设置为应用程序的主窗口对象
	CMainFrame* pFrame = new CMainFrame();
	if (!pFrame)
		return FALSE;
	pFrame->EnableLoadDockState(FALSE);

	m_pMainWnd = pFrame;
	// 创建并加载框架及其资源
	pFrame->LoadFrame(IDR_MAINFRAME);
	// 唯一的一个窗口已初始化，因此显示它并对其进行更新
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 SDI 应用程序中，这应在 ProcessShellCommand 之后发生
	return TRUE;
}

int CLeadowStockHeplerApp::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
	AfxOleTerm(FALSE);

	delete m_ScriptEng;
	delete m_TradClient;
	delete m_DateInterface;

	return CWinAppEx::ExitInstance();
}

// CLeadowStockHeplerApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CLeadowStockHeplerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CLeadowStockHeplerApp 自定义加载/保存方法

void CLeadowStockHeplerApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_NAV);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_NAV);
}

void CLeadowStockHeplerApp::LoadCustomState()
{
}

void CLeadowStockHeplerApp::SaveCustomState()
{
}

//连接交易软件（目前只支持通达信）
ITradInterface* ConnectTradClient()
{
	return new CTdxTrading();
}

void CLeadowStockHeplerApp::InitScriptEng()
{
	m_ScriptEng = new CScriptEng();
	m_DateInterface = new CHttpStockData();
	m_TradClient = ConnectTradClient();
	m_ScriptEng->SetDataInterface(m_DateInterface);
	m_ScriptEng->SetTradInterface(m_TradClient);
}

// CLeadowStockHeplerApp 消息处理程序



