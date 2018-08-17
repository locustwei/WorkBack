#include "Monitor.h"
#include "../WiseRegNotify/DirverUtils.h"

typedef struct _MAL_HASH_ENTRY
{//
	PMAL_FILE_NAME pName;
	struct _MAL_HASH_ENTRY* pNext;
}MAL_HASH_ENTRY, *PMAL_HASH_ENTRY;

//通知用户程序，等待用户程序确认放行、阻止-------
typedef struct _PROC_NOTIFY_DATA {  
	KEVENT hEvent;
	BOOLEAN bDeny;
	PPROC_NOTIFY_DATA_OUT Data;
}PROC_NOTIFY_DATA, *PPROC_NOTIFY_DATA;

typedef struct _NOTIFY_DATA_ENTRY{ 
	struct _NOTIFY_DATA_ENTRY* pNext;
	PPROC_NOTIFY_DATA pNotifyData;
}NOTIFY_DATA_ENTRY, *PNOTIFY_DATA_ENTRY;
//-------------------------------------------------

// typedef struct _SPIN_LOCK   //自旋锁
// {
// 	KSPIN_LOCK  SpinLock;
// 	KIRQL       Irql;
// } SPIN_LOCK, *PSPIN_LOCK;

struct PROCESSS_NOTIFY_DATA
{
	BOOLEAN         bSet;                  //已经设置了进程回掉函数
	PKEVENT         UserEvent;              //通知用户程序的Event
	SPIN_LOCK       hashSpinLock;           
	SPIN_LOCK       notifySpinLock;

	PNOTIFY_DATA_ENTRY pNotifyStack;          //等待用户程序读取栈 
	PMAL_HASH_ENTRY MalHash[MAL_HASH_COUNT];  //程序文件名hash表
};

struct PROCESSS_NOTIFY_DATA g_ProcNotify;

static ULONG HashKey(PWCH Key, int nLength)
{
	ULONG i = 0;
	while (nLength-- > 0) 
		i = (i << 5) + towlower(Key[nLength]) + i;
	return i;
};

NTSTATUS InitGlobalData()
{
	RtlZeroMemory(g_ProcNotify.MalHash, MAL_HASH_COUNT * sizeof(PMAL_HASH_ENTRY));
	RtlZeroMemory(&g_ProcNotify, sizeof(struct PROCESSS_NOTIFY_DATA));
	KeInitializeSpinLock(&g_ProcNotify.hashSpinLock.SpinLock);
	KeInitializeSpinLock(&g_ProcNotify.notifySpinLock.SpinLock);
	return STATUS_SUCCESS;
}

NTSTATUS FreeGlobalData()
{
	int i;
	PMAL_HASH_ENTRY pEntry;
	PNOTIFY_DATA_ENTRY pNotify;

	DbgPrint("FreeGlobalData");

	RemoveProcessNotify();

	KeAcquireSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, &g_ProcNotify.hashSpinLock.Irql);
	__try
	{
		for(i=0; i<MAL_HASH_COUNT; i++)
		{
			while(g_ProcNotify.MalHash[i] != NULL)
			{
				pEntry = g_ProcNotify.MalHash[i];
				g_ProcNotify.MalHash[i] = pEntry->pNext;
				
				ExFreePool(pEntry->pName);
				ExFreePool(pEntry);
			}

			g_ProcNotify.MalHash[i] = NULL;
		}
	}
	__finally
	{
		KeReleaseSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, g_ProcNotify.hashSpinLock.Irql);
	}
	
	KeAcquireSpinLock(&g_ProcNotify.notifySpinLock.SpinLock, &g_ProcNotify.notifySpinLock.Irql);
	__try
	{
		while(g_ProcNotify.pNotifyStack)
		{
			pNotify = g_ProcNotify.pNotifyStack;
			g_ProcNotify.pNotifyStack = pNotify->pNext;
			KeSetEvent(&pNotify->pNotifyData->hEvent, IO_NO_INCREMENT, FALSE);

// 			ExFreePool(pNotify->pNotifyData->Data);
// 			ExFreePool(pNotify->pNotifyData);
			ExFreePool(pNotify);
		}

		g_ProcNotify.pNotifyStack = NULL;
	}
	__finally
	{
		KeReleaseSpinLock(&g_ProcNotify.notifySpinLock.SpinLock, g_ProcNotify.notifySpinLock.Irql);
	}
	return STATUS_SUCCESS;
}

