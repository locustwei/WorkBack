#include "DirverUtils.h"

#define MAX_PATH                  260

typedef NTSTATUS QUERY_INFO_PROCESS(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	UINT32 ProcessInformationLength,
	PUINT32 ReturnLength
	);

static QUERY_INFO_PROCESS* _ZwQueryInformationProcess = NULL;

NTSTATUS ZwQueryInformationProcess(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	UINT32 ProcessInformationLength,
	PUINT32 ReturnLength
	)
{
	NTSTATUS Status;

	do 
	{
		if (PASSIVE_LEVEL != KeGetCurrentIrql()){  
			Status = STATUS_INFO_LENGTH_MISMATCH;
			break;
		}

		if (NULL == _ZwQueryInformationProcess){   
			UNICODE_STRING routineName;   
			RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");   

			_ZwQueryInformationProcess = (QUERY_INFO_PROCESS*)MmGetSystemRoutineAddress(&routineName);   

			if (NULL == _ZwQueryInformationProcess){   
				Status = STATUS_NOT_FOUND;
				break;
			}   
		}   

		Status = _ZwQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

	} while (FALSE);

	return Status;
}

PUNICODE_STRING GetProcessImageFileName(HANDLE pid)
{
	NTSTATUS            Status;
	PUNICODE_STRING     pName = NULL;   //进程名
	UINT32               nLen;

	HANDLE              hProcess = NULL;
	PEPROCESS           hObj = NULL;  

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return NULL;
	
	do 
	{
		Status = PsLookupProcessByProcessId(pid, &hObj);
		if(Status != STATUS_SUCCESS||hObj==NULL)
			break;
		
		Status = ObOpenObjectByPointer(hObj, OBJ_KERNEL_HANDLE, NULL, 0, *PsProcessType, KernelMode, &hProcess);
		if(Status != STATUS_SUCCESS)
			break;

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, NULL, 0, &nLen);
		if(Status != STATUS_INFO_LENGTH_MISMATCH){
			break;
		}

		nLen += sizeof(WCHAR);
		pName = (PUNICODE_STRING)MM_ALLOC(nLen);
		if(pName==NULL){
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlZeroMemory(pName, nLen);

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, pName, nLen, &nLen);

	} while (FALSE);

	if(Status != STATUS_SUCCESS){
		if(pName != NULL){
			MM_FREE(pName);
			pName = NULL;
		}
	}
	if(hProcess)
		ZwClose(hProcess);
	if(hObj)
		ObDereferenceObject(hObj);

	if(pName && (pName->Length == 0 || pName->Buffer == NULL)){ //没文件名
		MM_FREE(pName);
		pName = NULL;
	}

	return pName;
}
/*
PUNICODE_STRING GetProcessCmdLine(HANDLE pid)
{
	NTSTATUS            Status;
	PUNICODE_STRING     pName = NULL;   //进程名
	UINT32               nLen;
	PPROCESS_BASIC_INFORMATION pbi = NULL;

	HANDLE              hProcess = NULL;
	PEPROCESS           hObj = NULL;  

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return NULL;

	do 
	{
		Status = PsLookupProcessByProcessId(pid, &hObj);
		if(Status != STATUS_SUCCESS||hObj==NULL)
			break;

		Status = ObOpenObjectByPointer(hObj, OBJ_KERNEL_HANDLE, NULL, 0, *PsProcessType, KernelMode, &hProcess);
		if(Status != STATUS_SUCCESS)
			break;

		Status = ZwQueryInformationProcess(hProcess, ProcessBasicInformation, NULL, 0, &nLen);
		if(Status != STATUS_INFO_LENGTH_MISMATCH){
			break;
		}

		pbi = (PPROCESS_BASIC_INFORMATION)MM_ALLOC(nLen);

		Status = ZwQueryInformationProcess(hProcess, ProcessBasicInformation, pbi, nLen, &nLen);
		if(!NT_SUCCESS(Status))
			break;

		pName = (PUNICODE_STRING)MM_ALLOC(pbi->PebBaseAddress->ProcessParameters->CommandLine.Length + sizeof(UNICODE_STRING));
		if(pName==NULL){
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlZeroMemory(pName, nLen);
		pName->Length = pbi->PebBaseAddress->ProcessParameters->CommandLine.Length;
		pName->MaximumLength = pbi->PebBaseAddress->ProcessParameters->CommandLine.MaximumLength;
		RtlMoveMemory(pName->Buffer, pbi->PebBaseAddress->ProcessParameters->CommandLine.Buffer);
	} while (FALSE);

	if(pbi)
		MM_FREE(pbi);

	if(Status != STATUS_SUCCESS){
		if(pName != NULL){
			MM_FREE(pName);
			pName = NULL;
		}
	}
	if(hProcess)
		ZwClose(hProcess);
	if(hObj)
		ObDereferenceObject(hObj);

	if(pName && (pName->Length == 0 || pName->Buffer == NULL)){ //没文件名
		MM_FREE(pName);
		pName = NULL;
	}

	return pName;
}
*/
VOID FreeUtilsMemory(PVOID pMemory)
{
	if(pMemory)
		MM_FREE(pMemory);
}

