/*++

--*/
#include <fltKernel.h>
#include "filedisk.h"
#include "CtrlDevice.h"
#include "filedisk.h"
#include "Driver.h"
#include "FltCallback.h"
#include "FsData.h"
#include "..\..\WiseRegNotify\DirverUtils.h"

//全局变量
WFS_GLOBAL_DATA Globals={0};


DRIVER_INITIALIZE DriverEntry;
//入口
NTSTATUS DriverEntry (__in PDRIVER_OBJECT DriverObject,__in PUNICODE_STRING RegistryPath);

HANDLE g_WiseProcessId = NULL;


NTSTATUS
	DeviceCreate (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	if(DeviceObject->DeviceType == FILE_DEVICE_DISK)
		return FileDiskCreateClose(DeviceObject, Irp);
	else{
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

	return STATUS_SUCCESS;
}

NTSTATUS
	DeviceClose (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	if(DeviceObject->DeviceType == FILE_DEVICE_DISK)
		return FileDiskCreateClose(DeviceObject, Irp);
	else{
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}
	return STATUS_SUCCESS;
}



NTSTATUS
	DeviceCleanup (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	if(DeviceObject == Globals.CtrlDevice){
		return CtrlDeviceCleanup(DeviceObject, Irp);
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


NTSTATUS
	DeviceControl (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	if(DeviceObject == Globals.CtrlDevice)
		return CtrlDeviceControl(DeviceObject, Irp);
	else if(DeviceObject->DeviceType == FILE_DEVICE_DISK)
		return FileDiskDeviceControl(DeviceObject, Irp);
	else{
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}

	return STATUS_SUCCESS;
}

NTSTATUS
	DeviceReadWrite (
	IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Irp
	)
{
	if(DeviceObject->DeviceType == FILE_DEVICE_DISK)
		return FileDiskReadWrite(DeviceObject, Irp);
	else //if(DeviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM
	{
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}
	return STATUS_SUCCESS;
}


VOID DeviceUnload (
	__in PDRIVER_OBJECT DriverObject
	)
{
	if(Globals.FilterHandle){
		FltUnregisterFilter( Globals.FilterHandle );
		Globals.FilterHandle = NULL;
	}

	if(Globals.CtrlDevice)
		CtrlDeviceUnload(DriverObject);
	FileDiskUnload(DriverObject);
}

//  Filter driver initialization and unload routines
NTSTATUS DriverEntry (__in PDRIVER_OBJECT DriverObject, __in PUNICODE_STRING RegistryPath)
{
	NTSTATUS Status;
	UNICODE_STRING windir;
	RTL_OSVERSIONINFOEXW VersionInformation={sizeof(RTL_OSVERSIONINFOEXW)};

	Status = RtlGetVersion((PRTL_OSVERSIONINFOW)&VersionInformation);
	if(!NT_SUCCESS(Status))
		return Status;
	Globals.WinVer = VersionInformation.dwMajorVersion;
	
	//注册驱动
	Status = RegisiterFltCallback(DriverObject);

	if (!NT_SUCCESS( Status )) {

		goto lbError;
	}

	Status = CreateCtrlDevice(DriverObject);
	if(!NT_SUCCESS(Status))
		goto lbError;

	//Status = CreateFileDiskDevices(DriverObject);
	Status = CreateDeviceDirectory();
	if(!NT_SUCCESS(Status))
		goto lbError;

	Globals.DriverObject = DriverObject;

	DriverObject->MajorFunction[IRP_MJ_CREATE]         = DeviceCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE]          = DeviceClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DeviceCleanup;
	DriverObject->MajorFunction[IRP_MJ_READ]           = DeviceReadWrite;
	DriverObject->MajorFunction[IRP_MJ_WRITE]          = DeviceReadWrite;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
	DriverObject->DriverUnload                         = DeviceUnload;

	goto lbExit;

lbError:
	
	if(Globals.FilterHandle)
		FltUnregisterFilter( Globals.FilterHandle );
	Globals.FilterHandle = NULL;

lbExit:
	
	//InfoPrint("DriverEntry Status = 0x%x", Status);

	return Status;
}


