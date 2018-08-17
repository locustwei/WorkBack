
#include "DirverUtils.h"
#include "regfltr.h"


DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD     DeviceUnload;

__drv_dispatchType(IRP_MJ_CREATE)         DRIVER_DISPATCH DeviceCreate;
__drv_dispatchType(IRP_MJ_CLOSE)          DRIVER_DISPATCH DeviceClose;
__drv_dispatchType(IRP_MJ_CLEANUP)        DRIVER_DISPATCH DeviceCleanup;
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DeviceControl;

//
// Pointer to the device object used to register registry callbacks
//
PDEVICE_OBJECT g_DeviceObj = NULL;

//
// Registry callback version
//
ULONG g_MajorVersion;
ULONG g_MinorVersion;

NTSTATUS DriverEntry (
    __in PDRIVER_OBJECT  DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING NtDeviceName;
    UNICODE_STRING DosDevicesLinkName;

    UNREFERENCED_PARAMETER(RegistryPath);

	InfoPrint("WiseRegNotify starting");

	do 
	{
		RtlInitUnicodeString(&NtDeviceName, NT_DEVICE_NAME);

		Status = IoCreateDevice(DriverObject,                 // pointer to driver object
			0,                            // device extension size
			&NtDeviceName,                // device name
			FILE_DEVICE_UNKNOWN,          // device type
			0,                            // device characteristics
			FALSE,                        // not exclusive
			&g_DeviceObj);                     // returned device object pointer

		if (!NT_SUCCESS(Status)) {
			break;
		}

		//
		// Set dispatch routines.
		//
		DriverObject->MajorFunction[IRP_MJ_CREATE]         = DeviceCreate;
		DriverObject->MajorFunction[IRP_MJ_CLOSE]          = DeviceClose;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DeviceCleanup;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
		DriverObject->DriverUnload                         = DeviceUnload;

		//
		// Create a link in the Win32 namespace.
		//

		RtlInitUnicodeString(&DosDevicesLinkName, DOS_DEVICES_LINK_NAME);

		Status = IoCreateSymbolicLink(&DosDevicesLinkName, &NtDeviceName);

		if (!NT_SUCCESS(Status)) {
			IoDeleteDevice(DriverObject->DeviceObject);
			break;
		}

		Status = InitGlobalData();
		if(Status != STATUS_SUCCESS)
			break;

	} while (FALSE);

    //CmGetCallbackVersion(&g_MajorVersion, &g_MinorVersion);

    return Status;
    
}



NTSTATUS
DeviceCreate (
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}



NTSTATUS
DeviceClose (
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}



NTSTATUS
DeviceCleanup (
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);

	UnRegisterProcess(PsGetCurrentProcessId());

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
    case IOCTL_REGISTER_CALLBACK:
        Status = RegisterCallback(InputBufferLength, (PREGISTER_CALLBACK_IN)pInput);
        break;

    case IOCTL_UNREGISTER_CALLBACK:
        Status = UnRegisterCallback();
        break;
	case IOCTL_GET_CONTEXT_DATA:
		Status = GetRegNotifyData(OutputBufferLength, (PREGISTER_NOTIFY_DATA_OUT)pOut, &Irp->IoStatus.Information);
		break;
	case IOCTL_SET_NOTIFY_RESULT:
		Status = SetRegNotifyResult(InputBufferLength, (PREGISTER_CALLBACK_RESULT_IN)pInput);
		break;
	case IOCTL_ADD_EXCLUDE:
		Status = AddExcludeFile(InputBufferLength, (PEXCLUDE_ADD_IN)pInput);
		break;
	case IOCTL_REMOVE_EXCLUDE:
		Status = RemoveExcludeFile(InputBufferLength, (PEXCLUDE_ADD_IN)pInput);
		break;
	case IOCTL_CHANGE_CALLBACK:
		Status = AddExcludePosition(InputBufferLength, (PEXCLUDE_POSITION_IN)pInput);
		break;
	case IOCTL_GETFILTER_POSITIONS:
		Status = GetPositions(OutputBufferLength, (PGET_POSITION_OUT)pOut, &Irp->IoStatus.Information);
		break;
	case IOCTL_GETEXCLUDE_FILES:
		Status = GetExcludeFiles(OutputBufferLength, (PGET_EXCLUDE_FILE_OUT)pOut, &Irp->IoStatus.Information);
		break;
	case IOCTL_SETFILTER_POSITIONS:
		Status = SetPositions(InputBufferLength, pInput);
		break;
    default:
        //ErrorPrint("Unrecognized ioctl code 0x%x", Ioctl);
		break;
    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
    
}


VOID
DeviceUnload (
    __in PDRIVER_OBJECT DriverObject
    )
{
    UNICODE_STRING  DosDevicesLinkName;
	
	UnRegisterCallback();
	FreeGlobalData();

    RtlInitUnicodeString(&DosDevicesLinkName, DOS_DEVICES_LINK_NAME);
    IoDeleteSymbolicLink(&DosDevicesLinkName);


    IoDeleteDevice(DriverObject->DeviceObject);
}


