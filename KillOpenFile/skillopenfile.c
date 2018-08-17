#include <ntddk.h>  
  

#define NT_DEVICE_NAME              L"\\Device\\WiseUnlock"  
#define DOS_DEVICE_NAME             L"\\DosDevices\\WiseUnlock"  

#define IOCTL_DELETE_FILE \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0001, METHOD_BUFFERED, FILE_ANY_ACCESS)

NTSTATUS NTAPI VfatBuildRequest (PDEVICE_OBJECT DeviceObject,  
                                   PIRP Irp);  
VOID   
SKillUnloadDriver(   
    IN PDRIVER_OBJECT  DriverObject   
    )  
{  
    PDEVICE_OBJECT  deviceObject = DriverObject->DeviceObject;  
    UNICODE_STRING  uniSymLink;  
  
	DbgPrint("SKillUnloadDriver");

    RtlInitUnicodeString(&uniSymLink, DOS_DEVICE_NAME);  
      
    IoDeleteSymbolicLink(&uniSymLink);  
  
    IoDeleteDevice(deviceObject);  
}  
  
  
HANDLE  
SkillIoOpenFile(IN PCWSTR FileName,IN ACCESS_MASK DesiredAccess,IN ULONG ShareAccess)  
{  
    NTSTATUS            ntStatus;  
    UNICODE_STRING      uniFileName;  
    OBJECT_ATTRIBUTES   objectAttributes;  
    HANDLE              ntFileHandle;  
    IO_STATUS_BLOCK     ioStatus;  
  
    if (KeGetCurrentIrql() > PASSIVE_LEVEL)  
    {  
        return 0;  
    }  
  
    RtlInitUnicodeString(&uniFileName, FileName);  
  
    InitializeObjectAttributes(&objectAttributes, &uniFileName,  
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);  
  
    ntStatus = IoCreateFile(&ntFileHandle,  
        DesiredAccess,  
        &objectAttributes,  
        &ioStatus,  
        0,  
        FILE_ATTRIBUTE_NORMAL,  
        ShareAccess,  
        FILE_OPEN,  
        0,  
        NULL,  
        0,  
        0,  
        NULL,  
        IO_NO_PARAMETER_CHECKING);  
  
    if (!NT_SUCCESS(ntStatus))  
    {  
        return 0;  
    }  
  
    return ntFileHandle;  
}  
  
//回调函数  
NTSTATUS  
SkillSetFileCompletion(  
    IN PDEVICE_OBJECT DeviceObject,  
    IN PIRP Irp,  
    IN PVOID Context  
    )  
{  
    Irp->UserIosb->Status = Irp->IoStatus.Status;  
    Irp->UserIosb->Information = Irp->IoStatus.Information;  
  
    KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);  
  
    IoFreeIrp(Irp);//释放IRP  
  
    return STATUS_MORE_PROCESSING_REQUIRED;  
}  

//去掉只读属性   
BOOLEAN   
	SKillStripFileAttributes(   
	IN HANDLE    FileHandle   
	)   
{   
	NTSTATUS          ntStatus = STATUS_SUCCESS;   
	PFILE_OBJECT      fileObject;   
	PDEVICE_OBJECT    DeviceObject;   
	PIRP              Irp;   
	KEVENT            event;   
	FILE_BASIC_INFORMATION    FileInformation;   
	IO_STATUS_BLOCK ioStatus;   
	PIO_STACK_LOCATION irpSp;   

	ntStatus = ObReferenceObjectByHandle(FileHandle, DELETE, *IoFileObjectType,   
		KernelMode, &fileObject,  NULL);   

	if (!NT_SUCCESS(ntStatus))   
	{   
		return FALSE;   
	}   

	DeviceObject = IoGetRelatedDeviceObject(fileObject);   
	Irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);   

	if (Irp == NULL)   
	{   
		ObDereferenceObject(fileObject);   
		return FALSE;   
	}   

	KeInitializeEvent(&event, SynchronizationEvent, FALSE);   

	memset(&FileInformation,0,0x28);   

	FileInformation.FileAttributes = FILE_ATTRIBUTE_NORMAL;   
	Irp->AssociatedIrp.SystemBuffer = &FileInformation;   
	Irp->UserEvent = &event;   
	Irp->UserIosb = &ioStatus;   
	Irp->Tail.Overlay.OriginalFileObject = fileObject;   
	Irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();   
	Irp->RequestorMode = KernelMode;   

	irpSp = IoGetNextIrpStackLocation(Irp);   
	irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;   
	irpSp->DeviceObject = DeviceObject;   
	irpSp->FileObject = fileObject;   
	irpSp->Parameters.SetFile.Length = sizeof(FILE_BASIC_INFORMATION);   
	irpSp->Parameters.SetFile.FileInformationClass = FileBasicInformation;   
	irpSp->Parameters.SetFile.FileObject = fileObject;   

	IoSetCompletionRoutine(   
		Irp,   
		SkillSetFileCompletion,   
		&event,   
		TRUE,   
		TRUE,   
		TRUE);   

	IoCallDriver(DeviceObject, Irp);   

	KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, NULL);   

	ObDereferenceObject(fileObject);   

	return TRUE;   
}   
  
