#include "DirverUtils.h"
#include "regfltr.h"

LPCWSTR GetTransactionNotifyClassString (__in ULONG TransactionNotifcation);

LPCWSTR GetNotifyClassString (__in REG_NOTIFY_CLASS NotifyClass);

PUNICODE_STRING GetRootKeyString(PVOID pRootObject);

LARGE_INTEGER g_Cookie = {0};

NTSTATUS WaitForUser(PKEVENT pUser, PKEVENT pKernel)
{
	LARGE_INTEGER TimeOut;
	LONG out;

	KeSetEvent(pUser, IO_NO_INCREMENT, FALSE);
	//ZwSetEvent(hUser,&out);

	TimeOut.QuadPart = -3*1000*1000*100;
	KeClearEvent(pKernel);
	return KeWaitForSingleObject(pKernel, Executive, KernelMode, FALSE, &TimeOut); 
}


NTSTATUS Callback (PVOID CallbackContext, PVOID Argument1, PVOID Argument2)
{

    NTSTATUS Status = STATUS_SUCCESS;
	PCALLBACK_CONTEXT pContext = (PCALLBACK_CONTEXT) CallbackContext;
    REG_NOTIFY_CLASS NotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;
	PVOID pRegObject = NULL;

	PUNICODE_STRING pCompleteName = NULL, pRootKeyName = NULL, pSubKeyName = NULL;
	PUNICODE_STRING pValueName = NULL, pNewName = NULL;
	PUNICODE_STRING pProcessName;

	HANDLE pid = NULL;
	CALLBACK_NOTIFY rn;
	ULONG positionId = 0;
	PFILR_NOTIFY_DATA pNotifyData = NULL;
	PNOTIFY_DATA_STACK pDataStack = NULL;
	
	switch(NotifyClass){
	case RegNtPreDeleteKey:
		pRegObject = ((PREG_DELETE_KEY_INFORMATION)Argument2)->Object;
		break;
	case RegNtPreSetValueKey:
		pRegObject = ((PREG_SET_VALUE_KEY_INFORMATION)Argument2)->Object;
		pValueName = ((PREG_SET_VALUE_KEY_INFORMATION)Argument2)->ValueName;
		break;
	case RegNtPreDeleteValueKey:
		pRegObject = ((PREG_DELETE_VALUE_KEY_INFORMATION)Argument2)->Object;
		pValueName = ((PREG_DELETE_VALUE_KEY_INFORMATION)Argument2)->ValueName;
		break;
	case RegNtPreSetInformationKey:
		pRegObject = ((PREG_SET_INFORMATION_KEY_INFORMATION)Argument2)->Object;
		break;
	case RegNtPreRenameKey:
		pRegObject = ((PREG_RENAME_KEY_INFORMATION)Argument2)->Object;
		pNewName = ((PREG_RENAME_KEY_INFORMATION)Argument2)->NewName;
		break;
	case RegNtPreCreateKey:
		pCompleteName = ((PREG_PRE_CREATE_KEY_INFORMATION)Argument2)->CompleteName;
		break;
	case RegNtPreCreateKeyEx:
		pRegObject = ((PREG_CREATE_KEY_INFORMATION)Argument2)->RootObject;
		pCompleteName = ((PREG_CREATE_KEY_INFORMATION)Argument2)->CompleteName;
		break;
	case RegNtPostCreateKey:
	case RegNtPostDeleteKey:
	case RegNtPostSetValueKey:
	case RegNtPostDeleteValueKey:
	case RegNtPostSetInformationKey:
	case RegNtPostRenameKey:
	case RegNtPostCreateKeyEx:
		//pRegObject = ((PREG_POST_OPERATION_INFORMATION)Argument2)->Object;
		return STATUS_SUCCESS;
		//break;
	default:
		return STATUS_SUCCESS;
	}

	if(pRegObject ==NULL )
		return STATUS_SUCCESS;

	do 
	{
		pid = PsGetCurrentProcessId();
		if((ULONG)pid == 0 || (ULONG)pid == 4)  //system, idl
			break;
		positionId = FindPositionIDByData(NotifyClass, pRegObject, pCompleteName, pValueName, &pRootKeyName, &pSubKeyName);
		if(positionId == 0)
			break;
		else{
			rn = IsExcludePosition(positionId);
			if(rn == RN_ALLOW)
				break;
			if(rn == RN_DENY){  //Ö±½Ó¾Ü¾ø
				Status = STATUS_ACCESS_DENIED;
				break;
			}
		}

		pProcessName = GetProcessImageFileName(pid);
		if(pProcessName == NULL || pProcessName->Length == 0)
			break;
		rn = IsExcludeFile(pProcessName->Buffer, pProcessName->Length);
		if(rn == RN_DENY){
			Status = STATUS_ACCESS_DENIED;
			break;
		}else if(rn == RN_ALLOW){
			Status = STATUS_SUCCESS;
			break;
		}

		pNotifyData = InitRegNotifyData(positionId, NotifyClass, pRootKeyName, pSubKeyName,  pValueName, pNewName, pProcessName);
		if(pNotifyData == NULL)
			break;
		pDataStack = PushCallbackNotifyData(pNotifyData);
		if(pDataStack==NULL)
		{
			break;
		}
		if(NT_SUCCESS(WaitForUser(pContext->UserEvent, &pNotifyData->hEvent))){
			if(pNotifyData->Data.Denied == RN_DENY){
				Status = STATUS_ACCESS_DENIED;
			}
		}else
			Status = STATUS_SUCCESS;

	} while (FALSE);

	if(pDataStack != NULL){
		RemoveWaitNotifyDataStck(pDataStack);
	}
	if(pNotifyData!=NULL)
		FreeNotifyData(pNotifyData);


	if(pRootKeyName!=NULL)
		ExFreePool(pRootKeyName);
	if(pSubKeyName!=NULL)
		ExFreePool(pSubKeyName);
		
	return Status;
    
}

