/************************************************************************/
/*                                                                      */
/************************************************************************/
#include "DirverUtils.h"
#include "regfltr.h"

#define REG_ROOT_STRING               L"\\REGISTRY\\"  //注册表键名前面的字符（以这个开头的为完整键名）
#define REG_USER_ROOT_STRING          L"\\REGISTRY\\USER\\"  
#define REG_KEY_EXCLUDE_FILES         L"\\REGISTRY\\MACHINE\\SOFTWARE\\WiseCleaner\\WiseCare365\\RegFilter\\ExcludeFiles"  //设置信息写入注册表（排外列表等）
#define REG_KEY_EXCLUDE_POSITION      L"\\REGISTRY\\MACHINE\\SOFTWARE\\WiseCleaner\\WiseCare365\\RegFilter\\ExcludePosition"  //设置信息写入注册表（排外列表等）
#define REG_KEY_ROOT                  L"\\REGISTRY\\MACHINE\\SOFTWARE\\WiseCleaner\\WiseCare365\\RegFilter"  //设置信息注册表
#define REG_VALUE_DATAPATH            L"configfile"

#define DOS_FILE_PATH L"\\??\\"

PCALLBACK_CONTEXT g_Context = NULL;  

SPIN_LOCK g_PositionLock = {0};
ULONG g_PositionCount = 0;                 //注册表监控位置个数
PREGFILTER_POSITION* g_FilterPositions = NULL; //注册表监控位置数组
SPIN_LOCK g_ExcludePositionLock = {0};
PEXCLUDE_POSITION_STATCK g_ExcludePositionStack = NULL;

SPIN_LOCK g_ExcludeFilesLock = {0};            //访问排外文件回旋锁
PEXCLU_FILES_STACK g_ExcludeFileStack = NULL;  //按文件名排外堆栈
/*
SPIN_LOCK g_ExcludePidLock = {0};
PEXCLU_PID_STACK g_ExcludeHash[EXCLUDE_PID_HASH_COUNT] = {0};  //按进程ID排外堆栈
*/
SPIN_LOCK g_WaitNotifyLock = {0};  //
PNOTIFY_DATA_STACK g_WaitNotifyStack;  //已经被用户程序读出的通知堆栈

#define g_WhiteCount 6    //固定的白名单（系统程序排外）
PWSTR* g_WhiteList = NULL;  //白名单

const
	PWSTR EXCLUDE_WHITE_FILES[g_WhiteCount] = {
		L"\\system32\\services.exe",
		L"\\system32\\driverquery.exe",
		L"\\system32\\drvcfg.exe",
		L"\\system32\\ntoskrnl.exe",
		L"\\system32\\csrss.exe",
		L"\\system32\\svchost.exe"
};

/*
VOID ProcessNotifyProc(IN HANDLE ParentId, IN HANDLE ProcessId, IN BOOLEAN Create)
{
	PUNICODE_STRING pFilename = NULL;
	PEXCLU_FILES_STACK pExcludeStack = g_ExcludeFileStack; 
	if(Create){
		pFilename = GetProcessImageFileName(ProcessId);
		if(pFilename==NULL)
			return;
		if(InExcludeFile((PWSTR)pFilename->Buffer)!=NULL)
			AddExcludePid(ProcessId);
		FreeProcessImageFileName(pFilename);
	}else{
		RemoveExcludeId(ProcessId);
		//UnRegisterProcess(ProcessId);
	}
}
*/

