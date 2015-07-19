// Test.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ui\MainDlg.h"
#include "..\StockDataAPI\web\HttpStockData.h"
#include <commctrl.h>
#define MAX_LOADSTRING 100

#pragma comment(lib,"Comctl32.lib")

// 此代码模块中包含的函数的前向声明:

HINSTANCE hInstance;
ITradInterface* TradClient = NULL;
CScriptEng* ScriptEng = NULL;
IDataInterface* DateInterface = NULL;

//连接交易软件（目前只支持通达信）
ITradInterface* ConnectTradClient()
{
	return new CTdxTrading();
}

BOOL InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE hInst,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInstance = hInst;

	InitInstance();

	CMainDlg dlg;
	dlg.ShowDialog(MAKEINTRESOURCE(IDD_DIALOG_MAIN));
	
	ScriptEng = new CScriptEng();
	DateInterface = new CHttpStockData();
	TradClient = ConnectTradClient();
	ScriptEng->SetDataInterface(DateInterface);
	ScriptEng->SetTradInterface(TradClient);

	MSG msg;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if(ScriptEng)
		delete ScriptEng;
	if(TradClient)
		delete TradClient;
	if(DateInterface)
		delete DateInterface;

	return 0;
}