NTSTATUS RegisterCallback(ULONG InputBufferLength, PREGISTER_CALLBACK_IN pInput)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
	//PCALLBACK_CONTEXT pContext = NULL;
	
	do 
	{
		if (pInput==NULL || InputBufferLength < sizeof(REGISTER_CALLBACK_IN)) {
				Status = STATUS_PARAMETER_QUOTA_EXCEEDED;
				break;
		}

		Status = InitCallbackContext((HANDLE)pInput->UserEvent);
		if (Status != STATUS_SUCCESS) {
			break;
		}

		if(g_Cookie.QuadPart == 0){
			Status = CmRegisterCallback(Callback, g_Context, &g_Cookie);
			if (!NT_SUCCESS(Status)) {
				break;
			}else
				Status = STATUS_SUCCESS;
		}

	} while (FALSE);


    return Status;
}

NTSTATUS UnRegisterCallback() 
{
	NTSTATUS Status = STATUS_SUCCESS;
	if(g_Cookie.QuadPart != 0){
		Status = CmUnRegisterCallback(g_Cookie);
		if(NT_SUCCESS(Status)){
			FreeCallbackContext();
			g_Cookie.QuadPart = 0;
		}
	}
	return Status;
}

NTSTATUS GetRegNotifyData(ULONG OutputBufferLength,PREGISTER_NOTIFY_DATA_OUT pOut, PULONG_PTR pRetlen) 
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PFILR_NOTIFY_DATA pData = NULL;

	do 
	{
		pData = PopCallbackNotifyData();
		if(pData == NULL){
			break;
		}

		if (OutputBufferLength < pData->Data.nSize){
			*pRetlen = pData->Data.nSize;
			Status = STATUS_FLT_BUFFER_TOO_SMALL;
			break;
		}
		
		RtlMoveMemory(pOut, &pData->Data, pData->Data.nSize);
		*pRetlen = pData->Data.nSize;

		Status = STATUS_SUCCESS;
	} while (FALSE);


	return Status;
}