PUNICODE_STRING GetObjectNameString(PVOID pObject)
{
	NTSTATUS status; 
	UINT32 returnedLength; 
	PUNICODE_STRING pName = NULL;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return NULL;
	//判断 object 的有效性 
	if(pObject == NULL ) { 
		return NULL; 
	} 
	
	do 
	{
		status = ObQueryNameString(pObject, NULL, 0, &returnedLength ); 
		if(status == STATUS_INFO_LENGTH_MISMATCH){ 
			returnedLength += sizeof(WCHAR);

			pName = (PUNICODE_STRING)MM_ALLOC(returnedLength); 
			if ( pName == NULL ) 
				break;
			RtlZeroMemory(pName, returnedLength);
			pName->MaximumLength = returnedLength;
			status = ObQueryNameString(pObject, (POBJECT_NAME_INFORMATION)pName, returnedLength, &returnedLength ); 
		} 
	} while (FALSE);

	if(status != STATUS_SUCCESS){
		if(pName!=NULL){
			MM_FREE(pName);
			pName = NULL;
		}
	}

	return pName; 
}

BOOLEAN CopyUnicodeString(PUNICODE_STRING pDest, PUNICODE_STRING pSource)
{
	if(pDest == NULL || pSource == NULL)
		return FALSE;
	pDest->Length = pSource->Length;
	pDest->Buffer = (PWCH)((PCHAR)pDest + sizeof(UNICODE_STRING));
	RtlMoveMemory(pDest->Buffer, pSource->Buffer, pSource->Length);

	return TRUE;
}

BOOLEAN UnicodeStringCopyString(PUNICODE_STRING pDest, PWCH pSource, UINT32 nMax)
{
	UINT32 Length;

	if(pDest == NULL || pSource == NULL)
		return FALSE;

	Length = wcslen(pSource);
	pDest->Length = Length * sizeof(WCHAR);
	pDest->Buffer = (PWCH)((PCHAR)pDest + sizeof(UNICODE_STRING));
	RtlMoveMemory(pDest->Buffer, pSource, pDest->Length);

	return TRUE;
}

BOOLEAN UnicodeStringCatString(PUNICODE_STRING pDest, PWCH pSource, UINT32 nMax)
{
	UINT32 Length;

	if(pDest == NULL || pSource == NULL)
		return FALSE;

	Length = wcslen(pSource);//, nMax); //wcsnlen 不支持XP
	if(Length == 0)
		return TRUE;

	Length = Length * sizeof(WCHAR);
	RtlMoveMemory((PCHAR)pDest->Buffer + pDest->Length, pSource, Length);
	pDest->Length += Length;
	return TRUE;
}

BOOLEAN UnicodeStringCat(PUNICODE_STRING pDest, PUNICODE_STRING pSource)
{
	if(pDest == NULL || pSource == NULL)
		return FALSE;

	RtlMoveMemory((PWCH)((PCHAR)pDest->Buffer + pDest->Length), pSource->Buffer, pSource->Length);
	pDest->Length += pSource->Length;
	return TRUE;
}

