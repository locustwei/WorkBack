/*++

--*/

#include "WiseFs.h"
#include "HideFile.h"

//全局变量
WFS_GLOBAL_DATA Globals={0};


DRIVER_INITIALIZE DriverEntry;
//入口
NTSTATUS DriverEntry (__in PDRIVER_OBJECT DriverObject,__in PUNICODE_STRING RegistryPath);
//卸载回调
NTSTATUS Unload (__in FLT_FILTER_UNLOAD_FLAGS Flags);
//在每个卷安装实例回调
NTSTATUS InstanceSetup (__in PCFLT_RELATED_OBJECTS FltObjects,__in FLT_INSTANCE_SETUP_FLAGS Flags,__in DEVICE_TYPE VolumeDeviceType,__in FLT_FILESYSTEM_TYPE VolumeFilesystemType);

//文件读取回调函数
//FLT_PREOP_CALLBACK_STATUS PreReadCallBack (__inout PFLT_CALLBACK_DATA Data,__in PCFLT_RELATED_OBJECTS FltObjects,__deref_out_opt PVOID *CompletionContext);

/*
有时，驱动程序的某些部分必须驻留内存而另一些可以被分页，这就需要一种能控制代码和数据是否分页的方法。通过指导编译器的段分配可以实现这个目的。在运行时，装入器通过检查驱动程序中的段名把段放到你指定的内存池中。此外在运行时调用内存管理器的例程也能实现这个目的。
要注意，绝不能让可能在高的IRQL级别被调用的例程被换出页面。
*/
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Unload)
#pragma alloc_text(PAGE, InstanceSetup)

/*
#pragma alloc_text(PAGE, PostDirCtrlCallBack)
#pragma alloc_text(INIT, CreateUserPort)
#pragma alloc_text(PAGE, PortConnect)
#pragma alloc_text(PAGE, PortDisconnect)
#pragma alloc_text(PAGE, PortMessage)
*/
#endif

ULONG WIN_VER;

ULONG g_WiseProcessId = 0;

//
//  Filters callback routines
//

FLT_OPERATION_REGISTRATION Callbacks[] = {
	/*
	{ 
		IRP_MJ_READ,                                     //MajorFunction
		FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO,     //Flags
		PreReadCallBack,                                         //PreOperation
		NULL,                                            //PostOperation
		NULL                                             //Reserved1
	},
	*/	
	{
		IRP_MJ_DIRECTORY_CONTROL,
		0,
		NULL,
		PostDirCtrlCallBack,
		NULL
	},
	/*
	{
		IRP_MJ_SET_INFORMATION,
		FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO,
		PreFileDiskSetInformation,
		NULL,
		NULL
	},
	*/
	{ IRP_MJ_OPERATION_END }
};

//
// Filters registration data structure
//

FLT_REGISTRATION FilterRegistration = {

	sizeof( FLT_REGISTRATION ),             //  Size
	FLT_REGISTRATION_VERSION,               //  Version
	0,              //  Flags。 FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP不能停止，FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS支持 named pipe and mailslot requests.    
	NULL,                                   //  Context 上下文数组The last element in the array must be {FLT_CONTEXT_END}.
	Callbacks,                              //  IRP回调函数数组The last element in the array must be {IRP_MJ_OPERATION_END}.
	Unload,                                 //  Filters unload routine
	InstanceSetup,                          //  InstanceSetup routine
	NULL,                  //  InstanceQueryTeardown routine
	NULL,                  //  InstanceTeardownStart routine
	NULL,               //  InstanceTeardownComplete routine
	NULL,                                   //GenerateFileNameCallback 
	NULL,                                   //NormalizeNameComponentCallback
	NULL                                    //  NormalizeContextCleanupCallback
	//TransactionNotificationCallback (Windows Vista and later only.)   
	//NormalizeNameComponentExCallback
	//SectionNotificationCallback
};