/*从配置文件中加载注册表监控项（一个项可包含多个键、值）
配置文件位置这注册表\\REGISTRY\\MACHINE\\SOFTWARE\\WiseCleaner\\WiseCare365\\RegFilter 值configfile定义
*/
NTSTATUS LoadPositionData(PUNICODE_STRING fname)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PKEY_VALUE_PARTIAL_INFORMATION pKeyValue = NULL;
	HANDLE hFile = NULL;
	OBJECT_ATTRIBUTES fileAttr;
	IO_STATUS_BLOCK iosb;
	ULONG i, nPosition = 0, nCount = 0, k, nCheck;
	LARGE_INTEGER offset = {0};
	PPOSITION_KEY pPosKey = NULL;
	PREGFILTER_POSITION* tmpPosition;
	PUNICODE_STRING fileName;

	do 
	{
		Status = GetRegKeyValue(REG_KEY_ROOT, REG_VALUE_DATAPATH, KeyValuePartialInformation , &pKeyValue);
		if(Status != STATUS_SUCCESS)
			break;
		//文件名前面加上"\\??\\"
		fileName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, sizeof(UNICODE_STRING));
		fileName->Length = pKeyValue->DataLength - sizeof(WCHAR) + 8;
		fileName->MaximumLength = fileName->Length;
		fileName->Buffer = (PWSTR)ExAllocatePool(NonPagedPool, fileName->Length);
		RtlZeroMemory(fileName->Buffer, fileName->Length);
		RtlCopyMemory(fileName->Buffer, DOS_FILE_PATH, wcslen(DOS_FILE_PATH)*sizeof(WCHAR));
		RtlCopyMemory(fileName->Buffer + wcslen(DOS_FILE_PATH), pKeyValue->Data, pKeyValue->DataLength - sizeof(WCHAR));

		InitializeObjectAttributes(&fileAttr, fileName, OBJ_KERNEL_HANDLE, NULL, NULL);
		Status = ZwCreateFile(&hFile, GENERIC_READ|GENERIC_WRITE, &fileAttr, &iosb, NULL,  FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN, FILE_NON_DIRECTORY_FILE|FILE_RANDOM_ACCESS|FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);  

		if(Status != STATUS_SUCCESS){
			break;
		}

		Status = ZwReadFile(hFile, NULL, NULL, NULL, &iosb, &nCount, sizeof(nCount), &offset, NULL);
		if(Status != STATUS_SUCCESS){
			break;
		}

		if(nCount == 0){ //节点个数
			Status = STATUS_INVALID_CONFIG_FILE;
			break;
		}

		tmpPosition = (PREGFILTER_POSITION*)ExAllocatePool(NonPagedPool, nCount * sizeof(PREGFILTER_POSITION));
		if(tmpPosition == NULL){
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(tmpPosition, nCount * sizeof(PREGFILTER_POSITION));
		offset.QuadPart = sizeof(nCount);
		k = 0;
		while(Status == STATUS_SUCCESS){
			Status = ZwReadFile(hFile, NULL, NULL, NULL, &iosb, &nPosition, sizeof(nPosition), &offset, NULL);
			if(Status != STATUS_SUCCESS){
				break;
			}
			
			tmpPosition[k] = (PREGFILTER_POSITION)ExAllocatePool(NonPagedPool, nPosition);
			if(tmpPosition[k] == NULL){
				Status = STATUS_MEMORY_NOT_ALLOCATED;
				break;
			}
			RtlZeroMemory(tmpPosition[k], nPosition);

			offset.QuadPart += sizeof(nPosition);
			Status = ZwReadFile(hFile, NULL, NULL, NULL, &iosb, tmpPosition[k], nPosition, &offset, NULL);
			if(Status != STATUS_SUCCESS){
				break;
			}

			tmpPosition[k]->id = k + 1;

			offset.QuadPart += nPosition;
			nCheck = 0;
			for (i=0; i< tmpPosition[k]->keyCount; i++){ //验证文件正确性
				nCheck += tmpPosition[k]->Keys[i].KeySize + tmpPosition[k]->Keys[i].ValueSize + sizeof(WCHAR);
				if((nCheck > nPosition)||
					(tmpPosition[k]->Keys[i].Key == 0 || tmpPosition[k]->Keys[i].Key > nPosition) ||
					(tmpPosition[k]->Keys[i].Value != 0 && tmpPosition[k]->Keys[i].Key + tmpPosition[k]->Keys[i].KeySize + sizeof(WCHAR) != tmpPosition[k]->Keys[i].Value)){
						Status = STATUS_INVALID_CONFIG_FILE;
						break;
				}
			}
			k++;
		}
		
		if(k != nCount){  //验证文件的正确性
			Status = STATUS_INVALID_CONFIG_FILE;
			break;
		}else
			Status = STATUS_SUCCESS;

	} while (FALSE);

	if(hFile)
		ZwClose(hFile);
	if(pKeyValue)
		ExFreePool(pKeyValue);
	if(Status != STATUS_SUCCESS){
		if(tmpPosition != NULL){
			for (i = 0; i < nCount; i++){
				if(tmpPosition[i] != NULL)
					ExFreePool(tmpPosition[i]);
			}
			ExFreePool(tmpPosition);
		}

	}else{
		
		g_FilterPositions = tmpPosition;
		g_PositionCount = nCount;

		LoadRegsiterExcludePositions();

	}
	
	return Status;
}
//添加固定排外程序
NTSTATUS InitWhiterList()
{
	
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG length, i;
	UNICODE_STRING winSymb, winDir;

	RtlInitUnicodeString(&winSymb, SYSTEM_ROOT_SYMBOLIC);
	Status = GetSymbolicLink(&winSymb, &winDir);
	if(!NT_SUCCESS(Status))
		return Status;
	g_WhiteList = (PWSTR*)ExAllocatePool(NonPagedPool, g_WhiteCount * sizeof(PWSTR));
	if(g_WhiteList == NULL){
		return STATUS_MEMORY_NOT_ALLOCATED;
	}
	RtlZeroMemory(g_WhiteList, g_WhiteCount * sizeof(PWSTR));
	for(i = 0; i < g_WhiteCount; i++){
		length = wcslen(EXCLUDE_WHITE_FILES[i]) * sizeof(WCHAR) + winDir.Length + sizeof(WCHAR);
		g_WhiteList[i] = (PWSTR)ExAllocatePool(NonPagedPool, length);
		if(g_WhiteList[i] == NULL)
			return STATUS_MEMORY_NOT_ALLOCATED;
		RtlZeroMemory(g_WhiteList[i], length);
		RtlMoveMemory(g_WhiteList[i], winDir.Buffer, winDir.Length);
		RtlMoveMemory((PCHAR)g_WhiteList[i] + winDir.Length, EXCLUDE_WHITE_FILES[i], length - winDir.Length - sizeof(WCHAR));
	}
	FreeUtilsMemory(winDir.Buffer);

	return STATUS_SUCCESS;
}
//加载注册表中排外程序回掉函数
NTSTATUS EnumerateExcludeFileCallback(PVOID pData)
{
	PKEY_VALUE_FULL_INFORMATION pValueInfo = (PKEY_VALUE_FULL_INFORMATION)pData;
	if(pData == NULL)
		return STATUS_UNSUCCESSFUL;

	PushExcludeFile(pValueInfo->Name, pValueInfo->NameLength, *(PULONG)((PCHAR)pValueInfo + pValueInfo->DataOffset));
	return STATUS_SUCCESS;
}
//加载注册表中排外程序
VOID LoadRegsiterExcludeFiles()
{
	EnumerateRegValueKey(REG_KEY_EXCLUDE_FILES, KeyValueFullInformation, &EnumerateExcludeFileCallback);
}
//加载注册表中排外注册表位置回掉函数
NTSTATUS EnumerateExcludePositionCallback(PVOID pData)
{
	PKEY_VALUE_FULL_INFORMATION pValueInfo = (PKEY_VALUE_FULL_INFORMATION)pData;
	if(pData == NULL)
		return STATUS_UNSUCCESSFUL;

	PushExcludePositionByName(pValueInfo->Name, pValueInfo->NameLength, *(PULONG)((PCHAR)pValueInfo + pValueInfo->DataOffset));
	return STATUS_SUCCESS;
}
//加载注册表中排外位置
VOID LoadRegsiterExcludePositions()
{
	EnumerateRegValueKey(REG_KEY_EXCLUDE_POSITION, KeyValueFullInformation, &EnumerateExcludePositionCallback);
}