NTSTATUS CreateKey(PWCH pKeyName, PHANDLE hKey)
{
	UNICODE_STRING Name = {0};
	OBJECT_ATTRIBUTES keyAttr;
	
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	RtlInitUnicodeString(&Name, pKeyName);

	InitializeObjectAttributes(&keyAttr, &Name, OBJ_KERNEL_HANDLE, NULL, NULL);
	return ZwCreateKey(hKey, KEY_ALL_ACCESS, &keyAttr, 0, NULL, 0, NULL);

}
//创建注册表键
//"\\REGISTRY\\MACHINE\\SOFTWARE\\WiseCleaner\\WiseCare365\\RegFilter\\ExcludeFiles"
NTSTATUS CreateRegKey(LPWSTR KeyName, HANDLE* hKey)
{
	NTSTATUS Status = STATUS_INVALID_PARAMETER;
	HANDLE hTmp = NULL;
	UINT32 i, k, length, created = 0;
	UNICODE_STRING pName = {0};
	OBJECT_ATTRIBUTES keyAttr;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	do 
	{
		if(KeyName == NULL)
			break;
		k = wcslen(KeyName);

		for(i=k-1; i>0; i--){
			if(*(KeyName+i) == '\\'){
				if(pName.Buffer != NULL){
					MM_FREE(pName.Buffer);
					pName.Buffer = NULL;
				}
				length = (i + 1)* sizeof(WCHAR);
				pName.Buffer = (PWCH)MM_ALLOC(length);
				if(pName.Buffer == NULL)
					break;
				RtlZeroMemory(pName.Buffer, length);
				pName.Length = length - sizeof(WCHAR);
				pName.MaximumLength = length;
				RtlMoveMemory(pName.Buffer, KeyName, pName.Length);
				Status = RtlCheckRegistryKey(RTL_REGISTRY_ABSOLUTE, pName.Buffer);
				if(Status == STATUS_SUCCESS){
					created = i;
					break;
				}
			}
		}

		for(i=created + 1; i<k; i++){
			if(*(KeyName+i) == '\\'){
				if(pName.Buffer != NULL){
					MM_FREE(pName.Buffer);
					pName.Buffer = NULL;
				}
				length = (i + 1)* sizeof(WCHAR);
				pName.Buffer = (PWCH)MM_ALLOC(length);
				if(pName.Buffer == NULL)
					break;
				RtlZeroMemory(pName.Buffer, length);
				pName.Length = length - sizeof(WCHAR);
				pName.MaximumLength = length;
				RtlMoveMemory(pName.Buffer, KeyName, pName.Length);
				Status = CreateKey(pName.Buffer, &hTmp);
				if(Status != STATUS_SUCCESS)
					break;
				ZwClose(hTmp);
			}
		}

		Status = CreateKey(KeyName, hKey);
	} while (FALSE);

	if(pName.Buffer != NULL)
		MM_FREE(pName.Buffer);
	return Status;
}

NTSTATUS ClearRegisterKeyValues(HANDLE hKey)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PKEY_VALUE_BASIC_INFORMATION pBaskInfo = NULL;
	UNICODE_STRING ValueName;
	UINT32 ret = 0;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	while(TRUE){
		ret = 0;
		if(pBaskInfo!=NULL){
			MM_FREE(pBaskInfo);
			pBaskInfo = NULL;
		}
		Status = ZwEnumerateValueKey(hKey, 0, KeyValueBasicInformation, NULL, 0, &ret);
		if(Status != STATUS_BUFFER_TOO_SMALL){
			break;
		}
		ret += sizeof(WCHAR);
		pBaskInfo = (PKEY_VALUE_BASIC_INFORMATION)MM_ALLOC(ret);
		if(pBaskInfo==NULL)
			break;
		RtlZeroMemory(pBaskInfo, ret);
		Status = ZwEnumerateValueKey(hKey, 0, KeyValueBasicInformation, pBaskInfo, ret, &ret);
		if(Status != STATUS_SUCCESS){
			break;
		}
		RtlInitUnicodeString(&ValueName, pBaskInfo->Name);
		ZwDeleteValueKey(hKey, &ValueName);
	}

	ZwFlushKey(hKey);

	return Status;
}