BOOLEAN IsExistsMalwareName(PWCH pName, int Length)
{
	int nCode = HashKey(pName, Length) % MAL_HASH_COUNT;
	PMAL_HASH_ENTRY p;
	
	DbgPrint("IsExistsMalwareName %d %x", nCode, g_ProcNotify.MalHash[nCode]);
	
	for(p = g_ProcNotify.MalHash[nCode]; p!=NULL; p=p->pNext)
	{
 		if(p->pName->nLen == Length && _wcsnicmp(p->pName->Name, pName, Length) == 0)
 			return TRUE;
	}
	
	return FALSE;
}

BOOLEAN IsMalwareProcess(PCUNICODE_STRING pFileName)
{
	int i;
	PWCH p;
	BOOLEAN result;

	for(i=pFileName->Length/sizeof(WCHAR)-1; i>=0;i--)
	{
		if(pFileName->Buffer[i]=='\\')
		{
			break;
		}
	}
	if(i>=pFileName->Length/sizeof(WCHAR) - 1)
		return FALSE;
	i++;

	p = (PWCH)ExAllocatePool(PagedPool, (pFileName->Length/sizeof(WCHAR)-i)*sizeof(WCHAR));
	RtlZeroMemory(p, (pFileName->Length/sizeof(WCHAR) -i)*sizeof(WCHAR));
	RtlMoveMemory(p, pFileName->Buffer+i, (pFileName->Length/sizeof(WCHAR) - i)*sizeof(WCHAR));
	result = IsExistsMalwareName(p, pFileName->Length/sizeof(WCHAR) - i);
	ExFreePool(p);

	return result;
}

// NTSTATUS TerminateMalware(HANDLE ProcessID)
// {
// 	NTSTATUS status = STATUS_SUCCESS;
// 	
// 	HANDLE hProc;
// 
// 	status = ZwOpenProcess(&hProc, 0, NULL, NULL, ProcessID);
// 
// 	if(NT_SUCCESS(status))
// 	{
// 		status = ZwTerminateProcess(hProc, STATUS_SUCCESS);
// 		ZwClose(hProc);
// 	}
// 
// 	return status;
// }

//初始化注册表操作数据
PPROC_NOTIFY_DATA InitRegNotifyData(HANDLE ParentID, HANDLE ProcessID, PCUNICODE_STRING pCommand, PCUNICODE_STRING pImage)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PPROC_NOTIFY_DATA Result = NULL;
	ULONG Length = 0;
	PPROC_NOTIFY_DATA_OUT pOut;
	do 
	{
		if(pCommand)
			Length += pCommand->Length;
		if(pImage)
			Length += pImage->Length;
		pOut = (PPROC_NOTIFY_DATA_OUT)ExAllocatePool(NonPagedPool, sizeof(PROC_NOTIFY_DATA_OUT) + Length + 2 * sizeof(WCHAR));
		if(!pOut)
			break;

		RtlZeroMemory(pOut, sizeof(PROC_NOTIFY_DATA_OUT) + Length + 2 * sizeof(WCHAR));
		pOut->size = sizeof(PROC_NOTIFY_DATA_OUT) + Length + 2 * sizeof(WCHAR);
		pOut->Cookie = (ULONG64)pOut;

		pOut->ParentID = (ULONG)ParentID;
		pOut->ProcessID = (ULONG)ProcessID;
		pOut->CommandLine = 0;
		if(pCommand && pCommand->Length)
			RtlMoveMemory((PUCHAR)pOut->Data + pOut->CommandLine, pCommand->Buffer, pCommand->Length);
		pOut->ImageFile = pCommand->Length + sizeof(WCHAR);
		if(pImage && pImage->Length)
			RtlMoveMemory((PUCHAR)pOut->Data + pOut->ImageFile, pImage->Buffer, pImage->Length);
		Result = (PPROC_NOTIFY_DATA)ExAllocatePool(NonPagedPool, sizeof(PROC_NOTIFY_DATA));
		if(!Result)
		{
			ExFreePool(pOut);
			break;
		}
		RtlZeroMemory(Result, sizeof(PROC_NOTIFY_DATA));
		Result->Data = pOut;
		KeInitializeEvent(&Result->hEvent, SynchronizationEvent, FALSE);

	} while (FALSE);

	return Result;
}