//DriverEntry 初始化全局数据
NTSTATUS InitGlobalData()
{
	NTSTATUS status = STATUS_SUCCESS;

	KeInitializeSpinLock(&g_PositionLock.SpinLock);
	KeInitializeSpinLock(&g_ExcludeFilesLock.SpinLock);
	KeInitializeSpinLock(&g_WaitNotifyLock.SpinLock);
	KeInitializeSpinLock(&g_ExcludePositionLock.SpinLock);	

// 	status = LoadPositionData(NULL);
// 	if(STATUS_SUCCESS != status)
// 		return status;

	status = InitWhiterList();
	if(STATUS_SUCCESS != status)
		return status;

	LoadRegsiterExcludeFiles();

	return status;
}
//DeviceUnload 清理全局数据
NTSTATUS FreeGlobalData()
{
	NTSTATUS status = STATUS_SUCCESS;
	PEXCLU_FILES_STACK pFileStack = NULL;
	PEXCLUDE_POSITION_STATCK pPosStack = NULL;
	ULONG i;
	PVOID pTmp;

	//status = PsSetCreateProcessNotifyRoutine(ProcessNotifyProc, TRUE);

	AcquireSpinLock(&g_ExcludeFilesLock);
	__try{
		if(g_WhiteList){
			for(i = 0; i < g_WhiteCount; i++){
				if(g_WhiteList[i])
					ExFreePool(g_WhiteList[i]);
			}
			ExFreePool(g_WhiteList);
			g_WhiteList = NULL;
		}
		pFileStack = g_ExcludeFileStack;
		while(pFileStack!=NULL){
			pTmp = pFileStack->pNext;
			ExFreePool(pFileStack->pExcludeFile);
			ExFreePool(pFileStack);
			pFileStack = (PEXCLU_FILES_STACK)pTmp;
		}
		g_ExcludeFileStack = NULL;
	}__finally{
		ReleaseSpinLock(&g_ExcludeFilesLock);
	}
	
	AcquireSpinLock(&g_ExcludePositionLock);
	__try{
		pPosStack = g_ExcludePositionStack;
		while(pPosStack != NULL){
			pTmp = pPosStack->pNext;
			ExFreePool(pPosStack);
			pPosStack = (PEXCLUDE_POSITION_STATCK)pTmp;
		}
		g_ExcludePositionStack = NULL;
	}__finally{
		ReleaseSpinLock(&g_ExcludePositionLock);
	}

	AcquireSpinLock(&g_PositionLock);
	__try{
		if(g_FilterPositions != NULL){
			for (i = 0; i < g_PositionCount; i++){
				if(g_FilterPositions[i] != NULL)
					ExFreePool(g_FilterPositions[i]);
			}
			ExFreePool(g_FilterPositions);
			g_FilterPositions = NULL;
		}
	}__finally{
		ReleaseSpinLock(&g_PositionLock);
	}
	
	return status;
}
//排外文件写入到注册表
NTSTATUS SaveExcludeFile()
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE hKey = NULL;
	PEXCLU_FILES_STACK pFileStack = g_ExcludeFileStack;
	UNICODE_STRING ValueName = {0};
	ULONG v, length;
	
	do 
	{
		Status = CreateRegKey(REG_KEY_EXCLUDE_FILES, &hKey);
		if(Status != STATUS_SUCCESS){
			break;
		}
		ClearRegisterKeyValues(hKey);
		
		//AcquireSpinLock(&g_ExcludeFilesLock);  //自旋锁要提高IRQL ZwSetValueKey不能这此IRQL运行
		__try{
			while(pFileStack != NULL ){
				v = pFileStack->pExcludeFile->Denied;
				if(v){
					ValueName.Length = pFileStack->pExcludeFile->nNameSize;
					ValueName.MaximumLength = pFileStack->pExcludeFile->nNameSize;
					ValueName.Buffer = pFileStack->pExcludeFile->Filename;//(PWSTR)((PCHAR)ValueName + sizeof(UNICODE_STRING));

					ZwSetValueKey(hKey, &ValueName, 0, REG_DWORD, &v, sizeof(v));
				}

				pFileStack = pFileStack->pNext;
			}
		}__finally{
			//ReleaseSpinLock(&g_ExcludeFilesLock);
		}
		

	} while (FALSE);

	if(hKey)
		ZwClose(hKey);

	return Status;
}

PREGFILTER_POSITION FindPositonById(ULONG positionID)
{
	if(positionID == 0 || positionID > g_PositionCount)
		return NULL;
	return g_FilterPositions[positionID - 1];
}
//排外注册表位置写入到注册表
NTSTATUS SaveExcludePositon()
{
	NTSTATUS Status = STATUS_SUCCESS;
	HANDLE hKey = NULL;
	PEXCLUDE_POSITION_STATCK pPosStack = g_ExcludePositionStack;
	UNICODE_STRING ValueName = {0};
	ULONG v;
	PREGFILTER_POSITION pPosition;

	do 
	{
		Status = CreateRegKey(REG_KEY_EXCLUDE_POSITION, &hKey);
		if(Status != STATUS_SUCCESS){
			break;
		}
		ClearRegisterKeyValues(hKey);

		//AcquireSpinLock(&g_ExcludeFilesLock);  //自旋锁要提高IRQL ZwSetValueKey不能这此IRQL运行
		__try{
			while(pPosStack != NULL ){
				v = pPosStack->Denied;
				pPosition = FindPositonById(pPosStack->positionId);
				if(pPosition != NULL){
					ValueName.Length = pPosition->NameSize;
					ValueName.MaximumLength = pPosition->NameSize;
					ValueName.Buffer = (PWSTR)((PCHAR)pPosition + pPosition->Name);//(PWSTR)((PCHAR)ValueName + sizeof(UNICODE_STRING));

					ZwSetValueKey(hKey, &ValueName, 0, REG_DWORD, &v, sizeof(v));
				}
				pPosStack = pPosStack->pNext;
			}
		}__finally{
			//ReleaseSpinLock(&g_ExcludeFilesLock);
		}


	} while (FALSE);

	if(hKey)
		ZwClose(hKey);

	return Status;
}

CALLBACK_NOTIFY IsExcludePosition(ULONG positionId)
{
	CALLBACK_NOTIFY result = RN_NOTIFY;
	PEXCLUDE_POSITION_STATCK pStack = NULL;

	AcquireSpinLock(&g_ExcludePositionLock);
	__try{
		pStack = g_ExcludePositionStack;
		while(pStack != NULL){
			if(pStack->positionId == positionId){
				return pStack->Denied;
			}
			pStack = pStack->pNext;
		}

	}__finally{
		ReleaseSpinLock(&g_ExcludePositionLock);
	}

	return result;
}

ULONG RegFilterClassMap(REG_NOTIFY_CLASS NotfyClass)
{
	switch(NotfyClass){
	case RegNtDeleteKey:             return 1;
	case RegNtSetValueKey:           return 1 << 6;
	case RegNtDeleteValueKey:        return 1 << 8;
	case RegNtSetInformationKey:     return 1 << 10;
	case RegNtRenameKey:             return 1 << 2;
	case RegNtPreCreateKey:          return 1 << 4;
	case RegNtPostCreateKey:         return 1 << 5;
	case RegNtPostDeleteKey:         return 1 << 3;
	case RegNtPostSetValueKey:       return 1 << 7;
	case RegNtPostDeleteValueKey:    return 1 << 9;
	case RegNtPostSetInformationKey: return 1 << 12;
	case RegNtPostRenameKey:         return 1 << 3;
	case RegNtPreCreateKeyEx:        return 1 << 4;
	case RegNtPostCreateKeyEx:       return 1 << 5;
	default:
		return 0;
	}
}

