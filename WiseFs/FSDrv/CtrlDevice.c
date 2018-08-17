#include <fltKernel.h>
#include "CtrlDevice.h"
#include "filedisk.h"
#include "Driver.h"
#include "FltCallback.h"
#include "FsData.h"
#include "..\..\WiseRegNotify\DirverUtils.h"

NTSTATUS CreateCtrlDevice(__in PDRIVER_OBJECT DriverObject)
{
	ULONG                           i;
	NTSTATUS                        status = STATUS_SUCCESS;
	UNICODE_STRING                  ntDeviceName;
	UNICODE_STRING                  afdDeviceName;
	UNICODE_STRING                  win32DeviceName;
	BOOLEAN                         fSymbolicLink = FALSE;

	do 
	{
		RtlInitUnicodeString(&ntDeviceName, NT_DEVICE_NAME);
		status = IoCreateDevice(DriverObject,
			0,
			&ntDeviceName,
			0,
			FILE_DEVICE_SECURE_OPEN,
			FALSE,
			&Globals.CtrlDevice);
		if (!NT_SUCCESS (status))
			break;

		RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);
		status = IoCreateSymbolicLink(&win32DeviceName, &ntDeviceName);
		if (!NT_SUCCESS(status))
			break;

		fSymbolicLink = TRUE;


	} while (FALSE);

	if(!NT_SUCCESS(status)){

		if(fSymbolicLink)
			IoDeleteSymbolicLink(&win32DeviceName);

		if(Globals.CtrlDevice!=NULL){
			IoDeleteDevice(Globals.CtrlDevice);
			Globals.CtrlDevice = NULL;
		}
	}
	return status;
}

VOID CtrlDeviceUnload (
	__in PDRIVER_OBJECT DriverObject
	)
{

	UNICODE_STRING  DosDevicesLinkName;

	RtlInitUnicodeString(&DosDevicesLinkName, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&DosDevicesLinkName);

	if(Globals.CtrlDevice){
		IoDeleteDevice(Globals.CtrlDevice);
		Globals.CtrlDevice = NULL;
	}

	FsFreeData();
}