NTSTATUS PushCallbackNotifyData(PPROC_NOTIFY_DATA pNotifyData)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PNOTIFY_DATA_ENTRY Result = NULL;

	do 
	{
		if(pNotifyData == NULL)
			break;

		Result = (PNOTIFY_DATA_ENTRY)ExAllocatePool (
			NonPagedPool, 
			sizeof(NOTIFY_DATA_ENTRY));

		if(Result == NULL){
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(Result, sizeof(NOTIFY_DATA_ENTRY));
		Result->pNotifyData = pNotifyData;

		KeAcquireSpinLock(&g_ProcNotify.notifySpinLock.SpinLock, &g_ProcNotify.notifySpinLock.Irql);
		__try
		{
			Result->pNext = g_ProcNotify.pNotifyStack;
			g_ProcNotify.pNotifyStack = Result;
		}
		__finally
		{
			KeReleaseSpinLock(&g_ProcNotify.notifySpinLock.SpinLock, g_ProcNotify.notifySpinLock.Irql);
		}

	} while (FALSE);

	if(Status != STATUS_SUCCESS){
		if(Result != NULL){
			ExFreePool(Result);
			Result = NULL;
		}
	}

	return Status;
}

NTSTATUS WaitForUser(PKEVENT pUser, PKEVENT pEvent)
{
	LARGE_INTEGER TimeOut;
	LONG out;
	
	DbgPrint("WaitForUser");

	KeSetEvent(pUser, IO_NO_INCREMENT, FALSE);

	TimeOut.QuadPart = -3*1000*1000*100;
	KeClearEvent(pEvent);
	return KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, &TimeOut); 
}

void PopNotifyData(PPROC_NOTIFY_DATA pNotifyData)
{
	PNOTIFY_DATA_ENTRY pEntry = NULL, p = NULL;

	KeAcquireSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, &g_ProcNotify.hashSpinLock.Irql);
	__try
	{
		for(pEntry = g_ProcNotify.pNotifyStack; pEntry != NULL; pEntry = pEntry->pNext)
		{
			if(pEntry->pNotifyData == pNotifyData)
			{
				if(p)
					p->pNext = pEntry->pNext;
				else
					g_ProcNotify.pNotifyStack = pEntry->pNext;
				ExFreePool(pEntry);
				break;;
			}
			p = pEntry;
		}
	}
	__finally
	{
		KeReleaseSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, g_ProcNotify.hashSpinLock.Irql);
	}
}

void CreateProcessNotifyRoutineEx(HANDLE ParentID, HANDLE ProcessID, PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	PPROC_NOTIFY_DATA pNotifyData;

	if(PASSIVE_LEVEL != KeGetCurrentIrql())
		return;

	if(!CreateInfo)
	{
		return;
	}
	else
	{
		DbgPrint("ProcessID = %d CommandLine = %wZ FileName=%wZ FileOpenNameAvailable=%d Flags=%x ImageFileName=%wZ", 
			ProcessID,
			CreateInfo->CommandLine,
			CreateInfo->FileObject? &CreateInfo->FileObject->FileName:(PUNICODE_STRING)NULL,
			CreateInfo->FileOpenNameAvailable,
			CreateInfo->Flags,
			CreateInfo->ImageFileName);

		if(CreateInfo->ImageFileName != NULL && CreateInfo->ImageFileName->Length > 0)
		{
			if(IsMalwareProcess(CreateInfo->ImageFileName))
			{
				DbgPrint("IsMalwareProcessx ImageFileName=%wZ", CreateInfo->ImageFileName);

				pNotifyData = InitRegNotifyData(ParentID, ProcessID, CreateInfo->CommandLine, CreateInfo->ImageFileName);
				if(pNotifyData == NULL)
					return;

				if(NT_SUCCESS(PushCallbackNotifyData(pNotifyData)))
				{
					if(NT_SUCCESS(WaitForUser(g_ProcNotify.UserEvent, &pNotifyData->hEvent))){
						if(pNotifyData->bDeny){
							//TerminateMalware(ProcessID);
							CreateInfo->CreationStatus = STATUS_OBJECT_PATH_INVALID;
						}
					}
					PopNotifyData(pNotifyData);
				}

				ExFreePool(pNotifyData->Data);
				ExFreePool(pNotifyData);
			}
		}
	}
	return;
}

