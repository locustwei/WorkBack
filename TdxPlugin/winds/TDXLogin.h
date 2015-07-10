/*
通达信网上交易登录窗口
*/

#pragma once
#include "..\stdafx.h"
#include "..\..\PublicLib\hooks\WindowHook.h"

//登录窗口控件DlgItemID
#define ID_ZHLX = 0x044B      //账号类型（资金账号）
#define ID_DLFWQ = 0x044F	  //登录服务器名        
#define ID_ZH = 0x044C		  //账号                
#define ID_QD = 0x0001		  //确定按钮            
#define ID_QX = 0x0002		  //取消按钮            
#define ID_MM = 0x0450		  //密码                
#define ID_AQFS = 0x0457	  //安全方式            
#define ID_YZM = 0x0458		  //验证码              

class CTDXLogin :public CWndHook
{
private:
	HWND m_CbAType;  //账号类型（资金账号）  0x044B
	HWND m_CbSever;  //登录服务器名          0x044F
	HWND m_CbZH;     //账号                  0x044C
	HWND m_BtOk;     //确定按钮              0x0001
	HWND m_BtQX;     //取消按钮              0x0002
	HWND m_EdMM;     //密码                  0x0450
	HWND m_CbJYFS;   //安全方式              0x0457
	HWND m_EdYZM;    //验证码                0x0458
protected:
	virtual LRESULT WndPROC(HWND hwnd, UINT nCode,WPARAM wparam,LPARAM lparam);
public:
	CTDXLogin(HWND hWnd);
	~CTDXLogin(void);
	BOOL getYZM(LPTSTR szCode);       //获取验证码
public:
	static CTDXLogin* WndHooker;
	static void HookLoginWnd(HWND hwnd);
};

