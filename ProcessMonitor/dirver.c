#include <ntddk.h>  
#include "Monitor.h"
#include "DelFileEx.h"

#ifdef _ANTI_
#define NT_DEVICE_NAME              L"\\Device\\WiseAntiProcess"
#define DOS_DEVICE_NAME             L"\\DosDevices\\WiseAntiProcess"
#else
#define NT_DEVICE_NAME              L"\\Device\\WiseProcessMonitor"
#define DOS_DEVICE_NAME             L"\\DosDevices\\WiseProcessMonitor"
#endif 

VOID   
SKillUnloadDriver(   
    IN PDRIVER_OBJECT  DriverObject   
    )  
{  
    PDEVICE_OBJECT  deviceObject = DriverObject->DeviceObject;  
    UNICODE_STRING  uniSymLink;  
  
	DbgPrint("SKillUnloadDriver");

	FreeGlobalData();

    RtlInitUnicodeString(&uniSymLink, DOS_DEVICE_NAME);  
      
    IoDeleteSymbolicLink(&uniSymLink);  
  
    IoDeleteDevice(deviceObject);  
}  
 
NTSTATUS
OlsDispatch(
	IN	PDEVICE_OBJECT pDO,
	IN	PIRP pIrp
	)
{
	PIO_STACK_LOCATION pIrpStack;
	NTSTATUS status = STATUS_NOT_IMPLEMENTED;
	BOOLEAN ret;
	ULONG InputBufferLength, OutputBufferLength;
	PVOID pInput, pOut;

	pIrp->IoStatus.Information = 0;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	InputBufferLength  = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	pInput = pIrp->AssociatedIrp.SystemBuffer;

	OutputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	pOut = pIrp->AssociatedIrp.SystemBuffer;
	
	switch(pIrpStack->MajorFunction)
	{
		case IRP_MJ_CREATE:
			status = STATUS_SUCCESS;
			break;
		case IRP_MJ_CLOSE:
			status = STATUS_SUCCESS;
			break;

		case IRP_MJ_DEVICE_CONTROL:
			//  Dispatch on IOCTL
			switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode)
			{
			case IOCTL_ADD_MAL_FILE:
				DbgPrint("IOCTL_ADD_MAL_FILE");
				status = AddMalFileName(InputBufferLength, (PMAL_FILE_NAME)pInput);
				break;
			case IOCTL_REOME_MAL_FILE:
				DbgPrint("IOCTL_REOME_MAL_FILE");
				status = RemoveMalFileName(InputBufferLength, (PMAL_FILE_NAME)pInput);
				break;
			case IOCTL_SET_PROCESS_NOTIFY:
				DbgPrint("IOCTL_SET_PROCESS_NOTIFY");
				status = SetProcessNotify(InputBufferLength, (PREGISTER_CALLBACK_IN)pInput);
				break;
			case IOCTL_REMOVE_PROCESS_NOTIFY:
				DbgPrint("IOCTL_REMOVE_PROCESS_NOTIFY");
				status = RemoveProcessNotify();
				break;
			case IOCTL_GET_NOTIFY_DATA:
				DbgPrint("IOCTL_GET_NOTIFY_DATA");
				status = GetRegNotifyData(OutputBufferLength, (PPROC_NOTIFY_DATA_OUT)pOut, &pIrp->IoStatus.Information);
				break;
			case IOCTL_SET_NOTIFY_RESULT:
				DbgPrint("IOCTL_SET_NOTIFY_RESULT");
				status = SetRegNotifyResult(InputBufferLength, (PPROC_NOTIFY_DATA_IN)pInput);
				break;
			case IOCTL_DELETE_FILE:
				ret = SDeleteOpenFile((PCWSTR)pInput);
				if(ret)
				{
					*(PBOOLEAN)pOut = ret;
					pIrp->IoStatus.Information = sizeof(BOOLEAN);
					status = STATUS_SUCCESS;
				}
				else
					status = STATUS_UNSUCCESSFUL;
				break;
			case IOCTL_KILL_PROCESS:
				status = KillProcess(*(PULONG)pInput);
				break;
			}
	}

	pIrp->IoStatus.Status = status;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS
	DeviceCleanup (
	__in PDEVICE_OBJECT DeviceObject,
	__in PIRP Irp
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	RemoveProcessNotify();

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(  
	IN PDRIVER_OBJECT DriverObject,  
	IN PUNICODE_STRING RegistryPath  
	)  
{  
	UNICODE_STRING                uniDeviceName;  
	UNICODE_STRING                uniSymLink;  
	NTSTATUS                        ntStatus;  
	PDEVICE_OBJECT                deviceObject = NULL;  

	RtlInitUnicodeString(&uniDeviceName, NT_DEVICE_NAME);  
	RtlInitUnicodeString(&uniSymLink, DOS_DEVICE_NAME);  

	//创建设备对象  
	ntStatus = IoCreateDevice(  
		DriverObject,  
		0,  
		&uniDeviceName,  
		FILE_DEVICE_UNKNOWN,  
		FILE_DEVICE_SECURE_OPEN,  
		FALSE,  
		&deviceObject);

	//DriverObject->Flags |= 0x20;

	if (NT_SUCCESS(ntStatus))  
	{  
		DriverObject->MajorFunction[IRP_MJ_CREATE] = OlsDispatch;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = OlsDispatch;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DeviceCleanup;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OlsDispatch;
		DriverObject->DriverUnload = SKillUnloadDriver;  

		ntStatus = IoCreateSymbolicLink(&uniSymLink, &uniDeviceName);  
		if (!NT_SUCCESS(ntStatus))  
		{  
			IoDeleteDevice(deviceObject);  
		}  
	}

	ntStatus = InitGlobalData();

	DbgPrint("DriverEntry:%wZ status = %x \n",&uniDeviceName, ntStatus);

	return ntStatus;  
}  
