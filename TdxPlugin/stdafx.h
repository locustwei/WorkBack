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

#define MM_CUSTOM WM_USER + 0xA0CD
#define MM_RUNONMAINTHREAD MM_CUSTOM + 1      //向主窗口线程发消息，以便让代码运行在主线程空间。wprarm: 函数指针；lparam：参数
typedef int (* RUNONPROC)(LPARAM);            //主线程运行函数定义 
#define MM_LOGINED MM_CUSTOM + 2


extern HINSTANCE hInstance;  //动态库handle
extern DWORD dwViceThreadId; //副线程ID