
#pragma once

#define DEVICE_BASE_NAME    L"\\WiseFileDisk"
#define DEVICE_DIR_NAME     L"\\Device" DEVICE_BASE_NAME      
#define DEVICE_NAME_PREFIX  DEVICE_DIR_NAME     DEVICE_BASE_NAME
#define SYMBOL_NAME_PREFIX  L"\\Global??\\%C:" //L"\\DosDevices\\Global\\%C:"
//#define DISK_FLAG           "06BB01EB9B2D4179BFBF6ADB625283AC"

#pragma region 虚拟磁盘文件控制号
#define IOCTL_FILE_DISK_OPEN_FILE   CTL_CODE(FILE_DEVICE_DISK, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_FILE_DISK_CLOSE_FILE  CTL_CODE(FILE_DEVICE_DISK, 0x902, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_FILE_DISK_QUERY_FILE  CTL_CODE(FILE_DEVICE_DISK, 0x903, METHOD_BUFFERED, FILE_READ_ACCESS)
#pragma endregion 虚拟磁盘文件控制号

#pragma region 虚拟磁盘用户控制号
#define IOCTL_VHD_ENUM_DEVICE           CTL_CODE (FILE_DEVICE_UNKNOWN, (0xB00 + 1), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VHD_CREATE_DEVICE         CTL_CODE (FILE_DEVICE_UNKNOWN, (0xB00 + 2), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VHD_REMOVE_DEVICE         CTL_CODE (FILE_DEVICE_UNKNOWN, (0xB00 + 3), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VHD_SET_DATA              CTL_CODE (FILE_DEVICE_UNKNOWN, (0xB00 + 4), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VHD_GET_DATA              CTL_CODE (FILE_DEVICE_UNKNOWN, (0xB00 + 5), METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_VHD_UNMOUNT               CTL_CODE (FILE_DEVICE_UNKNOWN, (0xB00 + 6), METHOD_BUFFERED, FILE_ANY_ACCESS)
#pragma endregion 虚拟磁盘用户控制号

#pragma pack(push, 1)

typedef struct _SET_DISK_FILE_DATA {
	ULONG DeviceNumber;
	ULONGLONG FilePosition;
	ULONG DataSize;
	UCHAR Data[1];
}SET_DISK_FILE_DATA, *PSET_DISK_FILE_DATA;

typedef struct _OPEN_FILE_INFORMATION {
	ULONG           cb;
	ULONG           DeviceNumber;
	LARGE_INTEGER   FileSize;
	BOOLEAN         ReadOnly;
	WCHAR           DriveLetter;
	USHORT          FileNameLength;
	WCHAR            FileName[1];
} OPEN_FILE_INFORMATION, *POPEN_FILE_INFORMATION;

#pragma pack(pop)


#pragma region 虚拟文件驱动函数

NTSTATUS 
	MountFileDiskDevice (
	PDRIVER_OBJECT DriverObject,
	POPEN_FILE_INFORMATION pInfo, 
	OUT PULONG DeviceNumber
	);
NTSTATUS 
	UmountFileDiskDevice (
	PDRIVER_OBJECT DriverObject,
	ULONG DeviceNumber
	);
NTSTATUS 
	EnumDiskFiles(
	IN PDRIVER_OBJECT DriverObject, 
	OUT PVOID Buffer, 
	ULONG* BufferLength);
// NTSTATUS
// 	CreateFileDiskDevices (
// 	IN PDRIVER_OBJECT   DriverObject
// 	);
NTSTATUS CreateDeviceDirectory();

NTSTATUS 
	SetDiskFileData (
	PDRIVER_OBJECT DriverObject,
	PSET_DISK_FILE_DATA pData
	);
NTSTATUS 
	GetDiskFileData(
	PDRIVER_OBJECT DriverObject, 
	PSET_DISK_FILE_DATA pData, 
	PVOID pOut, 
	PULONG Outlen);
VOID
	FileDiskUnload (
	IN PDRIVER_OBJECT   DriverObject
	);

NTSTATUS
	FileDiskDeleteDevice (
	IN PDEVICE_OBJECT   DeviceObject
	);

NTSTATUS
	FileDiskCreateClose (
	IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Irp
	);

NTSTATUS
	FileDiskReadWrite (
	IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Irp
	);

NTSTATUS
	FileDiskDeviceControl (
	IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Irp
	);
NTSTATUS 
	RemoveFileDiskDevice (
	PDRIVER_OBJECT DriverObject, 
	ULONG DeviceNumber
	);

#pragma endregion 虚拟文件驱动函数