PDEVICE_OBJECT g_DeviceObject = NULL;

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

	//g_WiseProcessId = 0;

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
	case IOCTL_SET_EXCLUDE_HANDLE:
		g_WiseProcessId = *(PULONG)pInput;
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

	RtlInitUnicodeString(&DosDevicesLinkName, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&DosDevicesLinkName);


	//IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS CreateDosDriver(__in PDRIVER_OBJECT DriverObject)
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
			&g_DeviceObject);
		if (!NT_SUCCESS (status))
			break;

		RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);
		status = IoCreateSymbolicLink(&win32DeviceName, &ntDeviceName);
		if (!NT_SUCCESS(status))
			break;

		fSymbolicLink = TRUE;

		DriverObject->MajorFunction[IRP_MJ_CREATE]         = DeviceCreate;
		DriverObject->MajorFunction[IRP_MJ_CLOSE]          = DeviceClose;
		DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = DeviceCleanup;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
		DriverObject->DriverUnload                         = DeviceUnload;
		//InitRule();

	} while (FALSE);

	if(!NT_SUCCESS(status)){

		if(fSymbolicLink)
			IoDeleteSymbolicLink(&win32DeviceName);

		if(g_DeviceObject!=NULL)
			IoDeleteDevice(g_DeviceObject);
	}
	return status;
}
//  Filter driver initialization and unload routines
NTSTATUS DriverEntry (__in PDRIVER_OBJECT DriverObject, __in PUNICODE_STRING RegistryPath)
{
	NTSTATUS Status;
	RTL_OSVERSIONINFOEXW VersionInformation={sizeof(RTL_OSVERSIONINFOEXW)};

	Status = RtlGetVersion((PRTL_OSVERSIONINFOW)&VersionInformation);
	if(!NT_SUCCESS(Status))
		return Status;
	WIN_VER = VersionInformation.dwMajorVersion;
	//注册驱动
	Status = FltRegisterFilter( DriverObject,&FilterRegistration,&Globals.FilterHandle );

	if (!NT_SUCCESS( Status )) {
		return Status;
	}

	//用户通信端口
	//Status = CreateUserPort(Globals.FilterHandle);

	Status = FltStartFiltering( Globals.FilterHandle );   //驱动开始工作

	if (!NT_SUCCESS( Status )) {

		goto lbError;
	}

	CreateDosDriver(DriverObject);

	goto lbExit;

lbError:
	
	if(Globals.FilterHandle)
		FltUnregisterFilter( Globals.FilterHandle );
	Globals.FilterHandle = NULL;

lbExit:

	return Status;
}

NTSTATUS Unload (__in FLT_FILTER_UNLOAD_FLAGS Flags)
{
	UNREFERENCED_PARAMETER( Flags );  //无意义操作只是为消除警告信息

	PAGED_CODE();  //确保调用线程运行在一个允许分页的足够低IRQL级别。（仅Debug时有意义）

	//CloseUserProt();
	if(Globals.FilterHandle)
		FltUnregisterFilter( Globals.FilterHandle );

	Globals.FilterHandle = NULL;

	if(g_DeviceObject)
		IoDeleteDevice(g_DeviceObject);

	g_DeviceObject = NULL;

	return STATUS_SUCCESS;
}

//  新加Volume安装实例.
NTSTATUS 
	InstanceSetup (__in PCFLT_RELATED_OBJECTS FltObjects,__in FLT_INSTANCE_SETUP_FLAGS Flags,__in DEVICE_TYPE VolumeDeviceType,__in FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER( Flags );
	UNREFERENCED_PARAMETER( VolumeDeviceType );
	UNREFERENCED_PARAMETER( VolumeFilesystemType );

	PAGED_CODE();

	if (VolumeDeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM) {

		return STATUS_FLT_DO_NOT_ATTACH;
	}
	 
	return STATUS_SUCCESS;

}
/*
//Handle pre-read.
FLT_PREOP_CALLBACK_STATUS PreReadCallBack (__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
	)
{

	PFLT_FILE_NAME_INFORMATION NameInfo = NULL;
	NTSTATUS CbStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER( CompletionContext );

	if ((Data->Iopb->IrpFlags & IRP_PAGING_IO) || (Data->Iopb->IrpFlags & IRP_SYNCHRONOUS_PAGING_IO) || IoGetTopLevelIrp()) {
			return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	Status = FltGetFileNameInformation( Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &NameInfo );

	if (!NT_SUCCESS( Status )) {

		goto lbExit;
	}
	 
	Status = FltParseFileNameInformation( NameInfo );

	if (!NT_SUCCESS( Status )) {

		goto lbExit;

	}

	if (!FLT_IS_IRP_OPERATION( Data )) {

		CbStatus = FLT_PREOP_DISALLOW_FASTIO;
		goto lbExit;
	}

	CbStatus = STATUS_MEDIA_WRITE_PROTECTED;  //返回错误不让读取文件

lbExit:

	if (NameInfo) {

		FltReleaseFileNameInformation( NameInfo );
	}

	return CbStatus;
}
*/