#include <ntddk.h>  

#define IOCTL_ADD_MAL_FILE          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x702 + 0x0001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REOME_MAL_FILE        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x702 + 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_PROCESS_NOTIFY    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x702 + 0x0003, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMOVE_PROCESS_NOTIFY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x702 + 0x0004, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GET_NOTIFY_DATA       CTL_CODE (FILE_DEVICE_UNKNOWN, 0x702 + 0x0005, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_NOTIFY_RESULT     CTL_CODE (FILE_DEVICE_UNKNOWN, 0x702 + 0x0006, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MAL_HASH_COUNT 100


#pragma pack(push, 1)

typedef struct _MAL_FILE_NAME
{
	UCHAR nLen;
	//BOOLEAN block;
	WCHAR Name[1];
}MAL_FILE_NAME, *PMAL_FILE_NAME;

typedef struct _REGISTER_CALLBACK_IN {  
	ULONG positionId; 
	ULONG UserEvent;                       //Event Handle当有操作被检测到通过此通知用户程序
} REGISTER_CALLBACK_IN, *PREGISTER_CALLBACK_IN;

typedef struct _PROC_NOTIFY_DATA_OUT
{
	ULONG size;
	ULONG64 Cookie;
	ULONG ParentID;
	ULONG ProcessID;
	ULONG CommandLine;
	ULONG ImageFile;
	WCHAR Data[1];
}PROC_NOTIFY_DATA_OUT, *PPROC_NOTIFY_DATA_OUT;

typedef struct _PROC_NOTIFY_DATA_IN { 
	ULONG64 DataCookie;                    //操作数据Cookie，实际为数据指针
	BOOLEAN Denied;                        //是否拒绝
}PROC_NOTIFY_DATA_IN, *PPROC_NOTIFY_DATA_IN;


#pragma  pack(pop)


void CreateProcessNotifyRoutineEx(HANDLE ParentID, HANDLE ProcessID, PPS_CREATE_NOTIFY_INFO CreateInfo);

NTSTATUS InitGlobalData();
NTSTATUS FreeGlobalData();

NTSTATUS AddMalFileName(ULONG nSize, PMAL_FILE_NAME pName);
NTSTATUS RemoveMalFileName(ULONG nSize, PMAL_FILE_NAME pName);
NTSTATUS SetProcessNotify(ULONG InputBufferLength, PREGISTER_CALLBACK_IN pInput);
NTSTATUS RemoveProcessNotify();
NTSTATUS GetRegNotifyData(ULONG OutputBufferLength, PPROC_NOTIFY_DATA_OUT pOut, PULONG_PTR pRetlen);
NTSTATUS SetRegNotifyResult(ULONG InputBufferLength, PPROC_NOTIFY_DATA_IN Input);