NTSTATUS
	CtrlDeviceCleanup (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	FsEnableAll(FALSE);
	SaveConfigFile();

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
/*
NTSTATUS ChangeSessionID(ULONG sid, PULONGLONG outToken)
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE pid = NULL, hToken = NULL, hTokenDup = NULL;
	PEPROCESS pproc;
	do 
	{
		pid = NtCurrentProcess();;
		Status = ZwOpenProcessTokenEx(pid, TOKEN_ALL_ACCESS, 0, &hToken);
		if(!NT_SUCCESS(Status))
			break;
		Status = ZwDuplicateToken(hToken, MAXIMUM_ALLOWED, NULL, FALSE, TokenPrimary, &hTokenDup);
		if(!NT_SUCCESS(Status))
			break;
		Status = ZwSetInformationToken(hTokenDup, TokenSessionId, &sid, sizeof(sid));
		if(NT_SUCCESS(Status))
			*outToken = (ULONGLONG)hTokenDup;

	} while (FALSE);

	if(pid)
		NtClose(pid);
	if(hToken)
		ZwClose(hToken);
	if(hTokenDup)
		ZwClose(hTokenDup);

	return Status;
}
*/
NTSTATUS
	CtrlDeviceControl (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	PIO_STACK_LOCATION IrpStack;
	ULONG Ioctl;
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG InputBufferLength, OutputBufferLength;
	PVOID pInput, pOut;


	UNREFERENCED_PARAMETER(DeviceObject);

	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	Ioctl = IrpStack->Parameters.DeviceIoControl.IoControlCode;

	InputBufferLength  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
	pInput = Irp->AssociatedIrp.SystemBuffer;

	OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	pOut = Irp->AssociatedIrp.SystemBuffer;

	switch (Ioctl){
	case IOCTL_FS_VERSION:
		*(PULONG)pOut = VERSION;
		Irp->IoStatus.Information = sizeof(ULONG);
		Status = STATUS_SUCCESS;
		break;
	case IOCTL_SET_EXCLUDE_HANDLE:
		g_WiseProcessId = (HANDLE)*(PULONG)pInput;
		break;
		//--------------------------------------------------------------------------------------------------------------------------------
	case IOCTL_FS_ADD_FILE:
		if(pInput == NULL || InputBufferLength < sizeof(FS_FILE_RECORD) || ((PFS_FILE_RECORD)pInput)->cb != InputBufferLength ||
			OutputBufferLength != sizeof(ULONGLONG) || pOut == NULL)
			Status = STATUS_INVALID_PARAMETER;
		else{
			Status = AddFsFile((PFS_FILE_RECORD)pInput, (ULONGLONG*)pOut);
			if(NT_SUCCESS(Status))
				Irp->IoStatus.Information = sizeof(ULONGLONG);
		}
		break;
	case IOCTL_FS_REMOVE_FILE:
		Status = RemoveFsFile(*(ULONGLONG*)pInput);
		break;
	case IOCTL_FS_UPDATE_FILE:
		if(pInput == NULL || InputBufferLength < sizeof(FS_FILE_RECORD) || ((PFS_FILE_RECORD)pInput)->cb != InputBufferLength)
			Status = STATUS_INVALID_PARAMETER;
		else
			Status = UpdateFsFile((PFS_FILE_RECORD)pInput);
		break;
	case IOCTL_FS_ENUM_FILES:
		Status = EnumFsFiles(pOut, &OutputBufferLength);
		Irp->IoStatus.Information = OutputBufferLength;
		break;
	case IOCTL_FS_ENABLE_FILE:
		if(pInput == NULL || InputBufferLength != sizeof(SET_ENABLE_INPUT))
			Status = STATUS_INVALID_PARAMETER;
		else
			Status = SetFsFileEnable(((PSET_ENABLE_INPUT)pInput)->id, ((PSET_ENABLE_INPUT)pInput)->bDisable);
		break;
		//------------------------------------------------------------------------------------------------------------------------------------------
	case IOCTL_VHD_CREATE_DEVICE:
		if(pInput == NULL || InputBufferLength < sizeof(OPEN_FILE_INFORMATION) || InputBufferLength != ((POPEN_FILE_INFORMATION)pInput)->cb ||
			pOut == NULL || OutputBufferLength < sizeof(ULONG)){
			Status = STATUS_INVALID_PARAMETER;
		}else{
			Status = MountFileDiskDevice(DeviceObject->DriverObject, (POPEN_FILE_INFORMATION)pInput, (ULONG*)pOut);
			if(NT_SUCCESS(Status)){
				Irp->IoStatus.Information = sizeof(ULONG);
			}
		}
		break;
	case IOCTL_VHD_REMOVE_DEVICE:
		if(pInput == NULL || InputBufferLength < sizeof(ULONG)){
			Status = STATUS_INVALID_PARAMETER;
		}else
			Status = RemoveFileDiskDevice(DeviceObject->DriverObject, *(PULONG)pInput);
		break;
	case IOCTL_VHD_ENUM_DEVICE:
		Status = EnumDiskFiles(DeviceObject->DriverObject, pOut, &OutputBufferLength);
		if(Status == STATUS_SUCCESS)
			Irp->IoStatus.Information = OutputBufferLength;
		break;
	case IOCTL_VHD_SET_DATA:
		if(pInput == NULL || InputBufferLength < sizeof(SET_DISK_FILE_DATA) || InputBufferLength <= ((PSET_DISK_FILE_DATA)pInput)->DataSize)
			Status = STATUS_INVALID_PARAMETER;
		else
			Status = SetDiskFileData(DeviceObject->DriverObject, (PSET_DISK_FILE_DATA)pInput);
		break;
	case IOCTL_VHD_GET_DATA:
		if(pInput == NULL || InputBufferLength != sizeof(SET_DISK_FILE_DATA) || pOut == NULL || OutputBufferLength < ((PSET_DISK_FILE_DATA)pInput)->DataSize)
			Status = STATUS_INVALID_PARAMETER;
		else{
			Status = GetDiskFileData(DeviceObject->DriverObject, (PSET_DISK_FILE_DATA)pInput, pOut, &OutputBufferLength);
			if(NT_SUCCESS(Status))
				Irp->IoStatus.Information = OutputBufferLength;
		}
		break;
	case IOCTL_VHD_UNMOUNT:
		if(pInput == NULL || InputBufferLength < sizeof(ULONG)){
			Status = STATUS_INVALID_PARAMETER;
		}else
			Status = UmountFileDiskDevice(DeviceObject->DriverObject, *(PULONG)pInput);
		break;
		//--------------------------------------------------------------------------------------------------------------------------------------------
	default:
		Status = STATUS_UNSUCCESSFUL;
		break;
	}

	Irp->IoStatus.Status = Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;

}