//添加排外程序
NTSTATUS PushExcludeFile(PWSTR Filename, ULONG Namesize, CALLBACK_NOTIFY Notify)
{
	PEXCLU_FILES_STACK pFileStack = NULL;
	ULONG Length;
	
	if(Notify != RN_ALLOW && Notify != RN_DENY)
		return STATUS_UNSUCCESSFUL;

	AcquireSpinLock(&g_ExcludeFilesLock);
	__try{
		pFileStack = g_ExcludeFileStack;
		while(pFileStack != NULL){
			if(Namesize == pFileStack->pExcludeFile->nNameSize && _wcsnicmp(pFileStack->pExcludeFile->Filename, Filename, Namesize / sizeof(WCHAR))==0){
				break;
			}
			pFileStack = pFileStack->pNext;
		}

		if(pFileStack != NULL){
			pFileStack->pExcludeFile->Denied = Notify;
			return STATUS_SUCCESS;		
		}


		pFileStack = (PEXCLU_FILES_STACK)ExAllocatePool(NonPagedPool, sizeof(EXCLU_FILES_STACK));
		if(pFileStack == NULL)
			return STATUS_MEMORY_NOT_ALLOCATED;
	
		Length = sizeof(EXCLUDE_FILE) + Namesize + sizeof(WCHAR);
		pFileStack->pExcludeFile = (PEXCLUDE_FILE)ExAllocatePool(NonPagedPool, Length);
		if(pFileStack->pExcludeFile == NULL){
			ExFreePool(pFileStack);
			return STATUS_MEMORY_NOT_ALLOCATED;
		}
		RtlZeroMemory(pFileStack->pExcludeFile, Length);
		pFileStack->pExcludeFile->Denied = Notify;
		pFileStack->pExcludeFile->nNameSize = Namesize;
		wcsncpy(pFileStack->pExcludeFile->Filename, Filename, Namesize / sizeof(WCHAR));

		pFileStack->pNext = g_ExcludeFileStack;
		g_ExcludeFileStack = pFileStack;

	}__finally{
		ReleaseSpinLock(&g_ExcludeFilesLock);
	}

	return STATUS_SUCCESS;
}

//移除排外程序
NTSTATUS PopExcludeFile(PWSTR Filename, ULONG Namesize)
{

	PEXCLU_FILES_STACK pFileStack = NULL, p;
	
	AcquireSpinLock(&g_ExcludeFilesLock);
	__try{
		pFileStack = g_ExcludeFileStack;
		while(pFileStack != NULL){

			if(pFileStack->pExcludeFile->nNameSize == Namesize &&  
				_wcsnicmp(pFileStack->pExcludeFile->Filename, Filename, pFileStack->pExcludeFile->nNameSize / sizeof(WCHAR))==0){
				if(pFileStack == g_ExcludeFileStack)
					g_ExcludeFileStack = pFileStack->pNext;
				else{
					p->pNext = pFileStack->pNext;
				}
				ExFreePool(pFileStack->pExcludeFile);
				ExFreePool(pFileStack);
				break;
			}
			p = pFileStack;
			pFileStack = pFileStack->pNext;
		}
	}__finally{
		ReleaseSpinLock(&g_ExcludeFilesLock);
	}

	return STATUS_SUCCESS;
}
//添加排外位置
NTSTATUS PushExcludePositionByName(PWSTR Posname, ULONG Namesize, CALLBACK_NOTIFY Notify)
{
	PEXCLUDE_POSITION_STATCK pPosStack = NULL;
	ULONG positionId = 0, i;

	if(Notify != RN_ALLOW && Notify != RN_DENY)
		return STATUS_UNSUCCESSFUL;

	AcquireSpinLock(&g_PositionLock);
	__try{
		for(i = 0; i<g_PositionCount; i++){
			if(g_FilterPositions[i]->NameSize == Namesize && _wcsnicmp(Posname, (PWSTR)((PCHAR)g_FilterPositions[i] + g_FilterPositions[i]->Name), Namesize) == 0){
				positionId = g_FilterPositions[i]->id;
				break;
			}
		}
	}__finally{
		ReleaseSpinLock(&g_PositionLock);
	}

	if(positionId != 0){
		PushExcludePosition(positionId, Notify);
	}

	return STATUS_SUCCESS;
}

//是否是排外程序
CALLBACK_NOTIFY IsExcludeFile(PWSTR pFilename, ULONG nNameSize)
{
	PEXCLU_FILES_STACK pFileStack = g_ExcludeFileStack;
	ULONG i;
	if(g_WhiteList)
		for(i=0; i<g_WhiteCount; i++){ //固定的白名单
			if(wcslen(g_WhiteList[i])*sizeof(WCHAR) == nNameSize && _wcsnicmp(g_WhiteList[i], pFilename, nNameSize / sizeof(WCHAR))==0)
				return RN_ALLOW;
		}

	AcquireSpinLock(&g_ExcludeFilesLock);
	__try{
		while(pFileStack != NULL){
			if(nNameSize == pFileStack->pExcludeFile->nNameSize && _wcsnicmp(pFileStack->pExcludeFile->Filename, pFilename, nNameSize / sizeof(WCHAR))==0){
				return pFileStack->pExcludeFile->Denied;
			}
			pFileStack = pFileStack->pNext;
		}
	}__finally{
		ReleaseSpinLock(&g_ExcludeFilesLock);
	}

	return RN_NOTIFY;
}

VOID UnRegisterProcess(HANDLE ProcessId)
{
	if(g_Context != NULL && g_Context->hProcess == ProcessId){
		UnRegisterCallback();
	}

	SaveExcludeFile();
	SaveExcludePositon();
}

//释放注册表操作数据内存
VOID FreeNotifyData(PFILR_NOTIFY_DATA pNotifyData)
{
	if(pNotifyData == NULL)
		return;
	KeClearEvent(&pNotifyData->hEvent);
	ExFreePool(pNotifyData);
}

//释放Context内存
VOID FreeCallbackContext()
{
	PNOTIFY_DATA_STACK pStack, p1;

	if(g_Context == NULL)
		return;
	AcquireSpinLock(&(g_Context)->StackLock);
	__try {
		
		if((g_Context)->UserEvent!=NULL)
			ObDereferenceObject((g_Context)->UserEvent);	 
		/*
		if(g_Context->hEvent!=NULL)
			ZwClose(g_Context->hEvent);
		*/
		/*  
		pStack = (g_Context)->NotifyStack;
		while(pStack !=NULL){
			FreeNotifyData(pStack->pNotifyData);
			p1 = pStack->pNext;
			ExFreePool(pStack);
			pStack = p1;
		}
		*/
	}__finally{
		ReleaseSpinLock(&(g_Context)->StackLock);
		ExFreePool(g_Context);
		g_Context = NULL;
	}
}

// VOID PrintPositions()
// {
// 	ULONG i, j;
// 	for (i=0; i<g_PositionCount; i++)
// 	{
// 		for (j=0; j<g_FilterPositions[i]->keyCount; j++)
// 		{
// 			InfoPrint("%d, %d",g_FilterPositions[i]->id, g_FilterPositions[i]->Keys[j].Key);
// 		}
// 	}
// }

