
#pragma once

#ifdef _ANTI_
#define DRIVER_NAME             L"WiseAntiReg"
#define NT_DEVICE_NAME          L"\\Device\\WiseAntiReg"
#define DOS_DEVICES_LINK_NAME   L"\\DosDevices\\WiseAntiReg"
#define WIN32_DEVICE_NAME       L"\\\\.\\WiseAntiReg"
//#define NOTIFY_EVENT_NAME       L"\\WiseAntiRegEvent"
#else
#define DRIVER_NAME             L"WiseRegNotify"
#define NT_DEVICE_NAME          L"\\Device\\WiseRegNotify"
#define DOS_DEVICES_LINK_NAME   L"\\DosDevices\\WiseRegNotify"
#define WIN32_DEVICE_NAME       L"\\\\.\\WiseRegNotify"
#define NOTIFY_EVENT_NAME       L"\\WiseRegNotifyEvent"
#endif

#define IOCTL_REGISTER_CALLBACK        CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 1), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_UNREGISTER_CALLBACK      CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 2), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_CALLBACK_VERSION     CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 3), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_CONTEXT_DATA         CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 4), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_NOTIFY_RESULT        CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 5), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ADD_EXCLUDE              CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 6), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMOVE_EXCLUDE           CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 7), METHOD_BUFFERED, FILE_ANY_ACCESS)
//#define IOCTL_GET_EXCLUDE              CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 8), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CHANGE_CALLBACK          CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 9), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GETFILTER_POSITIONS      CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 10), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GETEXCLUDE_FILES         CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 11), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SETFILTER_POSITIONS      CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 12), METHOD_BUFFERED, FILE_ANY_ACCESS)

// typedef enum _NOTIFY_CLASS { //注册表操作动作类型 （WinXP没有前后之分）
// 	NotifyClassNone       = 0x0000,  //无意义
// 	PreDeleteKey          = 0x0001,   //删除Key（前）
// 	PostDeleteKey         = 0x0002,	  //删除Key（后）
// 	PreRenameKey          = 0x0003,	  //重命名Key（前）
// 	PostRenameKey         = 0x0004,	  //重命名Key（后）
// 	PreCreateKey          = 0x0005,	  //添加子Key（前）
// 	PostCreateKey         = 0x0006,	  //添加子Key（后）
// 	PreSetValueKey        = 0x0007,	  //添加或修改Key属性值（前）
// 	PostSetValueKey       = 0x0008,	  //添加或修改Key属性值（后）
// 	PreDeleteValueKey     = 0x0009,	  //删除Key属性值（前）
// 	PostDeleteValueKey    = 0x000A,	   //删除Key属性值（后）
// 	PreSetInformationKey  = 0x000B,
// 	PostSetInformationKey = 0x000C,
// } NOTIFY_CLASS;

#pragma pack(push, 1)

typedef enum _CALLBACK_NOTIFY{ //0: 通知用户程序；1：直接拒绝；2：直接允许；
	RN_NOTIFY = 0, 
	RN_ALLOW  = 1,
	RN_DENY   = 2
}CALLBACK_NOTIFY;

typedef struct _REGISTER_CALLBACK_IN {  //注册回掉函数传入参数
	ULONG positionId;                           //参数总长度
	ULONG UserEvent;                       //Event Handle当有操作被检测到通过此通知用户程序
	//REGFILTER_KEY Keys[0];
} REGISTER_CALLBACK_IN, *PREGISTER_CALLBACK_IN;

typedef struct _REGISTER_CALLBACK_RESULT_IN {  //设置注册表操作结果传入参数
	ULONG64 DataCookie;                    //操作数据Cookie，实际为数据指针
	CALLBACK_NOTIFY Denied;                        //是否拒绝
} REGISTER_CALLBACK_RESULT_IN, *PREGISTER_CALLBACK_RESULT_IN;

typedef struct _REGISTER_NOTIFY_DATA_OUT {  //获取注册表操作数据的输出参数。
	ULONG nSize;                            //数据总长度     
	ULONG64 Cookie;      
	union{
		ULONG positionId;                     //驱动内使用，是否拒绝标志
		CALLBACK_NOTIFY Denied;
	};
	ULONG NotifyClass;                      //注册表动作。
	ULONG FileName;                         //文件名
	ULONG KeyName;                          //完整的Key字符串存放的偏移位置
	ULONG SubKeyName;                       //子键名字符串存放的偏移位置
	ULONG ValueName;                        //属性名字符串存放的偏移位置
	ULONG NewName;
} REGISTER_NOTIFY_DATA_OUT, *PREGISTER_NOTIFY_DATA_OUT;

typedef struct _EXCLUDE_ADD_IN {            //添加排外列表	
	ULONG Namesize;                            
	ULONG pid;                              //进程ID
	ULONG Denied;                           //（允许/拒绝）
	WCHAR Filename[0];                      //进程文件名
} EXCLUDE_ADD_IN, *PEXCLUDE_ADD_IN;

typedef struct _EXCLUDE_POSITION_IN {       //排外位置
	USHORT positionId;
	CALLBACK_NOTIFY Denied;                          //是否拒绝
} EXCLUDE_POSITION_IN, *PEXCLUDE_POSITION_IN;

typedef struct _GET_POSITION_OUT {         //获取所有监控位置
	USHORT id;
	ULONG name;                          
	CALLBACK_NOTIFY Denied;
} GET_POSITION_OUT, *PGET_POSITION_OUT;

typedef struct _GET_EXCLUDE_FILE_OUT {     //获取所有排外程序  
	CALLBACK_NOTIFY Denied;
	ULONG name;                          
} GET_EXCLUDE_FILE_OUT, *PGET_EXCLUDE_FILE_OUT;

#pragma pack(pop)