NTSTATUS SetRegNotifyResult(ULONG InputBufferLength, PREGISTER_CALLBACK_RESULT_IN Input)
{
	NTSTATUS Status = STATUS_SUCCESS;	
	PCALLBACK_CONTEXT  CallbackCtx;
	PNOTIFY_DATA_STACK pStack = NULL;


	do 
	{
		if (InputBufferLength != sizeof(REGISTER_CALLBACK_RESULT_IN)) {
			Status = STATUS_PARAMETER_QUOTA_EXCEEDED;
			break;
		}

		SetWaitNotifyData(Input->DataCookie, Input->Denied);

	} while (FALSE);
		
	return Status;
}

NTSTATUS AddExcludePosition(ULONG Length, PEXCLUDE_POSITION_IN pInput)
{
	if(Length != sizeof(EXCLUDE_POSITION_IN) || pInput == NULL)
		return STATUS_INVALID_PARAMETER;
	return PushExcludePosition(pInput->positionId, pInput->Denied);
}
/*
NTSTATUS UnRegisterAll()
{
	NTSTATUS Status = STATUS_SUCCESS;

	if(g_Cookie.QuadPart != 0){
		Status = CmUnRegisterCallback(g_Cookie);
		if(Status == STATUS_SUCCESS){
			g_Cookie.QuadPart = 0;
		}
	}

	RemoveAllContextStack();

	return Status;
}
*/
NTSTATUS AddExcludeFile(ULONG Length, PEXCLUDE_ADD_IN pInput)
{
	if(pInput == NULL || Length < pInput->Namesize + sizeof(EXCLUDE_ADD_IN))
		return STATUS_UNSUCCESSFUL;
	return PushExcludeFile(pInput->Filename, pInput->Namesize, pInput->Denied);
}
NTSTATUS RemoveExcludeFile(ULONG Length, PEXCLUDE_ADD_IN pInput)
{
	if(pInput == NULL || Length < pInput->Namesize + sizeof(EXCLUDE_ADD_IN))
		return STATUS_UNSUCCESSFUL;
	return PopExcludeFile(pInput->Filename, pInput->Namesize);
}
/*
NTSTATUS GetCallbackVersion(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp) 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpStack;
    ULONG OutputBufferLength;
    PGET_CALLBACK_VERSION_OUTPUT GetCallbackVersionOutput;

    UNREFERENCED_PARAMETER(DeviceObject);

    //
    // Get the output buffer and verify its size
    //
    
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (OutputBufferLength < sizeof(GET_CALLBACK_VERSION_OUTPUT)) {
        Status = STATUS_BUFFER_OVERFLOW;
        goto Exit;
    }

    GetCallbackVersionOutput = (PGET_CALLBACK_VERSION_OUTPUT) Irp->AssociatedIrp.SystemBuffer;

    //
    // Call CmGetCallbackVersion and store the results in the output buffer
    //
    
    CmGetCallbackVersion(&GetCallbackVersionOutput->MajorVersion, 
                         &GetCallbackVersionOutput->MinorVersion);   

    Irp->IoStatus.Information = sizeof(GET_CALLBACK_VERSION_OUTPUT);

  Exit:

    if (Status != STATUS_SUCCESS) {
        ErrorPrint("GetCallbackVersion failed. Status 0x%x", Status);
    } else {
        InfoPrint("GetCallbackVersion succeeded");
    }

    return Status;
}
*/
/*
LPCWSTR GetNotifyClassString (
    __in REG_NOTIFY_CLASS NotifyClass)
{
    switch (NotifyClass) {
        case RegNtPreDeleteKey:                 return L"RegNtPreDeleteKey";
        case RegNtPreSetValueKey:               return L"RegNtPreSetValueKey";
        case RegNtPreDeleteValueKey:            return L"RegNtPreDeleteValueKey";
        case RegNtPreSetInformationKey:         return L"RegNtPreSetInformationKey";
        case RegNtPreRenameKey:                 return L"RegNtPreRenameKey";
        case RegNtPreEnumerateKey:              return L"RegNtPreEnumerateKey";
        case RegNtPreEnumerateValueKey:         return L"RegNtPreEnumerateValueKey";
        case RegNtPreQueryKey:                  return L"RegNtPreQueryKey";
        case RegNtPreQueryValueKey:             return L"RegNtPreQueryValueKey";
        case RegNtPreQueryMultipleValueKey:     return L"RegNtPreQueryMultipleValueKey";
        case RegNtPreKeyHandleClose:            return L"RegNtPreKeyHandleClose";
        case RegNtPreCreateKeyEx:               return L"RegNtPreCreateKeyEx";
        case RegNtPreOpenKeyEx:                 return L"RegNtPreOpenKeyEx";
        case RegNtPreFlushKey:                  return L"RegNtPreFlushKey";
        case RegNtPreLoadKey:                   return L"RegNtPreLoadKey";
        case RegNtPreUnLoadKey:                 return L"RegNtPreUnLoadKey";
        case RegNtPreQueryKeySecurity:          return L"RegNtPreQueryKeySecurity";
        case RegNtPreSetKeySecurity:            return L"RegNtPreSetKeySecurity";
        case RegNtPreRestoreKey:                return L"RegNtPreRestoreKey";
        case RegNtPreSaveKey:                   return L"RegNtPreSaveKey";
        case RegNtPreReplaceKey:                return L"RegNtPreReplaceKey";

        case RegNtPostDeleteKey:                return L"RegNtPostDeleteKey";
        case RegNtPostSetValueKey:              return L"RegNtPostSetValueKey";
        case RegNtPostDeleteValueKey:           return L"RegNtPostDeleteValueKey";
        case RegNtPostSetInformationKey:        return L"RegNtPostSetInformationKey";
        case RegNtPostRenameKey:                return L"RegNtPostRenameKey";
        case RegNtPostEnumerateKey:             return L"RegNtPostEnumerateKey";
        case RegNtPostEnumerateValueKey:        return L"RegNtPostEnumerateValueKey";
        case RegNtPostQueryKey:                 return L"RegNtPostQueryKey";
        case RegNtPostQueryValueKey:            return L"RegNtPostQueryValueKey";
        case RegNtPostQueryMultipleValueKey:    return L"RegNtPostQueryMultipleValueKey";
        case RegNtPostKeyHandleClose:           return L"RegNtPostKeyHandleClose";
        case RegNtPostCreateKeyEx:              return L"RegNtPostCreateKeyEx";
        case RegNtPostOpenKeyEx:                return L"RegNtPostOpenKeyEx";
        case RegNtPostFlushKey:                 return L"RegNtPostFlushKey";
        case RegNtPostLoadKey:                  return L"RegNtPostLoadKey";
        case RegNtPostUnLoadKey:                return L"RegNtPostUnLoadKey";
        case RegNtPostQueryKeySecurity:         return L"RegNtPostQueryKeySecurity";
        case RegNtPostSetKeySecurity:           return L"RegNtPostSetKeySecurity";
        case RegNtPostRestoreKey:               return L"RegNtPostRestoreKey";
        case RegNtPostSaveKey:                  return L"RegNtPostSaveKey";
        case RegNtPostReplaceKey:               return L"RegNtPostReplaceKey";

        case RegNtCallbackObjectContextCleanup: return L"RegNtCallbackObjectContextCleanup";

        default:
            return L"Unsupported REG_NOTIFY_CLASS";
    }
}


LPCWSTR GetTransactionNotifyClassString (
    __in ULONG TransactionNotifcation)
{
    switch (TransactionNotifcation) {
        case TRANSACTION_NOTIFY_COMMIT:         return L"TRANSACTION_NOTIFY_COMMIT";
        case TRANSACTION_NOTIFY_ROLLBACK:       return L"TRANSACTION_NOTIFY_ROLLBACK";
        
        default:
            return L"Unsupported Transaction Notification";
    }
}
*/