void CreateProcessNotifyRoutine(HANDLE ParentID, HANDLE ProcessID, BOOLEAN Create)
{
	PPROC_NOTIFY_DATA pNotifyData;
	PUNICODE_STRING pImageName = NULL;

	if(PASSIVE_LEVEL != KeGetCurrentIrql())
		return;

	if(!Create)
	{
		return;
	}
	else
	{
		pImageName = GetProcessImageFileName(ProcessID);
		if(pImageName == NULL)
			return;

		if(IsMalwareProcess(pImageName))
		{
			DbgPrint("IsMalwareProcessx ImageFileName=%wZ", pImageName);

			pNotifyData = InitRegNotifyData(ParentID, ProcessID, NULL, pImageName);
			if(pNotifyData == NULL)
				return;

			if(NT_SUCCESS(PushCallbackNotifyData(pNotifyData)))
			{
				if(NT_SUCCESS(WaitForUser(g_ProcNotify.UserEvent, &pNotifyData->hEvent))){
					if(pNotifyData->bDeny){
						//TerminateMalware(ProcessID);
						//CreateInfo->CreationStatus = STATUS_OBJECT_PATH_INVALID;
						ZwTerminateProcess(ProcessID, STATUS_SUCCESS);
					}
				}
				PopNotifyData(pNotifyData);
			}

			ExFreePool(pNotifyData->Data);
			ExFreePool(pNotifyData);
		}

		MM_FREE(pImageName);
	}
	return;
}

NTSTATUS AddMalFileName(ULONG nSize, PMAL_FILE_NAME pName)
{
	int nCode;
	PMAL_HASH_ENTRY p;

	if(!pName || nSize < sizeof(MAL_FILE_NAME) || nSize > sizeof(MAL_FILE_NAME) + 260*sizeof(WCHAR))
		return STATUS_INVALID_PARAMETER;

	nCode = HashKey(pName->Name, pName->nLen) % MAL_HASH_COUNT;

	DbgPrint("AddMalFileName %d %S", nCode, pName->Name);

	for(p = g_ProcNotify.MalHash[nCode]; p!=NULL; p=p->pNext)
	{
		if(p->pName->nLen == pName->nLen && _wcsnicmp(p->pName->Name, pName->Name, pName->nLen) == 0)
		{
			//p->pName->block = pName->block;
			return STATUS_SUCCESS;
		}
	}

	p = (PMAL_HASH_ENTRY)ExAllocatePool(NonPagedPool, sizeof(MAL_HASH_ENTRY));
	p->pNext = g_ProcNotify.MalHash[nCode];
	g_ProcNotify.MalHash[nCode]=p;
	p->pName = (PMAL_FILE_NAME)ExAllocatePool(NonPagedPool, sizeof(MAL_FILE_NAME)+pName->nLen*sizeof(WCHAR));
	p->pName->nLen = pName->nLen;
	//p->pName->block = pName->block;
	RtlMoveMemory(p->pName->Name, pName->Name, pName->nLen*sizeof(WCHAR));

	return STATUS_SUCCESS;
}

NTSTATUS RemoveMalFileName(ULONG nSize, PMAL_FILE_NAME pName)
{

	int nCode = HashKey(pName->Name, pName->nLen) % MAL_HASH_COUNT;
	PMAL_HASH_ENTRY p, p1=NULL;
	
	if(!pName || nSize < sizeof(MAL_FILE_NAME) || nSize > sizeof(MAL_FILE_NAME) + 260*sizeof(WCHAR))
		return STATUS_INVALID_PARAMETER;

	for(p = g_ProcNotify.MalHash[nCode]; p!=NULL; p=p->pNext)
	{
		if(p->pName->nLen == pName->nLen && _wcsnicmp(p->pName->Name, pName->Name, pName->nLen) == 0)
		{
			if(p1==NULL)
				g_ProcNotify.MalHash[nCode] = p->pNext;
			else
				p1->pNext = p->pNext;
			ExFreePool(p->pName);
			ExFreePool(p);
			break;
		}
		p1 = p;
	}

	return STATUS_SUCCESS;
}

