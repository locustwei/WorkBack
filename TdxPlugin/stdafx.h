// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once


#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include "windows.h"
// TODO: 在此处引用程序需要的其他头文件
#include "resource.h"
#include "TdxTradSocket.h"

#define MM_CUSTOM WM_USER + 0xA0CD
#define MM_RUNONMAINTHREAD MM_CUSTOM + 1      //向主窗口线程发消息，以便让代码运行在主线程空间。wprarm: 函数指针；lparam：参数 见：RunOnMainThread
#define MM_LOGINED MM_CUSTOM + 2              //登录后向主窗口发生消息  
#define MM_STOCK_BY_END MM_CUSTOM + 3         //股票买入下单完成向买入窗口发送消息
#define MM_STOCK_SELL_END MM_CUSTOM + 4       //股票卖出下单完成向卖出窗口发送消息

extern HINSTANCE hInstance;  //动态库handle
extern DWORD dwViceThreadId; //副线程ID
extern HWND hMainWnd;
extern CTdxTradSocket* TradSocket;

typedef void (* RunThreadFunc)(LPARAM lparam); 

//在辅助线程中执行（如处理模态对话框）
inline void RunOnViceThread(RunThreadFunc func, LPARAM lparam)
{
	PostThreadMessage(dwViceThreadId, WM_USER, (WPARAM)func, lparam); 
}
//在主线程执行（所有与界面相关都尽量在主线程中执行）
inline void RunOnMainThread(RunThreadFunc func, LPARAM lparam)
{
	PostMessage(hMainWnd, MM_RUNONMAINTHREAD, (WPARAM)func, lparam); 
}