VOID EnumerateRegValueKey(PWSTR KeyName, KEY_VALUE_INFORMATION_CLASS infoClass, EnumerateRegValueKeyCallback Callback)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PVOID pValueInfo = NULL;
	UINT32 ret = 0, idx = 0;

	HANDLE hKey = NULL;

	UNICODE_STRING Name = {0};
	OBJECT_ATTRIBUTES keyAttr;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return;

	RtlInitUnicodeString(&Name, KeyName);
	InitializeObjectAttributes(&keyAttr, &Name, OBJ_KERNEL_HANDLE, NULL, NULL);
	Status = ZwOpenKey(&hKey, KEY_ALL_ACCESS, &keyAttr);
	if(!NT_SUCCESS(Status))
		return;

	while(TRUE){
		ret = 0;
		Status = ZwEnumerateValueKey(hKey, idx, infoClass, NULL, 0, &ret);
		if(Status != STATUS_BUFFER_TOO_SMALL){
			break;
		}
		ret += sizeof(WCHAR);
		pValueInfo = (PKEY_VALUE_FULL_INFORMATION)MM_ALLOC(ret);
		if(pValueInfo==NULL)
			break;
		RtlZeroMemory(pValueInfo, ret);
		Status = ZwEnumerateValueKey(hKey, idx, infoClass, pValueInfo, ret, &ret);
		if(Status != STATUS_SUCCESS){
			break;
		}
		Status = Callback(pValueInfo);
		if(!NT_SUCCESS(Status))
			break;

		if(pValueInfo != NULL){
			MM_FREE(pValueInfo);
			pValueInfo = NULL;
		}

		idx++;

	}

	if(pValueInfo != NULL){
		MM_FREE(pValueInfo);
		pValueInfo = NULL;
	}
	if(hKey)
		ZwClose(hKey);
}

