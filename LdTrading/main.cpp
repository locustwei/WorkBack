// Test.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ui\MainDlg.h"
#include "ITradInterface.h"
#include "interface\TdxTrading.h"

#define MAX_LOADSTRING 100

// 此代码模块中包含的函数的前向声明:

HINSTANCE hInstance;
ITradInterface* TradClient;

//连接交易软件（目前只支持通达信）
ITradInterface* ConnectTradClient()
{
	return new CTdxTrading();
}

int APIENTRY _tWinMain(HINSTANCE hInst,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInstance = hInst;

	CMainDlg dlg;
	dlg.ShowDialog(MAKEINTRESOURCE(IDD_DIALOG_MAIN));
	
	TradClient = ConnectTradClient();

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

	return (int) msg.wParam;
}

