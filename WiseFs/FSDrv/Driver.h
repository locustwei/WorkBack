#pragma once

typedef struct _WFS_GLOBAL_DATA {
	PDRIVER_OBJECT DriverObject;
	PDEVICE_OBJECT CtrlDevice;
	PFLT_FILTER  FilterHandle;          //驱动Handle。注册时生成，卸载时释放
	ULONG WinVer;
} WFS_GLOBAL_DATA;

extern WFS_GLOBAL_DATA Globals;
extern ULONG WIN_VER;


extern HANDLE g_WiseProcessId;