NTSTATUS GetRegKeyValue(LPWSTR KeyName, LPWSTR ValueName, KEY_VALUE_INFORMATION_CLASS ValueClass, PVOID* pData)
{
	NTSTATUS Status;
	HANDLE hKey = NULL;
	UNICODE_STRING Name = {0};
	OBJECT_ATTRIBUTES keyAttr;
	UINT32 ret;
	PVOID pKeyValue = NULL;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	do 
	{
		RtlInitUnicodeString(&Name, KeyName);
		InitializeObjectAttributes(&keyAttr, &Name, OBJ_KERNEL_HANDLE, NULL, NULL);

		Status = ZwOpenKey(&hKey, KEY_ALL_ACCESS, &keyAttr);
		if(Status != STATUS_SUCCESS)
			break;
		RtlInitUnicodeString(&Name, ValueName);

		Status = ZwQueryValueKey(hKey, &Name, ValueClass, NULL, 0, &ret);
		if(Status != STATUS_BUFFER_TOO_SMALL)
			break;
		pKeyValue = (PVOID)MM_ALLOC(ret);
		if(pKeyValue == NULL){
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(pKeyValue, ret);
		Status = ZwQueryValueKey(hKey, &Name, ValueClass, pKeyValue, ret, &ret);
		if(Status != STATUS_SUCCESS)
			break;
		*pData = pKeyValue;
	} while (FALSE);

	if(Status != STATUS_SUCCESS)
		if(pKeyValue != NULL)
			MM_FREE(pKeyValue);
	if(hKey)
		ZwClose(hKey);
	return Status;
}

NTSTATUS GetSymbolicLink(PUNICODE_STRING SymbolicLinkName, PUNICODE_STRING SymbolicLink)
{
	NTSTATUS			status;
	NTSTATUS			returnStatus = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES	oa;
	UNICODE_STRING		tmpSymbolicLink = {0};
	HANDLE				tmpLinkHandle = NULL;
	ULONG				symbolicLinkLength = 0;
	
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	InitializeObjectAttributes(&oa, SymbolicLinkName, OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwOpenSymbolicLinkObject(&tmpLinkHandle, GENERIC_READ, &oa);

	if ( STATUS_SUCCESS == status ){
		status = ZwQuerySymbolicLinkObject(tmpLinkHandle, &tmpSymbolicLink, &symbolicLinkLength);

		if ( STATUS_BUFFER_TOO_SMALL == status && symbolicLinkLength > 0 ){
			tmpSymbolicLink.Buffer = (PWCH)MM_ALLOC(symbolicLinkLength );
			RtlZeroMemory(tmpSymbolicLink.Buffer, symbolicLinkLength);
			tmpSymbolicLink.Length = 0;
			tmpSymbolicLink.MaximumLength = (USHORT) symbolicLinkLength;

			status = ZwQuerySymbolicLinkObject(tmpLinkHandle, &tmpSymbolicLink, &symbolicLinkLength);

			if ( STATUS_SUCCESS == status ){
				SymbolicLink->Buffer		= tmpSymbolicLink.Buffer;
				SymbolicLink->Length		= tmpSymbolicLink.Length;
				SymbolicLink->MaximumLength = tmpSymbolicLink.MaximumLength;
				returnStatus				= STATUS_SUCCESS;
			}else{
				MM_FREE( tmpSymbolicLink.Buffer );
			}
		}
	}
	if(tmpLinkHandle)
		ZwClose(tmpLinkHandle);

	return returnStatus;
}

NTSTATUS GetSystemRootPath(PUNICODE_STRING SystemRootPath)
{
	UNICODE_STRING winSymb;

	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return STATUS_UNSUCCESSFUL;

	RtlInitUnicodeString(&winSymb, SYSTEM_ROOT_SYMBOLIC);
	return GetSymbolicLink(&winSymb, SystemRootPath);
}

/*
NTSTATUS
ExtractDriveString
(
IN OUT	PUNICODE_STRING		Source
)
{
NTSTATUS	status;
UINT32		i;
UINT32		numSlashes;

status		= STATUS_UNSUCCESSFUL;
i			= 0;
numSlashes	= 0;

while ( ( (i * 2) < Source->Length ) && ( 4 != numSlashes ) )
{
if ( L'\\' == Source->Buffer[i] )
{
numSlashes++;
}
i++;
}

if ( ( 4 == numSlashes ) && ( i > 1 ) )
{
i--;
Source->Buffer[i]	= L'\0';
Source->Length		= (USHORT) i * 2;
status				= STATUS_SUCCESS;
}

return status;
}

NTSTATUS GetSystemRootPath( OUT	PUNICODE_STRING	SystemRootPath)
{
	NTSTATUS			status;
	NTSTATUS			returnStatus;
	UNICODE_STRING		systemRootName;
	UNICODE_STRING		systemRootSymbolicLink1;
	UNICODE_STRING		systemRootSymbolicLink2;
	UNICODE_STRING		systemDosRootPath;
	PDEVICE_OBJECT		deviceObject;
	PFILE_OBJECT		fileObject;
	HANDLE				linkHandle;
	UINT32				fullPathLength;

	returnStatus = STATUS_UNSUCCESSFUL;

	RtlInitUnicodeString(&systemRootName, SYSTEM_ROOT);

	//
	// Get the full path for the system root directory
	//
	status = GetSymbolicLink(&systemRootName, &systemRootSymbolicLink1, &linkHandle);

	if ( STATUS_SUCCESS == status ){
		//
		// At this stage we have the full path but its in the form:
		// \Device\Harddisk0\Partition1\WINDOWS lets try to get the symoblic name for 
		// this drive so it looks more like c:\WINDOWS.
		//
		DbgPrint( "Full System Root Path: %ws\n", systemRootSymbolicLink1.Buffer );
		fullPathLength = systemRootSymbolicLink1.Length;
		ZwClose( linkHandle );

		//
		// Remove the path so we can query the drive letter
		//
		status = ExtractDriveString( &systemRootSymbolicLink1 );

		if ( STATUS_SUCCESS == status )
		{
			//
			// We've added a NULL termination character so we must reflect that in the 
			// total length.
			//
			fullPathLength = fullPathLength - 2;

			//
			// Query the drive letter
			//
			status = GetSymbolicLink
				(
				&systemRootSymbolicLink1,
				&systemRootSymbolicLink2,
				&linkHandle
				);

			if ( STATUS_SUCCESS == status )
			{
				status = IoGetDeviceObjectPointer
					(
					&systemRootSymbolicLink2,
					SYNCHRONIZE | FILE_ANY_ACCESS,
					&fileObject,
					&deviceObject
					);

				if ( STATUS_SUCCESS == status )
				{
					ObReferenceObject( deviceObject );

					//
					// Get the dos name for the drive
					//
					status = RtlVolumeDeviceToDosName
						(
						deviceObject,
						&systemDosRootPath
						);

					if ( STATUS_SUCCESS == status && NULL != systemDosRootPath.Buffer )
					{
						SystemRootPath->Buffer = ExAllocatePool
							( 
							NonPagedPool, 
							fullPathLength 
							);
						RtlZeroMemory( SystemRootPath->Buffer, fullPathLength );
						SystemRootPath->Length = 0;
						SystemRootPath->MaximumLength = (USHORT) fullPathLength;

						//
						// Drive
						//
						RtlMoveMemory
							( 
							SystemRootPath->Buffer,  
							systemDosRootPath.Buffer, 
							systemDosRootPath.Length 
							);

						//
						// Drive Slash
						//
						RtlMoveMemory
							( 
							SystemRootPath->Buffer + (systemDosRootPath.Length/2),  
							L"\\",  
							2 
							);

						//
						// Drive Slash Directory
						//
						RtlMoveMemory
							( 
							SystemRootPath->Buffer + (systemDosRootPath.Length/2) + 1,  
							systemRootSymbolicLink1.Buffer + 
							(systemRootSymbolicLink1.Length/2) + 1,  
							fullPathLength - systemRootSymbolicLink1.Length
							);

						SystemRootPath->Length = (systemDosRootPath.Length + 2) + 
							((USHORT) fullPathLength - systemRootSymbolicLink1.Length);

						ExFreePool( systemDosRootPath.Buffer );

						returnStatus = STATUS_SUCCESS;
					}

					ObDereferenceObject( deviceObject );
				}
				ZwClose( linkHandle );
				ExFreePool( systemRootSymbolicLink2.Buffer );
			}
		}

		ExFreePool( systemRootSymbolicLink1.Buffer );
	}
	return returnStatus;
}
*/
/*
void Ring0EnumProcess()
{
//初始化缓冲区大小 32kb
UINT32 cbBuffer = 0x8000;

PVOID pBuffer = NULL;
NTSTATUS ntStatus;
PSYSTEM_PROCESS_INFORMATION pInfo;
do 
{ //分配内存缓冲区
pBuffer = ExAllocatePool(NonPagedPool, cbBuffer);
if (pBuffer == NULL)
{
KdPrint(("分配内存失败!"));
return; 
}
ntStatus = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, pBuffer, cbBuffer, NULL);
if (ntStatus == STATUS_INFO_LENGTH_MISMATCH) //如果缓冲区太小
{
ExFreePool(pBuffer); //释放缓冲区
cbBuffer*=2;   //增加缓冲区到原来的2倍
}
else if (!NT_SUCCESS(ntStatus)) //如果获取信息不成功
{
ExFreePool(pBuffer);
return;
}

} 
while(ntStatus == STATUS_INFO_LENGTH_MISMATCH);

pInfo = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
while(TRUE)
{
LPWSTR pszProcessName = pInfo->ProcessName.Buffer;
//如果获取映像名失败则返回空
if (pszProcessName == NULL)
{
pszProcessName = L"NULL";
}
DbgPrint("pid %d ps %S\n", pInfo->ProcessId, pInfo->ProcessName.Buffer); //调试输出结果
if (pInfo->NextEntryDelta == 0)
{
break; //没有后继了,退出链表循环.
}
pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo)+pInfo->NextEntryDelta);

}

ExFreePool(pBuffer); //释放分配的内存
return;
}
*/