NTSTATUS SetProcessNotify(ULONG InputBufferLength, PREGISTER_CALLBACK_IN pInput)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	do 
	{
		if(g_ProcNotify.bSet)
			break;

		if (pInput==NULL || InputBufferLength < sizeof(REGISTER_CALLBACK_IN)) {
			Status = STATUS_PARAMETER_QUOTA_EXCEEDED;
			break;
		}

		Status = ObReferenceObjectByHandle((HANDLE)pInput->UserEvent, EVENT_MODIFY_STATE, *ExEventObjectType, UserMode, (PVOID*)&g_ProcNotify.UserEvent, NULL);
		if(!NT_SUCCESS(Status)){
			break;
		}	
#ifdef _XP_
		Status = PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, FALSE);
#else
		Status = PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyRoutineEx, FALSE);
#endif 


		g_ProcNotify.bSet = NT_SUCCESS(Status);

	} while (FALSE);


	DbgPrint("SetProcessNotify = %x", Status);

	return Status;
}

NTSTATUS RemoveProcessNotify()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	do 
	{
		if(!g_ProcNotify.bSet)
			break;

#ifdef _XP_
		Status = PsSetCreateProcessNotifyRoutine(CreateProcessNotifyRoutine, TRUE);
#else
		Status = PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyRoutineEx, TRUE);
#endif 
		
		if(!NT_SUCCESS(Status))
			break;

		if(g_ProcNotify.UserEvent)
			ObDereferenceObject(g_ProcNotify.UserEvent);

		g_ProcNotify.bSet = FALSE;

	} while (FALSE);

	return Status;
}

NTSTATUS GetRegNotifyData(ULONG OutputBufferLength, PPROC_NOTIFY_DATA_OUT pOut, PULONG_PTR pRetlen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PPROC_NOTIFY_DATA_OUT pData = NULL;

	KeAcquireSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, &g_ProcNotify.hashSpinLock.Irql);
	__try
	{
		if(g_ProcNotify.pNotifyStack)
		{
			pData = g_ProcNotify.pNotifyStack->pNotifyData->Data;
			if(OutputBufferLength < pData->size)
			{
				Status = STATUS_FLT_BUFFER_TOO_SMALL;
			}else
			{
				RtlMoveMemory(pOut, pData, pData->size);
				Status = STATUS_SUCCESS;
			}
			*pRetlen = pData->size;
		}
	}
	__finally
	{
		KeReleaseSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, g_ProcNotify.hashSpinLock.Irql);
	}
	return Status;
}

NTSTATUS SetRegNotifyResult(ULONG InputBufferLength, PPROC_NOTIFY_DATA_IN Input)
{
	NTSTATUS Status = STATUS_SUCCESS;	
	PNOTIFY_DATA_ENTRY pEntry = NULL;
	
	DbgPrint("SetRegNotifyResult %d %d", InputBufferLength, sizeof(PROC_NOTIFY_DATA_IN));

	if(!Input || InputBufferLength != sizeof(PROC_NOTIFY_DATA_IN))
		return STATUS_INVALID_PARAMETER;

	KeAcquireSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, &g_ProcNotify.hashSpinLock.Irql);
	__try
	{
		for(pEntry = g_ProcNotify.pNotifyStack; pEntry != NULL; pEntry = pEntry->pNext)
		{
			if(pEntry->pNotifyData->Data->Cookie == Input->DataCookie)
			{
				pEntry->pNotifyData->bDeny = Input->Denied;
				KeSetEvent(&pEntry->pNotifyData->hEvent, IO_NO_INCREMENT, FALSE);
				DbgPrint("KeSetEvent %d", Input->Denied);
				break;
			}
		}
	}
	__finally
	{
		KeReleaseSpinLock(&g_ProcNotify.hashSpinLock.SpinLock, g_ProcNotify.hashSpinLock.Irql);
	}

	return Status;
}