BOOLEAN SKillDeleteFile(IN HANDLE  FileHandle)  
{  
    NTSTATUS        ntStatus = STATUS_SUCCESS;  
    PFILE_OBJECT    fileObject;  
    PDEVICE_OBJECT  DeviceObject;  
    PIRP            Irp;  
    KEVENT          event;  
    FILE_DISPOSITION_INFORMATION  FileInformation;  
    IO_STATUS_BLOCK ioStatus;  
    PIO_STACK_LOCATION irpSp;  
    PSECTION_OBJECT_POINTERS pSectionObjectPointer;  
  
    ntStatus = ObReferenceObjectByHandle(FileHandle,  
        DELETE,  
        *IoFileObjectType,  
        KernelMode,  
        &fileObject,  
        NULL);//打开文件的设备对象  
      
    if (!NT_SUCCESS(ntStatus))  
    {  
        return FALSE;  
    }     
  
    DeviceObject = IoGetRelatedDeviceObject(fileObject);  //返回打开的对象指针  
    Irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);  //分配一个该设备对象的IRP  
  
    if (Irp == NULL)  
    {  
        ObDereferenceObject(fileObject);  
        return FALSE;  
    }  
  
    KeInitializeEvent(&event, SynchronizationEvent, FALSE);//初始化IRP的信号状态  
      
    FileInformation.DeleteFile = TRUE;  
  
    //设置IRP  
    Irp->AssociatedIrp.SystemBuffer = &FileInformation;  
    Irp->UserEvent = &event;  
    Irp->UserIosb = &ioStatus;  
    Irp->Tail.Overlay.OriginalFileObject = fileObject;  
    Irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();  
    Irp->RequestorMode = KernelMode;  
      
    irpSp = IoGetNextIrpStackLocation(Irp);  
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;  
    irpSp->DeviceObject = DeviceObject;  
    irpSp->FileObject = fileObject;  
    irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);  
    irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;  
    irpSp->Parameters.SetFile.FileObject = fileObject;  
  
    //设置完成IPR请求的回调函数  
    IoSetCompletionRoutine(  
            Irp,  
            SkillSetFileCompletion,  
            &event,  
            TRUE,  
            TRUE,  
            TRUE);  
    pSectionObjectPointer = fileObject->SectionObjectPointer;  
	pSectionObjectPointer->SharedCacheMap = 0;
    pSectionObjectPointer->ImageSectionObject = 0;  
    pSectionObjectPointer->DataSectionObject = 0;  
  
    IoCallDriver(DeviceObject, Irp);         //发送IRP  
  
    KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, NULL);  
  
    ObDereferenceObject(fileObject);          //关闭设备对象  
  
    return TRUE;  
}  
  
BOOLEAN SDeleteOpenFile(IN PCWSTR FileName)
{
	BOOLEAN result = FALSE;
	HANDLE hFileHandle;

	DbgPrint("FileName:%S\n",FileName);

	hFileHandle = SkillIoOpenFile(FileName,   
		FILE_READ_ATTRIBUTES,  
		FILE_SHARE_DELETE);  

	if (hFileHandle!=NULL)  
	{  
		result = SKillDeleteFile(hFileHandle);    //删除文件  
		ZwClose(hFileHandle);  
	}  
	return result;
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

	pIrp->IoStatus.Information = 0;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	switch(pIrpStack->MajorFunction)
	{
		case IRP_MJ_CREATE:
			//if(refCount != (ULONG)-1){refCount++;}
			status = STATUS_SUCCESS;
			break;
		case IRP_MJ_CLOSE:
			//if(refCount != (ULONG)-1){refCount--;}
			status = STATUS_SUCCESS;
			break;

		case IRP_MJ_DEVICE_CONTROL:
			//  Dispatch on IOCTL
			DbgPrint("IRP_MJ_DEVICE_CONTROL:%d\n",pIrpStack->Parameters.DeviceIoControl.IoControlCode);
			switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode)
			{
			case IOCTL_DELETE_FILE:

				ret = SDeleteOpenFile((PCWSTR)pIrp->AssociatedIrp.SystemBuffer);
				if(ret)
				{
					*(PBOOLEAN)pIrp->AssociatedIrp.SystemBuffer = ret;
					pIrp->IoStatus.Information = sizeof(BOOLEAN);
					status = STATUS_SUCCESS;
				}
				else
					status = STATUS_UNSUCCESSFUL;
				
				pIrp->IoStatus.Status = status;

				IoCompleteRequest(pIrp, IO_DISK_INCREMENT);
				return status;
			}
			break;
	}

	pIrp->IoStatus.Status = status;

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
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

	DbgPrint("DriverEntry:%S\n",uniDeviceName);

	//创建设备对象  
	ntStatus = IoCreateDevice(  
		DriverObject,  
		0,  
		&uniDeviceName,  
		FILE_DEVICE_UNKNOWN,  
		FILE_DEVICE_SECURE_OPEN,  
		FALSE,  
		&deviceObject);  

	if (!NT_SUCCESS(ntStatus))  
	{  
		return ntStatus;  
	}  

	DriverObject->MajorFunction[IRP_MJ_CREATE] = OlsDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = OlsDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OlsDispatch;
	DriverObject->DriverUnload = SKillUnloadDriver;  

	//设置设备对象的名称  
	ntStatus = IoCreateSymbolicLink(&uniSymLink, &uniDeviceName);  

	if (!NT_SUCCESS(ntStatus))  
	{  
		IoDeleteDevice(deviceObject);  
		return ntStatus;  
	}  


	//SDeleteOpenFile(L"\\??\\c:\\Project1.exe");

	return STATUS_SUCCESS;  
}  
