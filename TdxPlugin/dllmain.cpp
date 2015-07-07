// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "addinentry.h"
#include "..\LeadowUi\LdWnd.h"

HINSTANCE hInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		CLdWnd::hInstance = hInstance;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//进程注入方式动态库导出函数。
extern "C" int __declspec( dllexport ) RunPlugin(DWORD pid)
{
	InstallHooks(pid);

	
	while(true)   //线程不能结束。否则崩溃
		Sleep(10);

	return 0;
}

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
	timerid = SetTimer(NULL, 123, 100, &TimerProc);


	HMODULE h = LoadLibrary(L"AddinCommonControl.dll");

	Addin_ p = (Addin_)GetProcAddress(h, "Addin_GetObject");

	int o = p();

	return o;
}

extern "C" void __declspec( dllexport ) PlugTest()
{
	InitOnMainThread(0);

}