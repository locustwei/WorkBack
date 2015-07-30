// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "winds\TDXLogin.h"
#include "..\publiclib\Utils_Wnd.h"
#include "winds\TDXMain.h"
#include "..\publiclib\comps\NotifyIcon.h"
#include "..\publiclib\comps\LdList.h"
#include "TDXDataStruct.h"
#include <stdio.h>
#include "TdxTradSocket.h"

HINSTANCE hInstance;
DWORD dwViceThreadId;
CTdxTradSocket* TradSocket;
HWND hWndLogin = NULL;
HWND hMainWnd = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//----------------------------------------------------------------------------------

BOOL CALLBACK EnumThreadWndProc(HWND hwnd, LPARAM lparam)
{
	if(WndClassNameIs(hwnd, CN_Dialog)){
		DWORD style = GetWindowLong(hwnd, GWL_STYLE);
		if(style==0x94000044){
			hWndLogin = hwnd;
		}else if(style==0x0CCF0044)
			hMainWnd = hwnd;
	}
	return hWndLogin == NULL || hMainWnd == NULL;
}

WNDPROC oldProc;

//启动时临时使用钩子，用于注入代码进入主线程
LRESULT WINAPI TempWndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam)
{
	if(nCode==MM_RUNONMAINTHREAD){
		(WNDPROC)SetWindowLongPtr(hWndLogin, GW_WNDPROC, (LONG)oldProc); //钩子使命完成还原他。
		((RunThreadFunc)wparam)(lparam);
	}
	return CallWindowProc(oldProc, hwnd, nCode, wparam, lparam);
}

//副线程，这个线程用于处理诸如主线程中的模态对话框、循环等待，等问题
DWORD WINAPI ViceThreadProc(_In_ LPVOID lpParameter)
{
	MSG msg;
	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message == WM_QUIT)
			break;
		else if(msg.wParam!=0){
			RunThreadFunc fun = (RunThreadFunc)msg.wParam;
			fun(msg.lParam);
		}
	}

	return 0;
}

//在主线程出初始化。
void InitOnMainThread(LPARAM param)
{

	if(hWndLogin!=NULL){
		CTDXLogin::WndHooker = new CTDXLogin(hWndLogin);
		CTDXLogin::WndHooker->StartHook();
	}

	if(hMainWnd!=NULL){
		CTDXMain::WndHooker = new CTDXMain(hMainWnd);
		CTDXMain::WndHooker->StartHook();

	}

	CreateThread(NULL, 0, &ViceThreadProc, NULL, 0, &dwViceThreadId);

	TradSocket = new CTdxTradSocket();

}

BOOL InstallHooks(DWORD tid)
{
	EnumThreadWindows(tid, &EnumThreadWndProc, 0);
	if(hWndLogin!=NULL){
		if(tid == GetCurrentThreadId())
			InitOnMainThread(0);
		else{
			/*当前线程并非主线程，如果在这个线程做创建窗口等动作会有许多线程同步问题。
			所以先找到登录窗口设置消息钩子，然后发送消息在主线程执行初始化函数*/

			oldProc = (WNDPROC)SetWindowLongPtr(hWndLogin, GW_WNDPROC, (LONG)&TempWndPROC);
			PostMessage(hWndLogin, MM_RUNONMAINTHREAD, (WPARAM)&InitOnMainThread, 0);
		}
		return TRUE;
	}else
		return FALSE;
}

void UnInstallHooks()
{
}
//-------------------------------------------------------------------------------------------------------------------------------------
//进程注入方式动态库导出函数。
extern "C" int __declspec( dllexport ) RunPlugin(DWORD pid)
{
	InstallHooks(pid);

	
	while(true)   //线程不能结束。否则崩溃
		Sleep(10);

	return 0;
}
//------------------------------------------------------------------------------------------------------------------------------------
typedef int (* Addin_)(); 
INT_PTR timerid;

VOID CALLBACK TimerProc(
	__in  HWND hwnd,
	__in  UINT uMsg,
	__in  UINT_PTR idEvent,
	__in  DWORD dwTime
	)
{
	if(InstallHooks(GetCurrentThreadId()))
		KillTimer(NULL, timerid);
}
//通达信插件方式导出函数
extern "C" int __declspec( dllexport ) Addin_GetObject()
{
	 //加载Dll是程序刚刚启动窗口还没创建，设置定时器等待。
	timerid = SetTimer(NULL, 123, 100, &TimerProc); 


	HMODULE h = LoadLibrary(L"AddinCommonControl.dll");

	Addin_ p = (Addin_)GetProcAddress(h, "Addin_GetObject");

	int o = p();

	return o;
}
//------------------------------------------------------------------------------------------------------------------------------------
//测试用
#ifdef _DEBUG
extern "C" void __declspec( dllexport ) PlugTest()
{
	InitOnMainThread(0);
}
#endif