//初始化回掉函数Context
NTSTATUS InitCallbackContext(HANDLE UserEvent) 
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES EventAttr;
	UNICODE_STRING EventName;
	HANDLE hEvent = NULL;

	if(g_FilterPositions == NULL)
		return Status;
	if(g_Context != NULL)
		return Status;

	do 
	{

		g_Context = (PCALLBACK_CONTEXT) ExAllocatePool (
			NonPagedPool, 
			sizeof(CALLBACK_CONTEXT));

		if  (g_Context == NULL) {
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(g_Context, sizeof(CALLBACK_CONTEXT));
		g_Context->hProcess = PsGetCurrentProcessId();
		KeInitializeSpinLock(&g_Context->StackLock.SpinLock);


		Status = ObReferenceObjectByHandle(UserEvent, EVENT_MODIFY_STATE, *ExEventObjectType, UserMode, (PVOID*)&g_Context->UserEvent, NULL);
		if(!NT_SUCCESS(Status)){
			break;
		}	

		Status = STATUS_SUCCESS;
	} while (FALSE);

	if(!NT_SUCCESS(Status)){
		ExFreePool(g_Context);
		g_Context = NULL;
	}
	//InfoPrint("InitCallbackContext %x", Status);
	return Status;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//把注册表操作数据从等等堆栈中移除。
VOID RemoveWaitNotifyDataStck(PNOTIFY_DATA_STACK pStack)
{
	PNOTIFY_DATA_STACK pHead, p;
	BOOLEAN Found = FALSE;

	if(pStack == NULL)
		return;

	AcquireSpinLock(&g_WaitNotifyLock);
	__try
	{
		if(pStack == g_WaitNotifyStack){
			g_WaitNotifyStack = pStack->pNext;
			Found = TRUE;
		}else{
			pHead = g_WaitNotifyStack;
			while(pHead != NULL){
				if(pHead == pStack){
					p->pNext = pStack->pNext;
					Found = TRUE;
					break;
				}
				p = pHead;
				pHead = pHead->pNext;
			}
		}
	}
	__finally
	{
		ReleaseSpinLock(&g_WaitNotifyLock);		
	}

	if(!Found){ //如果等等堆栈中没有，则在Context堆栈中（用户程序没有读取）
		AcquireSpinLock(&g_Context->StackLock);
		__try
		{
			if(pStack == g_Context->NotifyStack){
				g_Context->NotifyStack = pStack->pNext;
			}else{
				pHead = g_Context->NotifyStack;
				while(pHead != NULL){
					if(pHead == pStack){
						p->pNext = pStack->pNext;
						break;
					}
					p = pHead;
					pHead = pHead->pNext;
				}
			}
		}
		__finally
		{
			ReleaseSpinLock(&g_Context->StackLock);		
		}
	}
}
//初始化注册表操作数据
PFILR_NOTIFY_DATA InitRegNotifyData(ULONG positionId, REG_NOTIFY_CLASS NotifyClass, PUNICODE_STRING pKeyName,  PUNICODE_STRING pSubKeyname, PUNICODE_STRING pValueName, PUNICODE_STRING pNewName, PUNICODE_STRING pProcessName)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PFILR_NOTIFY_DATA Result = NULL;
	ULONG Length = 0;

	do 
	{
		if(positionId == 0 || positionId > g_PositionCount)
			break;

		if(pKeyName != NULL)
			Length += pKeyName->Length + sizeof(WCHAR);
		if(pSubKeyname != NULL)
			Length += pSubKeyname->Length + sizeof(WCHAR);
		if(pValueName != NULL)
			Length += pValueName->Length + sizeof(WCHAR);
		if(pNewName != NULL)
			Length += pNewName->Length + sizeof(WCHAR);
		if(pProcessName != NULL){
			Length += pProcessName->Length + sizeof(WCHAR);
		}

		Result = (PFILR_NOTIFY_DATA)ExAllocatePool(NonPagedPool, Length + sizeof(FILR_NOTIFY_DATA));
		if(Result == NULL){
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(Result, Length + sizeof(FILR_NOTIFY_DATA));

		Result->Data.nSize = Length + sizeof(REGISTER_NOTIFY_DATA_OUT);
		Result->Data.NotifyClass = RegFilterClassMap(NotifyClass);

		Length = sizeof(REGISTER_NOTIFY_DATA_OUT);

		if(pProcessName!=NULL){
			Result->Data.FileName = Length;
			RtlMoveMemory((PCHAR)&Result->Data+Length, pProcessName->Buffer, pProcessName->Length);
			Length += pProcessName->Length + sizeof(WCHAR);
		}

		if(pKeyName!=NULL){
			Result->Data.KeyName = Length;
			RtlMoveMemory((PCHAR)&Result->Data+Length, pKeyName->Buffer, pKeyName->Length);
			Length += pKeyName->Length + sizeof(WCHAR);
		}

		if(pSubKeyname!=NULL){
			Result->Data.SubKeyName = Length;
			RtlMoveMemory((PCHAR)&Result->Data+Length, pSubKeyname->Buffer, pSubKeyname->Length);
			Length += pSubKeyname->Length + sizeof(WCHAR);
		}

		if(pValueName != NULL){
			Result->Data.ValueName = Length;
			RtlMoveMemory((PCHAR)&Result->Data+Length, pValueName->Buffer, pValueName->Length);
			Length += pValueName->Length + sizeof(WCHAR);
		}

		if(pNewName!=NULL){
			Result->Data.NewName = Length;
			RtlMoveMemory((PCHAR)&Result->Data+Length, pNewName->Buffer, pNewName->Length);
			Length += pNewName->Length + sizeof(WCHAR);
		}
		Result->Data.positionId = positionId;
		Result->Data.Cookie = (ULONG64)Result;

		KeInitializeEvent(&Result->hEvent, SynchronizationEvent, FALSE);

	} while (FALSE);

	return Result;
}
//把注册表操作数据压入Context堆栈中等待用户程序读取
PNOTIFY_DATA_STACK PushCallbackNotifyData(PFILR_NOTIFY_DATA pNotifyData)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PNOTIFY_DATA_STACK Result = NULL;

	do 
	{
		if(pNotifyData == NULL)
			break;

		Result = (PNOTIFY_DATA_STACK)ExAllocatePool (
			NonPagedPool, 
			sizeof(NOTIFY_DATA_STACK));

		if(Result == NULL){
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(Result, sizeof(NOTIFY_DATA_STACK));
		Result->pNotifyData = pNotifyData;

		AcquireSpinLock(&g_Context->StackLock);
		Result->pNext = g_Context->NotifyStack;
		g_Context->NotifyStack = Result;
		ReleaseSpinLock(&g_Context->StackLock);

	} while (FALSE);

	if(Status != STATUS_SUCCESS){
		if(Result != NULL){
			ExFreePool(Result);
			Result = NULL;
		}
	}

	return Result;
}
//注册表操作数据从Context弹出，并压入等待堆栈（用户读取操作数据后删除）
PFILR_NOTIFY_DATA PopCallbackNotifyData()
{
	PNOTIFY_DATA_STACK Result = NULL;

	if(g_Context == NULL || g_Context->NotifyStack == NULL)
		return NULL;

	AcquireSpinLock(&g_Context->StackLock);
	__try{
		Result = g_Context->NotifyStack;
		g_Context->NotifyStack = g_Context->NotifyStack->pNext;
	}
	__finally{
		ReleaseSpinLock(&g_Context->StackLock);
	}

	if(!Result)
		return NULL;

	AcquireSpinLock(&g_WaitNotifyLock);
	__try{
		Result->pNext = g_WaitNotifyStack;
		g_WaitNotifyStack = Result;
	}__finally{
		ReleaseSpinLock(&g_WaitNotifyLock);
	}
	if(Result)
		return Result->pNotifyData;
	else
		return NULL;
}
//用户程序设置注册表操作结果（允许/拒绝）
PFILR_NOTIFY_DATA SetWaitNotifyData(ULONG64 DataCookie, CALLBACK_NOTIFY Denied)
{
	PFILR_NOTIFY_DATA Result = NULL;
	PNOTIFY_DATA_STACK pStack;

	AcquireSpinLock(&g_WaitNotifyLock);
	__try{
		pStack = g_WaitNotifyStack;
		while(pStack){
			if((ULONG64)pStack->pNotifyData == DataCookie){
				Result = pStack->pNotifyData;
				Result->Data.Denied = Denied;
				KeSetEvent(&pStack->pNotifyData->hEvent, IO_NO_INCREMENT, FALSE);
				break;
			}
			pStack = pStack->pNext;
		}
	}
	__finally{
		ReleaseSpinLock(&g_WaitNotifyLock);
	}

	return Result;
}

//添加排外注册表位置
NTSTATUS PushExcludePosition(ULONG positionId, CALLBACK_NOTIFY Denied)
{
	PEXCLUDE_POSITION_STATCK pStack = NULL, pTmp = NULL;
	CALLBACK_NOTIFY rn = RN_NOTIFY;

	if(/*positionId == 0 || positionId > g_PositionCount ||*/ Denied > RN_DENY)
		return STATUS_INVALID_PARAMETER;
	AcquireSpinLock(&g_ExcludePositionLock);
	__try{
		pStack = g_ExcludePositionStack;
		while(pStack != NULL){

			if(pStack->positionId == positionId){
				if(Denied == RN_NOTIFY){  //删除
					if(pStack == g_ExcludePositionStack)
						g_ExcludePositionStack = pStack->pNext;
					else
						pTmp->pNext = pStack->pNext;
					ExFreePool(pStack);
				}else
					pStack->Denied = Denied;
				
				return STATUS_SUCCESS;
			}
			pTmp = pStack;
			pStack = pStack->pNext;
		}
		if(Denied == RN_ALLOW || Denied == RN_DENY){
			pStack = (PEXCLUDE_POSITION_STATCK)ExAllocatePool(NonPagedPool, sizeof(PEXCLUDE_POSITION_STATCK));
			if(pStack == NULL)
				return STATUS_MEMORY_NOT_ALLOCATED;
			RtlZeroMemory(pStack, sizeof(PEXCLUDE_POSITION_STATCK));
			pStack->pNext = g_ExcludePositionStack;
			pStack->positionId = positionId;
			pStack->Denied = Denied;
			g_ExcludePositionStack = pStack;
		}
	}__finally
	{
		ReleaseSpinLock(&g_ExcludePositionLock);
	}
	return STATUS_SUCCESS;
}
//
PUNICODE_STRING GetRegistryObjectCompleteName(PVOID pRootObject, PUNICODE_STRING pKeyName) 
{
	NTSTATUS status = STATUS_SUCCESS; 
	ULONG Length = 0; 
	PUNICODE_STRING pRootName = NULL;
	PUNICODE_STRING pFullName = NULL;

	do 
	{
		if(pKeyName != NULL && pKeyName->Length>10*sizeof(WCHAR) && RtlCompareMemory(pKeyName->Buffer, REG_ROOT_STRING, 10*sizeof(WCHAR))==10*sizeof(WCHAR)){  //pKeyName 已经是全名
			Length = sizeof(UNICODE_STRING) + pKeyName->Length + sizeof(WCHAR);
			pFullName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, Length);
			if(pFullName==NULL)
				break;
			RtlZeroMemory(pFullName, Length);
			CopyUnicodeString(pFullName, pKeyName);
			pFullName->MaximumLength = pFullName->Length;
		}else if(pRootObject != NULL){
			pRootName = GetObjectNameString(pRootObject);
			if(pRootName==NULL)
				break;
			Length = sizeof(UNICODE_STRING) + pRootName->Length + sizeof(WCHAR);
			if(pKeyName != NULL)
				Length += pKeyName->Length + sizeof(WCHAR) * 2;

			pFullName = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, Length);
			if(pFullName==NULL)
				break;
			RtlZeroMemory(pFullName, Length);
			pFullName->MaximumLength = Length;

			CopyUnicodeString(pFullName, pRootName);

			if(pKeyName != NULL ){
				*(PWCHAR)(pFullName->Buffer + pFullName->Length / sizeof(WCHAR)) = '\\';
				pFullName->Length += sizeof(WCHAR);
				//UnicodeStringCatString(pFullName, L"\\", 4);
				UnicodeStringCat(pFullName, pKeyName);
			}
		}

	} while (FALSE);

	if(pRootName!=NULL)
		FreeUtilsMemory(pRootName);
	if(status != STATUS_SUCCESS){
		if(pFullName!=NULL){
			ExFreePool(pFullName);
			pFullName = NULL;
		}
	}

	return pFullName;
}

