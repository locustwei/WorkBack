
#pragma once

#define VERSION                 211
#define DRIVER_NAME             L"WiseFS"
#define NT_DEVICE_NAME          L"\\Device\\" DRIVER_NAME
#define DOS_DEVICE_NAME         L"\\DosDevices\\" DRIVER_NAME

#define IOCTL_FS_VERSION              CTL_CODE (FILE_DEVICE_UNKNOWN, (0x900 + 0), METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma region 控制设备函数
NTSTATUS CtrlDeviceControl (__in PDEVICE_OBJECT DeviceObject,__in PIRP Irp);
NTSTATUS CreateCtrlDevice(__in PDRIVER_OBJECT DriverObject);
NTSTATUS CtrlDeviceCleanup (__in PDEVICE_OBJECT DeviceObject,__in PIRP Irp);
VOID CtrlDeviceUnload (__in PDRIVER_OBJECT DriverObject);
#pragma endregion 控制设备函数