PUNICODE_STRING ExractKeyName(PUNICODE_STRING pFullKeyname)
{
	PUNICODE_STRING Result = NULL;
	ULONG i = 0;
	ULONG len = pFullKeyname->Length / sizeof(WCHAR);

	for(i=len-1; i>=0; i--){
		if(pFullKeyname->Buffer[i] == '\\')
			break;
	}
	if(i<=0)
		return NULL;
	
	len = (len - i) * sizeof(WCHAR);

	Result = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, sizeof(UNICODE_STRING) + len);
	if(Result == NULL)
		return NULL;
	RtlZeroMemory(Result, sizeof(UNICODE_STRING) + len);

	Result->Buffer = (PWSTR)((PCHAR)Result + sizeof(UNICODE_STRING));
	Result->Length = len - sizeof(WCHAR);
	Result->MaximumLength = len;
	RtlMoveMemory(Result->Buffer, pFullKeyname->Buffer + i + 1, Result->Length);

	pFullKeyname->Length -= Result->Length + sizeof(WCHAR);
	RtlZeroMemory((PCHAR)pFullKeyname->Buffer + pFullKeyname->Length, len);

	return Result;
}

BOOLEAN CompareKey(PREGFILTER_POSITION pPosition, REG_NOTIFY_CLASS NotifyClass, PUNICODE_STRING pFullkeyName, PUNICODE_STRING pValueName)
{
	BOOLEAN Result = FALSE;
	
	ULONG i, KeyNameLength = pFullkeyName->Length;


	for(i = 0; i < pPosition->keyCount; i++){
		if(!pPosition->SubKeys && KeyNameLength != pPosition->Keys[i].KeySize) //不包含子键，比较长度
			continue;
		if(KeyNameLength < pPosition->Keys[i].KeySize)
			continue;
		//if(RtlCompareMemory(pFullkeyName->Buffer, ((PCHAR)pPosition + pPosition->Keys[i].Key), pPosition->Keys[i].KeySize)!=pPosition->Keys[i].KeySize)
		if(_wcsnicmp(pFullkeyName->Buffer, (PWSTR)((PCHAR)pPosition + pPosition->Keys[i].Key), (pPosition->Keys[i].KeySize / sizeof(WCHAR)))!=0)
			continue;
		
		if((NotifyClass == RegNtPreSetValueKey || NotifyClass == RegNtPreDeleteValueKey) && pPosition->Keys[i].ValueSize != 0){ // 比较 ValueName
			if(pPosition->Keys[i].Value == 0){
				if(pValueName->Length > 0)
					continue;;
			}else{
				if(pValueName->Length == 0 || pValueName->Length != pPosition->Keys[i].ValueSize)
					continue;
// 				if(RtlCompareMemory(pValueName->Buffer, ((PCHAR)pPosition + pPosition->Keys[i].Value), pValueName->Length)!=pValueName->Length)
				if(_wcsnicmp(pValueName->Buffer, (PWSTR)((PCHAR)pPosition + pPosition->Keys[i].Value), pValueName->Length / sizeof(WCHAR))!=0)
					continue;
			}
		}
		
		Result = TRUE;  //都没毛病，匹配成功
		break;
	}
	
	return Result;
}

VOID RemoveUserSid(PUNICODE_STRING pKeyName)
{
	ULONG i;

	if(pKeyName != NULL && pKeyName->Length>15*sizeof(WCHAR) && _wcsnicmp(pKeyName->Buffer, REG_USER_ROOT_STRING, 15)==0){
		for(i=15; i<pKeyName->Length / sizeof(WCHAR); i++){
			if(pKeyName->Buffer[i] == '\\' || pKeyName->Buffer[i] == '_'){
				RtlMoveMemory(pKeyName->Buffer + 15, pKeyName->Buffer + i + 1, pKeyName->Length - (i+1) * sizeof(WCHAR));
				RtlZeroMemory(pKeyName->Buffer + pKeyName->Length / sizeof(WCHAR) - (i + 1 -15), (i + 1 -15) * sizeof(WCHAR));
				pKeyName->Length = pKeyName->Length - (i + 1 -15) * sizeof(WCHAR);
				break;
			}
		}
	}

}

ULONG FindPositionIDByData(REG_NOTIFY_CLASS NotifyClass, PVOID pRegObject, PUNICODE_STRING pCompleteName, PUNICODE_STRING pValueName, PUNICODE_STRING* pNewKeyname, PUNICODE_STRING *pNewValuename)
{
	ULONG i, result = 0, myClass;
	PUNICODE_STRING KeyName = NULL, SubKeyName = NULL; 
	UNICODE_STRING ValueName = {0};
	PREGFILTER_POSITION pPosition = NULL;
	
	if(KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return 0;

	if(g_FilterPositions == NULL || g_PositionCount==0)
		return 0;

	do 
	{
		myClass = RegFilterClassMap(NotifyClass);
		if(myClass == 0)
			break;
		
		KeyName = GetRegistryObjectCompleteName(pRegObject, pCompleteName);
		if(KeyName == NULL)
			break;

		if(pValueName != NULL && pValueName->Length > 0){
			__try{
				ValueName.Length = pValueName->Length;
				ValueName.MaximumLength = pValueName->MaximumLength;
				ValueName.Buffer = (PWSTR)ExAllocatePool(NonPagedPool, pValueName->Length+sizeof(WCHAR));
				if(ValueName.Buffer){
					RtlZeroMemory(ValueName.Buffer, pValueName->Length+sizeof(WCHAR));
					RtlMoveMemory(ValueName.Buffer, pValueName->Buffer, pValueName->Length);
				}
			}__except(-1){
				break;
			}
		}
		RemoveUserSid(KeyName);

		
		AcquireSpinLock(&g_PositionLock);
		__try{
			for(i=0; i<g_PositionCount; i++){
				if((g_FilterPositions[i]->NotifyClass & myClass) == myClass){
					pPosition = g_FilterPositions[i];
					
					if(NotifyClass == RegNtPreCreateKey || NotifyClass == RegNtPreCreateKeyEx || NotifyClass == RegNtPreRenameKey || NotifyClass == RegNtPreDeleteKey){
						SubKeyName = ExractKeyName(KeyName);
						if(SubKeyName == NULL)
							break;
					}
					
					if(CompareKey(pPosition, NotifyClass, KeyName, &ValueName)){
						result = pPosition->id;
						break;
					}
				}
			}
		}__finally{
			ReleaseSpinLock(&g_PositionLock);
		}
		
		
	} while (FALSE);

	if(ValueName.Buffer)
	{
		ExFreePool(ValueName.Buffer);
		ValueName.Buffer = NULL;
	}

	if(result != 0){
		*pNewKeyname = KeyName;
		*pNewValuename = SubKeyName;
	}else{
		if(KeyName)
			ExFreePool(KeyName);
		if(SubKeyName)
			ExFreePool(SubKeyName);
	}

	return result;

}

NTSTATUS GetPositions(ULONG OutputBufferLength, PGET_POSITION_OUT pOut, PULONG_PTR pRetlen)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG i, length = 0;
	CALLBACK_NOTIFY rn;

	if(g_PositionCount)
		length = sizeof(ULONG);

	AcquireSpinLock(&g_PositionLock);
	for (i=0; i<g_PositionCount; i++){
		length += sizeof(GET_POSITION_OUT) + g_FilterPositions[i]->NameSize + sizeof(WCHAR);
	}
	ReleaseSpinLock(&g_PositionLock);

	*pRetlen = length;

	do 
	{
		if(OutputBufferLength < length){
			Status = STATUS_FLT_BUFFER_TOO_SMALL;
			break;
		}
		RtlZeroMemory(pOut, length);

		AcquireSpinLock(&g_PositionLock);
		__try{
			
			length = g_PositionCount * sizeof(GET_POSITION_OUT);
			for (i=0; i<g_PositionCount; i++){
				pOut[i].id = g_FilterPositions[i]->id;
				pOut[i].name = length;
				pOut[i].Denied = IsExcludePosition(g_FilterPositions[i]->id);
				RtlMoveMemory((PCHAR)pOut + pOut[i].name, (PCHAR)g_FilterPositions[i] + g_FilterPositions[i]->Name, g_FilterPositions[i]->NameSize);
				length += g_FilterPositions[i]->NameSize;
				length += sizeof(WCHAR);
			}
			
		}__finally{
			ReleaseSpinLock(&g_PositionLock);
		}


		*(PULONG_PTR)((PCHAR)pOut + length) = g_PositionCount;
	} while (FALSE);

	return Status;
}

NTSTATUS SetPositions(ULONG InputBufferLength, PVOID pData)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG i, nPosSize = 0, nCount = 0, k, nCheck;
	LARGE_INTEGER offset = {0};
	PPOSITION_KEY pPosKey = NULL;
	PREGFILTER_POSITION* tmpPosition;
	PCHAR p = (PCHAR)pData;
	
	if(g_FilterPositions)
		return STATUS_SUCCESS;

	do 
	{
		nCount = *((PULONG)p);
		if(nCount == 0){ //节点个数
			Status = STATUS_INVALID_CONFIG_FILE;
			break;
		}

		tmpPosition = (PREGFILTER_POSITION*)ExAllocatePool(NonPagedPool, nCount * sizeof(PREGFILTER_POSITION));
		if(tmpPosition == NULL){
			Status = STATUS_MEMORY_NOT_ALLOCATED;
			break;
		}
		RtlZeroMemory(tmpPosition, nCount * sizeof(PREGFILTER_POSITION));
		p += sizeof(nCount);
		k = 0;
		while(p - pData < InputBufferLength){
			nPosSize = *((PULONG)p);

			tmpPosition[k] = (PREGFILTER_POSITION)ExAllocatePool(NonPagedPool, nPosSize);
			if(tmpPosition[k] == NULL){
				Status = STATUS_MEMORY_NOT_ALLOCATED;
				break;
			}
			RtlZeroMemory(tmpPosition[k], nPosSize);
			p += sizeof(nPosSize);
			RtlMoveMemory(tmpPosition[k], p, nPosSize);
			tmpPosition[k]->id = k + 1;

			p += nPosSize;
			nCheck = 0;
			//InfoPrint("%d %S", tmpPosition[k]->Name, (PUCHAR)tmpPosition[k] + tmpPosition[k]->Name);
			for (i=0; i< tmpPosition[k]->keyCount; i++){ //验证文件正确性
				nCheck += tmpPosition[k]->Keys[i].KeySize + tmpPosition[k]->Keys[i].ValueSize + sizeof(WCHAR);
				//InfoPrint("%S", (PUCHAR)tmpPosition[k] + tmpPosition[k]->Keys[i].Key);
				//if(tmpPosition[k]->Keys[i].Value)
					//InfoPrint("%S", (PUCHAR)tmpPosition[k] + tmpPosition[k]->Keys[i].Value);
				if((nCheck > nPosSize)||
					(tmpPosition[k]->Keys[i].Key == 0 || tmpPosition[k]->Keys[i].Key > nPosSize) ||
					(tmpPosition[k]->Keys[i].Value != 0 && tmpPosition[k]->Keys[i].Key + tmpPosition[k]->Keys[i].KeySize + sizeof(WCHAR) != tmpPosition[k]->Keys[i].Value)){
						Status = STATUS_INVALID_CONFIG_FILE;
						break;
				}
			}
			k++;
		}

		if(k != nCount){  //验证文件的正确性
			Status = STATUS_INVALID_CONFIG_FILE;
			break;
		}else
			Status = STATUS_SUCCESS;

	} while (FALSE);

	if(Status != STATUS_SUCCESS){
		if(tmpPosition != NULL){
			for (i = 0; i < nCount; i++){
				if(tmpPosition[i] != NULL)
					ExFreePool(tmpPosition[i]);
			}
			ExFreePool(tmpPosition);
		}

	}else{
		g_FilterPositions = tmpPosition;
		g_PositionCount = nCount;
		LoadRegsiterExcludePositions();
	}
	
	return Status;
}

NTSTATUS GetExcludeFiles(ULONG OutputBufferLength, PGET_EXCLUDE_FILE_OUT pOut, PULONG_PTR pRetlen)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG k = 0, length = 0;
	PEXCLU_FILES_STACK pFileStack = g_ExcludeFileStack;

	AcquireSpinLock(&g_ExcludeFilesLock);  
	__try{
		while(pFileStack != NULL){
			length += pFileStack->pExcludeFile->nNameSize + sizeof(WCHAR);
			pFileStack = pFileStack->pNext;
			k++;
		}
		length += k * sizeof(GET_EXCLUDE_FILE_OUT) + sizeof(ULONG);
	}__finally{
		ReleaseSpinLock(&g_ExcludeFilesLock);
	}

	*pRetlen = length;

	do 
	{
		if(OutputBufferLength < length){
			Status = STATUS_FLT_BUFFER_TOO_SMALL;
			break;
		}
		RtlZeroMemory(pOut, length);

		length = k * sizeof(GET_EXCLUDE_FILE_OUT);
		k = 0;
		AcquireSpinLock(&g_ExcludeFilesLock);  
		__try{
			pFileStack = g_ExcludeFileStack;
			while(pFileStack != NULL){

				pOut[k].Denied = pFileStack->pExcludeFile->Denied;
				pOut[k].name = length;
				RtlMoveMemory((PCHAR)pOut + pOut[k].name, pFileStack->pExcludeFile->Filename, pFileStack->pExcludeFile->nNameSize);
				length += pFileStack->pExcludeFile->nNameSize + sizeof(WCHAR);
				pFileStack = pFileStack->pNext;
				k++;
			}
		}__finally{
			ReleaseSpinLock(&g_ExcludeFilesLock);
		}

		*(PULONG_PTR)((PCHAR)pOut + length) = k;
	} while (FALSE);

	return Status;
}