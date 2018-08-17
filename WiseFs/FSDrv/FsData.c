
#include <fltKernel.h>
#include "FsData.h"
#include "CtrlDevice.h"
#include "..\..\WiseRegNotify\DirverUtils.h"

#define CONFIG_FILE_NAME L"wisefs.dat"
#define MAX_HIDE_COUNT   10 

HANDLE g_ConfigFile = NULL;


SPIN_LOCK g_StackLock;

PFS_FILE_VOLUME_STACK g_FsHide_Stack   = NULL;      //对应FS_HIDE类型堆栈
PFS_FILE_VOLUME_STACK g_FsWrite_Stack  = NULL;      //对应FS_WRITE类型堆栈
PFS_FILE_VOLUME_STACK g_FsDelete_Stack = NULL;      //对应FS_DELETE类型堆栈
PFS_FILE_VOLUME_STACK g_FsRename_Stack = NULL;      //对应FS_RENAME类型堆栈
PFS_FILE_VOLUME_STACK g_FsCreate_Stack = NULL;      //对应FS_CREATE类型堆栈

ULONG g_FilesCount = 0;
PFS_FILE_RECORD* g_FsFiles = NULL;

VOID FreeVolumeStack(PFS_FILE_VOLUME_STACK pStack)
{
	PFS_FILE_VOLUME_STACK pVolume = NULL, pVolume1 = NULL;
	PFS_FILE_STACK pLink = NULL, pLink1 = NULL;
	pVolume = pStack;
	while(pVolume){
		pLink = pVolume->pFileStack;
		while(pLink){
			pLink1 = pLink->pNext;
			ExFreePool(pLink);
			pLink = pLink1;
		}
		pVolume1 = pVolume->pNext;
		ExFreePool(pVolume);
		pVolume = pVolume1;
	}
}

VOID FsFreeData()
{
	ULONG i, itmp;

	SaveConfigFile();

	if(g_ConfigFile){
		ZwClose(g_ConfigFile);
		g_ConfigFile = NULL;
	}

	AcquireSpinLock(&g_StackLock);
	__try{
		FreeVolumeStack(g_FsHide_Stack);
		FreeVolumeStack(g_FsRename_Stack);
		FreeVolumeStack(g_FsWrite_Stack);
		FreeVolumeStack(g_FsDelete_Stack);
		g_FsHide_Stack = NULL;
		g_FsRename_Stack = NULL;
		g_FsWrite_Stack = NULL;
		g_FsDelete_Stack = NULL;

		itmp = g_FilesCount;
		g_FilesCount = 0;

		for(i=0; i<g_FilesCount; i++){
			if(g_FsFiles[i]){
				ExFreePool(g_FsFiles[i]);
				g_FsFiles[i] = NULL;
			}
		}
		if(g_FsFiles)
			ExFreePool(g_FsFiles);
		g_FsFiles = NULL;

	}__finally{
		ReleaseSpinLock(&g_StackLock);
	}

	
}

VOID LoadFsData()
{
	KeInitializeSpinLock(&g_StackLock.SpinLock);

	LoadConfigFile();
}

#pragma region 配置文件

NTSTATUS GetConfigFileName(PWSTR* szConfigFileName)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PWSTR szFileName = NULL;
	UNICODE_STRING windir = {0};
	ULONG nCb;

	do 
	{
		Status = GetSystemRootPath(&windir);
		if(!NT_SUCCESS(Status))
			break;

		nCb = windir.Length + wcslen(CONFIG_FILE_NAME)*sizeof(WCHAR) + sizeof(WCHAR) * 2;
		szFileName = (PWSTR)ExAllocatePool(NonPagedPool, nCb);
		if(szFileName == NULL)
			break;
		RtlZeroMemory(szFileName, nCb);
		RtlMoveMemory(szFileName, windir.Buffer, windir.Length);
		szFileName[wcslen(szFileName)] = '\\';
		wcscat(szFileName, CONFIG_FILE_NAME);
		*szConfigFileName = szFileName;

	} while (FALSE);

	if(windir.Buffer)
		ExFreePool(windir.Buffer);

	return Status;
}

PFS_FILE_RECORD* ReadConfigRecord(ULONG nCount, IO_STATUS_BLOCK* iosb, PLARGE_INTEGER offset)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG k = 0, nCb;
	PFS_FILE_RECORD* g_tmp;

	g_tmp = (PFS_FILE_RECORD*)ExAllocatePool(NonPagedPool, nCount * sizeof(PFS_FILE_RECORD));

	if(g_tmp == NULL){
		return NULL;
	}
	RtlZeroMemory(g_tmp, nCount * sizeof(PFS_FILE_RECORD));
	k = 0;
	while(NT_SUCCESS(Status) && k < nCount){
		Status = ZwReadFile(g_ConfigFile, NULL, NULL, NULL, iosb, &nCb, sizeof(nCb), offset, NULL);
		if(!NT_SUCCESS(Status) || iosb->Information == 0){
			break;
		}

		g_tmp[k] = (PFS_FILE_RECORD)ExAllocatePool(NonPagedPool, nCb);
		if(g_tmp[k] == NULL){
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlZeroMemory(g_tmp[k], nCb);

		Status = ZwReadFile(g_ConfigFile, NULL, NULL, NULL, iosb, g_tmp[k], nCb, offset, NULL);
		if(!NT_SUCCESS(Status) || iosb->Information != nCb){
			break;
		}
		g_tmp[k]->bDisable = FALSE;
		g_tmp[k]->id = k;

		offset->QuadPart += nCb;

		k++;
	}

	if(!NT_SUCCESS(Status) || nCount != k ){
		if(g_tmp){
			while(k-->0){
				if(g_tmp[k])
					ExFreePool(g_tmp[k]);
			}
			ExFreePool(g_tmp);
		}
		g_tmp = NULL;
	}

	return g_tmp;
}

PFS_FILE_VOLUME_STACK PushVolume(PCWSTR pVolumeStr, PFS_FILE_VOLUME_STACK* pVolumeStack)
{
	int l1, l2, k;
	PFS_FILE_VOLUME_STACK pVolume = NULL;
	PFS_FILE_VOLUME_STACK pNewVolume = NULL;

	l1 = wcslen(pVolumeStr);
	for(pVolume = *pVolumeStack; pVolume != NULL; pVolume = pVolume->pNext){
		l2 = wcslen((pVolume)->Volume);
		k = _wcsnicmp(pVolumeStr, (pVolume)->Volume, l1>l2 ? l2:l1);


		if(k == 0 && l1==l2){
			return pVolume;
		}else if(k > 0 || (k == 0 && l1 > l2)){
			break;
		}
	}

	pNewVolume = (PFS_FILE_VOLUME_STACK)MM_ALLOC(sizeof(FS_FILE_VOLUME_STACK));
	RtlZeroMemory(pNewVolume, sizeof(FS_FILE_VOLUME_STACK));
	pNewVolume->Volume = (PWSTR)MM_ALLOC((1+wcslen(pVolumeStr))*sizeof(WCHAR));
	RtlZeroMemory(pNewVolume->Volume, (1+wcslen(pVolumeStr))*sizeof(WCHAR));
	wcscat(pNewVolume->Volume, pVolumeStr);

	if(*pVolumeStack == NULL){
		*pVolumeStack = pNewVolume;
	}else if(pVolume == NULL){
		for(pVolume = *pVolumeStack; pVolume->pNext != NULL; pVolume = pVolume->pNext);
		pVolume->pNext = pNewVolume;
		pNewVolume->pPriv = pVolume;
	}else{
		if(!pVolume->pPriv){
			pNewVolume->pNext = *pVolumeStack;
			(*pVolumeStack)->pPriv = pNewVolume;
			*pVolumeStack = pNewVolume;
		}else{
			pVolume->pPriv->pNext = pNewVolume;
			pNewVolume->pNext = pVolume;
			pNewVolume->pPriv = pVolume->pPriv;
			pVolume->pPriv = pNewVolume;
		}
	}

	return pNewVolume;
}
/*
 *	按大小顺序放入堆栈
 */
VOID PushFile2VolumeStack(ULONG RecordId, PFS_FILE_VOLUME_STACK* pVolumeStack)
{
	int l1, l2, k;
	PFS_FILE_VOLUME_STACK pVolume = NULL;
	PFS_FILE_VOLUME_STACK pNewVolume = NULL;
	PFS_FILE_STACK pLink = NULL, pNewLink = NULL;

	PFS_FILE_RECORD pFile = g_FsFiles[RecordId]; 
	if(!pFile)
		return;

	pVolume = PushVolume(GETFILE_VOLUME(pFile), pVolumeStack);
	if(!pVolume)
		return;

	pNewLink = (PFS_FILE_STACK)ExAllocatePool(NonPagedPool, sizeof(FS_FILE_STACK));
	if(pNewLink == NULL)
		return;
	RtlZeroMemory(pNewLink, sizeof(FS_FILE_STACK));		
	pNewLink->RecordId = RecordId;

	if(!pVolume->pFileStack){
		pVolume->pFileStack = pNewLink;
		return;
	}

	for(pLink = pVolume->pFileStack; pLink; pLink = pLink->pNext){
		if(g_FsFiles[pLink->RecordId]){
			l1 = wcslen(GETFILE_PATH(pFile));
			l2 = wcslen(GETFILE_PATH(g_FsFiles[pLink->RecordId]));
			if(_wcsnicmp(GETFILE_PATH(g_FsFiles[pLink->RecordId]), GETFILE_PATH(pFile), l1>l2 ? l2:l1)<0){
				break;
			}
		}
	}

	if(pLink){
		if(pLink->pPriv)
			pLink->pPriv->pNext = pNewLink;
		else 
			pVolume->pFileStack = pNewLink;

		pNewLink->pPriv = pLink->pPriv;
		pNewLink->pNext = pLink;

		pLink->pPriv = pNewLink;
	}else {//放在最后
		for(pLink = pVolume->pFileStack; pLink->pNext; pLink = pLink->pNext);
		pLink->pNext = pNewLink;
		pNewLink->pPriv = pLink;
	}
}

VOID PopFileVolumeStack(ULONG RecordId, PFS_FILE_VOLUME_STACK* pVolumeStack)
{
	PFS_FILE_VOLUME_STACK pVolume = NULL, pVolume1 = NULL;
	PFS_FILE_STACK pLink = NULL;

	for(pVolume = *pVolumeStack; pVolume; pVolume = pVolume->pNext){
		for(pLink = pVolume->pFileStack; pLink; pLink = pLink->pNext){
			if(pLink->RecordId == RecordId){

				if(pLink->pPriv)
					pLink->pPriv->pNext = pLink->pNext;
				else
					pVolume->pFileStack = pLink->pNext;
				if(pLink->pNext)
					pLink->pNext->pPriv = pLink->pPriv;

				ExFreePool(pLink);

				if(pVolume->pFileStack == NULL){
					if(pVolume == *pVolumeStack)
						*pVolumeStack = pVolume->pNext;
					else if(pVolume1){
						pVolume1->pNext = pVolume->pNext;
					}

					ExFreePool(pVolume->Volume);
					ExFreePool(pVolume);
				}
				return;
			}
		}
		pVolume1 = pVolume;
	}

}

VOID DispatchFiles(ULONG count, PFS_FILE_RECORD* pFiles)
{
	ULONG i;

	PAGED_CODE();

	for(i=0; i<count; i++){
		if(FS_HIDE & pFiles[i]->pt){
			PushFile2VolumeStack(i, &g_FsHide_Stack);
		}
		if(FS_WRITE & pFiles[i]->pt){
			PushFile2VolumeStack(i, &g_FsWrite_Stack);
		}
		if(FS_DELETE & pFiles[i]->pt){
			PushFile2VolumeStack(i, &g_FsDelete_Stack);
		}
		if(FS_RENAME & pFiles[i]->pt){
			PushFile2VolumeStack(i, &g_FsRename_Stack);
		}
		if(FS_CREATE & pFiles[i]->pt)
			PushFile2VolumeStack(i, &g_FsCreate_Stack);
	}
}

NTSTATUS LoadConfigFile()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES fileAttr;
	IO_STATUS_BLOCK iosb;

	LARGE_INTEGER offset = {0};
	UNICODE_STRING fileName;

	CONFIG_FILE_HEADER header = {0};
	
	PWSTR szConfigFileName = NULL;
	do 
	{

		Status = GetConfigFileName(&szConfigFileName);
		if(!NT_SUCCESS(Status) || szConfigFileName == NULL)
			break;
		

		RtlInitUnicodeString(&fileName, szConfigFileName);
		InitializeObjectAttributes(&fileAttr, &fileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
		RtlZeroMemory(&iosb, sizeof(IO_STATUS_BLOCK));

		Status = ZwCreateFile(&g_ConfigFile, 
			GENERIC_READ | GENERIC_WRITE, 
			&fileAttr, 
			&iosb, 
			NULL,  
			FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, 
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			FILE_OPEN_IF,
			FILE_NON_DIRECTORY_FILE |
			FILE_RANDOM_ACCESS |
			FILE_SYNCHRONOUS_IO_NONALERT,
			NULL, 
			0);  

		if(!NT_SUCCESS(Status)){
			break;
		}

 		if(iosb.Information == FILE_CREATED)
 			break;
		Status = ZwReadFile(g_ConfigFile, NULL, NULL, NULL, &iosb, &header, sizeof(CONFIG_FILE_HEADER), &offset, NULL);
		

		if(!NT_SUCCESS(Status)){
			break;
		}

		offset.QuadPart = sizeof(CONFIG_FILE_HEADER);

		if(header.FileCount){
			g_FsFiles = (PFS_FILE_RECORD*)ReadConfigRecord(header.FileCount, &iosb, &offset);
			if(g_FsFiles){
				DispatchFiles(header.FileCount, (PFS_FILE_RECORD*)g_FsFiles);
				g_FilesCount = header.FileCount;
			}else{
				Status = STATUS_UNSUCCESSFUL;
				break;
			}
		}

	} while (FALSE);

	if(szConfigFileName)
		ExFreePool(szConfigFileName);
		
	return Status;
}


NTSTATUS SaveConfigFile()
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	OBJECT_ATTRIBUTES fileAttr;
	IO_STATUS_BLOCK iosb;
	LARGE_INTEGER offset = {0};
	UNICODE_STRING fileName;
	
	FILE_VALID_DATA_LENGTH_INFORMATION fileSize;

	CONFIG_FILE_HEADER header = {0};
	ULONG i;

	do 
	{
		//break;;

		if(g_ConfigFile == NULL)
			break;
		
		Status = ZwWriteFile(g_ConfigFile, NULL, NULL, NULL, &iosb, &header, sizeof(CONFIG_FILE_HEADER), &offset, NULL);
		if(!NT_SUCCESS(Status))
			break;
		offset.QuadPart += sizeof(CONFIG_FILE_HEADER);

		for(i=0; i<g_FilesCount; i++){
			if(g_FsFiles[i]){
				Status = ZwWriteFile(g_ConfigFile, NULL, NULL, NULL, &iosb, g_FsFiles[i], g_FsFiles[i]->cb, &offset, NULL);
				if(!NT_SUCCESS(Status)){
					break;
				}
				offset.QuadPart += g_FsFiles[i]->cb;
				header.FileCount++;
			}
		}

		offset.QuadPart = 0;
		Status = ZwWriteFile(g_ConfigFile, NULL, NULL, NULL, &iosb, &header, sizeof(CONFIG_FILE_HEADER), &offset, NULL);
		if(!NT_SUCCESS(Status))
			break;

		//Status = ZwSetInformationFile(g_ConfigFile, &iosb, &fileSize, sizeof(FILE_VALID_DATA_LENGTH_INFORMATION), FileValidDataLengthInformation);
	} while (FALSE);

	return Status;
}

#pragma endregion 配置文件

#pragma region 文件过滤处理

NTSTATUS AddFsFile(PFS_FILE_RECORD pAddFile, OUT ULONGLONG* id)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG i;
	PFS_FILE_RECORD p = NULL;
	PFS_FILE_RECORD* pTmp;

	PAGED_CODE();

	do 
	{
		if(!pAddFile || pAddFile->cb < sizeof(FS_FILE_RECORD))
			break;
		p = (PFS_FILE_RECORD)ExAllocatePool(NonPagedPool, pAddFile->cb);
		if(p==NULL){
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlMoveMemory(p, pAddFile, pAddFile->cb);

		AcquireSpinLock(&g_StackLock);
		__try{
			pTmp = (PFS_FILE_RECORD*)ExAllocatePool(NonPagedPool, (g_FilesCount + 1) * sizeof(PFS_FILE_RECORD));
			if(!pTmp){
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			RtlMoveMemory(pTmp, g_FsFiles, g_FilesCount * sizeof(PFS_FILE_RECORD));
			pTmp[g_FilesCount] = p;

			if(g_FsFiles)
				ExFreePool(g_FsFiles);
			g_FsFiles = pTmp;

			if(FS_HIDE & g_FsFiles[g_FilesCount]->pt){
				PushFile2VolumeStack(g_FilesCount, &g_FsHide_Stack);
			}
			if(FS_WRITE & g_FsFiles[g_FilesCount]->pt){
				PushFile2VolumeStack(g_FilesCount, &g_FsWrite_Stack);
			}
			if(FS_DELETE & g_FsFiles[g_FilesCount]->pt){
				PushFile2VolumeStack(g_FilesCount, &g_FsDelete_Stack);
			}
			if(FS_RENAME & g_FsFiles[g_FilesCount]->pt){
				PushFile2VolumeStack(g_FilesCount, &g_FsRename_Stack);
			}
			if(FS_CREATE & g_FsFiles[g_FilesCount]->pt)
				PushFile2VolumeStack(g_FilesCount, &g_FsCreate_Stack);

			p->id = g_FilesCount;
			*id = g_FilesCount;
	
			g_FilesCount++;
			
			//SaveConfigFile();

		}__finally{
			ReleaseSpinLock(&g_StackLock);
		}


		Status = STATUS_SUCCESS;
	} while (FALSE);

	if(!NT_SUCCESS(Status)){
		if(p)
			ExFreePool(p);
	}
		

	return Status;
}

NTSTATUS RemoveFsFile(ULONGLONG id)
{
	PFS_FILE_RECORD pFile;

	PAGED_CODE();

	AcquireSpinLock(&g_StackLock);
	__try{
		if(id>=g_FilesCount)
			return STATUS_INVALID_PARAMETER;

		pFile = g_FsFiles[id];
		if(!pFile)
			return STATUS_INVALID_PARAMETER;

		if(FS_HIDE & pFile->pt)
			PopFileVolumeStack(id, &g_FsHide_Stack);
		if(FS_WRITE & pFile->pt)
			PopFileVolumeStack(id, &g_FsWrite_Stack);
		if(FS_DELETE & pFile->pt)
			PopFileVolumeStack(id, &g_FsDelete_Stack);
		if(FS_RENAME & pFile->pt)
			PopFileVolumeStack(id, &g_FsRename_Stack);

		g_FsFiles[id] = NULL;

		ExFreePool(pFile);

		return STATUS_SUCCESS;

	}__finally{
		ReleaseSpinLock(&g_StackLock);
	}

	return STATUS_UNSUCCESSFUL;
}

NTSTATUS SetFsFileEnable(ULONGLONG id, BOOLEAN bDisable)
{
	int i;
	PFS_FILE_RECORD pFile;

	PAGED_CODE();

	AcquireSpinLock(&g_StackLock);
	__try{
		
		if(id>=g_FilesCount)
			return STATUS_UNSUCCESSFUL;

		if(g_FsFiles[id])
			g_FsFiles[id]->bDisable = bDisable;

	}__finally{
		ReleaseSpinLock(&g_StackLock);
	}
	return STATUS_SUCCESS;
}

NTSTATUS FsEnableAll(BOOLEAN bDisable)
{
	int i;
	PFS_FILE_RECORD pFile;

	PAGED_CODE();

	AcquireSpinLock(&g_StackLock);
	__try{
		for(i=0; i<g_FilesCount; i++){
			if(g_FsFiles[i])
				g_FsFiles[i]->bDisable = bDisable;
		}
	}__finally{
		ReleaseSpinLock(&g_StackLock);
	}

	return STATUS_SUCCESS;
}

NTSTATUS UpdateFsFile(PFS_FILE_RECORD pFile)
{
	ULONG id = pFile->id;
	NTSTATUS Status = STATUS_SUCCESS;

	PAGED_CODE();

	AcquireSpinLock(&g_StackLock);
	__try{
		if(id >= g_FilesCount)
			return STATUS_UNSUCCESSFUL;
		if(!g_FsFiles[id])
			return STATUS_UNSUCCESSFUL;

		if(pFile->cb > g_FsFiles[id]->cb){
			ExFreePool(g_FsFiles[id]);
			g_FsFiles[id] = NULL;

			g_FsFiles[id] = (PFS_FILE_RECORD)ExAllocatePool(NonPagedPool, pFile->cb);
		}
		if(g_FsFiles[id])
			RtlMoveMemory(g_FsFiles[id], pFile, pFile->cb);

	}__finally{
		ReleaseSpinLock(&g_StackLock);
	}
	return Status;
}

NTSTATUS EnumFsFiles(OUT PVOID Buffer, ULONG* BufferLength)
{
	ULONG i, length = 0;
	PFS_FILE_RECORD pFile;
	PCHAR p = (PCHAR)Buffer;

	PAGED_CODE();

	AcquireSpinLock(&g_StackLock);
	__try{
		for(i=0; i<g_FilesCount; i++){
			if(g_FsFiles[i]){
				length += g_FsFiles[i]->cb;
				if(p && length <= *BufferLength){
					RtlMoveMemory(p, g_FsFiles[i], g_FsFiles[i]->cb);
					p += g_FsFiles[i]->cb;
				}
			}
		}
		if(length > *BufferLength){
			*BufferLength = length;
			return STATUS_BUFFER_OVERFLOW;
		}
	}__finally{
		ReleaseSpinLock(&g_StackLock);
	}
	return STATUS_SUCCESS;
}

int GetFsHideFiles(IN PUNICODE_STRING pVolume, IN PUNICODE_STRING pParentDir, OUT PWSTR** fileNames)
{
	int k, count = 0, Memcount = 0, l1, l2;
	PFS_FILE_VOLUME_STACK pVolumeStack;
	PFS_FILE_STACK pLink;
	PFS_FILE_RECORD pFile;
	PWSTR* pTmp;

	if(!pVolume || !pVolume->Buffer || !pParentDir || !pParentDir->Buffer)
		return 0;
	
	if(!g_FsHide_Stack)
		return 0;

	for(pVolumeStack = g_FsHide_Stack; pVolumeStack; pVolumeStack = pVolumeStack->pNext){
		l1 = wcslen(pVolumeStack->Volume);
		if(l1 != pVolume->Length / sizeof(WCHAR))
			continue;

		k = _wcsnicmp(pVolume->Buffer, pVolumeStack->Volume, l1); 

		if(k > 0)
			break;
		else if(k == 0){
			for(pLink = pVolumeStack->pFileStack; pLink; pLink = pLink->pNext){
				
				pFile = g_FsFiles[pLink->RecordId];
				
				if(!pFile || pFile->bDisable){
					continue;
				}

				l1 = pParentDir->Length / sizeof(WCHAR);
				l2 = wcslen(GETFILE_PATH(pFile));

				if(l1 == l2 && _wcsnicmp(pParentDir->Buffer, GETFILE_PATH(pFile), l1) == 0){
					if(count >= Memcount){							
						pTmp = (PWSTR*)ExAllocatePool(NonPagedPool, (Memcount + MAX_HIDE_COUNT) * sizeof(PWSTR));
						if(!pTmp)
							return 0;
						RtlZeroMemory(pTmp, (Memcount + MAX_HIDE_COUNT) * sizeof(PWSTR));
						if(*fileNames && Memcount){
							RtlMoveMemory(pTmp, *fileNames, Memcount * sizeof(PWSTR));
							ExFreePool(*fileNames);
						}
						*fileNames = pTmp;
						Memcount += MAX_HIDE_COUNT;
					}
					(*fileNames)[count] = (PWSTR)GETFILE_NAME(pFile);
					
					count++;
				}else if(k > 0)
					break;
						
			}
		}
	}

	return count;

}

BOOLEAN IsParentDir(PWCH Dir, ULONG DirLength, PUNICODE_STRING pDir, PUNICODE_STRING pFileName)
{
	PWCH p = NULL;
	ULONG l = (pFileName->Length + pDir->Length) / sizeof(WCHAR);
	BOOLEAN result = FALSE;

	if(l>=DirLength)
		return FALSE;

	do 
	{
		p = (PWCH)ExAllocatePool(NonPagedPool, pFileName->Length + pDir->Length + sizeof(WCHAR));
		if(!p)
			break;
		RtlZeroMemory(p, pFileName->Length + pDir->Length + sizeof(WCHAR));
		RtlMoveMemory(p, pDir->Buffer, pDir->Length);
		if(pFileName->Length > 0)
			RtlMoveMemory((PCHAR)p+pDir->Length, pFileName->Buffer, pFileName->Length);
		else
			p[l-1] = 0;

		if(_wcsnicmp(p, Dir, l) == 0){
			if(Dir[l]=='\\')
				result = TRUE;
		}
	} while (FALSE);

	if(p)
		ExFreePool(p);

	return result;
}

BOOLEAN IsChildFile(PCWSTR Dir, ULONG DirLength, PCWSTR Name, ULONG NameLength, PUNICODE_STRING pDir)
{
	PWCH p = NULL;
	ULONG l = (DirLength + NameLength ) * sizeof(WCHAR);
	BOOLEAN result = FALSE;

	if(pDir->Length < l || (pDir->Length > l && pDir->Buffer[l / sizeof(WCHAR)] != '\\'))
		return FALSE;
	do 
	{
		p = (PWCH)ExAllocatePool(NonPagedPool, l + sizeof(WCHAR));
		if(!p)
			break;
		RtlZeroMemory(p, l + sizeof(WCHAR));
		RtlMoveMemory(p, Dir, DirLength * sizeof(WCHAR));
		RtlMoveMemory(p + (DirLength), Name, NameLength * sizeof(WCHAR));

		if(_wcsnicmp(p, pDir->Buffer, l / sizeof(WCHAR)) == 0){
			result = TRUE;
		}
	} while (FALSE);

	if(p)
		ExFreePool(p);

	return result;
}

BOOLEAN IsProtectFile(FS_FILE_TYPE fsType, PFS_FILE_VOLUME_STACK pVolumeStack, PUNICODE_STRING pVolumeName, /*PUNICODE_STRING pDir,*/ PUNICODE_STRING pFileName)
{
	int k, l1, l2, l3;
	PFS_FILE_VOLUME_STACK pVolume;
	PFS_FILE_STACK pLink;
	PFS_FILE_RECORD pFile;
	
	if(!pFileName || !pFileName->Length)
		return 0;
	if(!pVolumeName || !pVolumeName->Length)
		return 0;

	for(pVolume = pVolumeStack; pVolume; pVolume = pVolume->pNext){
		l1 = pVolumeName->Length / 2;
		if(l1 != pVolumeName->Length / sizeof(WCHAR))
			continue;

		k = _wcsnicmp(pVolumeName->Buffer, pVolume->Volume, l1); 

		if(k > 0)
			break;
		else if(k == 0){
			for(pLink = pVolume->pFileStack; pLink; pLink = pLink->pNext){
				

				pFile = g_FsFiles[pLink->RecordId];
				if(!pFile || pFile->bDisable)
					continue;
				
				l1 = pFileName->Length / sizeof(WCHAR);
				l2 = wcslen(GETFILE_PATH(pFile));// - pVolumeName->Length / sizeof(WCHAR); //路径
				l3 = wcslen(GETFILE_NAME(pFile));
				
				k = _wcsnicmp(pFileName->Buffer, GETFILE_PATH(pFile), l2 > l1 ? l1:l2);

				if(k == 0){

					if(l1 >= l2 + l3 && _wcsnicmp(pFileName->Buffer + l2, GETFILE_NAME(pFile), l3) == 0){
						if(l1 == l2 + l3 || (l1 > l2 + l3 && pFileName->Buffer[l2+l3] == '\\'))
							return TRUE;
					}else if(fsType == FS_RENAME && l1<l2){ //重命名时阻止父目录
						if((GETFILE_PATH(pFile))[l1] == '\\')
							return TRUE;
					}
				}else if(k > 0)
					break;	

			}
		}
	}

	return FALSE;

}

BOOLEAN FileIsDirectory(PUNICODE_STRING pName)
{
	HANDLE hFile = NULL;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	OBJECT_ATTRIBUTES fileAttr;
	IO_STATUS_BLOCK iosb;
	FILE_BASIC_INFORMATION finfo = {0};
	LARGE_INTEGER offset = {0};
	BOOLEAN result = FALSE;

	if(KeGetCurrentIrql() != PASSIVE_LEVEL){
		return FALSE;
	}

	InitializeObjectAttributes(&fileAttr, pName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	RtlZeroMemory(&iosb, sizeof(IO_STATUS_BLOCK));

	do 
	{
		Status = ZwCreateFile(&hFile, 
			GENERIC_READ, 
			&fileAttr, 
			&iosb, 
			NULL,  
			FILE_ATTRIBUTE_NORMAL, 
			FILE_SHARE_VALID_FLAGS,
			FILE_OPEN,
			FILE_DIRECTORY_FILE,
			NULL, 
			0);
		if(!NT_SUCCESS(Status))
			break;;
		Status = ZwQueryInformationFile(hFile, &iosb, &finfo, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation);
		if(NT_SUCCESS(Status))
			result = finfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		ZwClose(hFile);
	} while (FALSE);

	return result;
}

int GetFsFiles(IN FS_FILE_TYPE fsType, IN PUNICODE_STRING pVolume, IN PUNICODE_STRING pParentDir, IN PUNICODE_STRING pName, OUT PWSTR** fileNames)
{
	PFS_FILE_VOLUME_STACK pVolumeStack = NULL;
	UNICODE_STRING FileName = {0}, Volume = {0};  //IRQL问题，必须copy
	SHORT i, l;
	PCHAR p, p1;
	BOOLEAN isDir = FALSE;
	int  ret;
	
	if(!pVolume || pVolume->Length == 0 || !pParentDir || !pName || pName->Length == 0)
		return 0;

	if(pName->Length > pVolume->Length && _wcsnicmp(pName->Buffer, pVolume->Buffer, pVolume->Length / sizeof(WCHAR)) == 0){ //名称包含路径
		FileName.Length = pName->Length - pVolume->Length;
		FileName.Buffer = (PWCH)ExAllocatePool(NonPagedPool, FileName.Length + sizeof(WCHAR));
		if(!FileName.Buffer)
			return 0;
		RtlZeroMemory(FileName.Buffer, FileName.Length + sizeof(WCHAR));
		RtlMoveMemory(FileName.Buffer, (PUCHAR)pName->Buffer + pVolume->Length, FileName.Length);
	}else{
		if(pParentDir->Length < pVolume->Length)
			return 0;
		FileName.Length = pName->Length + pParentDir->Length - pVolume->Length;
		FileName.Buffer = (PWCH)ExAllocatePool(NonPagedPool, FileName.Length + sizeof(WCHAR));
		if(FileName.Buffer)
			return 0;
		RtlZeroMemory(FileName.Buffer, FileName.Length + sizeof(WCHAR));
		RtlMoveMemory(FileName.Buffer, (PUCHAR)pParentDir->Buffer + pVolume->Length, pParentDir->Length - pVolume->Length);
		RtlMoveMemory((PUCHAR)FileName.Buffer + pParentDir->Length - pVolume->Length, pName->Buffer, pName->Length);
	}


	if(fsType == FS_HIDE && FileName.Buffer[FileName.Length / sizeof(WCHAR) - 1] != '\\'){
		FileName.Buffer[FileName.Length / sizeof(WCHAR)] = '\\';
		FileName.Length += sizeof(WCHAR);
	}


	/*
	//去掉路径中的盘符
	if( pParentDir->Length > pVolume->Length && _wcsnicmp(pVolume->Buffer, pParentDir->Buffer, pVolume->Length / sizeof(WCHAR)) == 0){  
		Dir.Length = pParentDir->Length - pVolume->Length;
		Dir.Buffer = (PWCH)ExAllocatePool(NonPagedPool, Dir.Length);
		RtlMoveMemory(Dir.Buffer, (PWCH)((PUCHAR)pParentDir->Buffer + pVolume->Length), Dir.Length);
	}else{
		Dir.Length = pParentDir->Length;
		if(Dir.Length){
			Dir.Buffer = (PWCH)ExAllocatePool(NonPagedPool, Dir.Length);
			RtlMoveMemory(Dir.Buffer, pParentDir->Buffer, Dir.Length);
		}
	}

	//去掉文件名中的路径
	l = pName->Length;
	for(i = pName->Length / sizeof(WCHAR) - 1; i>=0; i--){ 
		if(pName->Buffer[i] == '\\'){
			l = pName->Length - (i + 1) * sizeof(WCHAR);
			//FileName.Length = pName->Length - (i + 1) * sizeof(WCHAR);
			//FileName.Buffer = pName->Buffer + i + 1;
			break;
		}
	}
	p1 = (PCHAR)pName->Buffer + (pName->Length - l);
	
	if(fsType == FS_HIDE){
		p = (PCHAR)ExAllocatePool(NonPagedPool, l + Dir.Length + sizeof(WCHAR));
		RtlMoveMemory(p, Dir.Buffer, Dir.Length);
		if(l > 0)
			RtlMoveMemory(p+Dir.Length, p1, l);

		if(l){
			*((PWCH)(p + Dir.Length + l)) = '\\';
			FileName.Length = Dir.Length + l + sizeof(WCHAR);
		}else
			FileName.Length = Dir.Length + l;

		FileName.Buffer = (PWCH)p;
	}else{
		FileName.Length = l;
		if(l>0){
			FileName.Buffer = (PWCH)ExAllocatePool(NonPagedPool, FileName.Length);
			RtlMoveMemory(FileName.Buffer, p1, FileName.Length);
		}
	}
	*/
	Volume.Length = pVolume->Length;
	Volume.Buffer = (PWCH)ExAllocatePool(NonPagedPool, Volume.Length);
	RtlMoveMemory(Volume.Buffer, pVolume->Buffer, Volume.Length);


// 	if(fsType == FS_RENAME){
// 		isDir = FileIsDirectory(pName);
// 	}

	AcquireSpinLock(&g_StackLock);
	__try{
		switch(fsType){
		case FS_HIDE:
			ret = GetFsHideFiles(&Volume, &FileName, fileNames);
			return ret;
		case FS_CREATE:
			pVolumeStack = g_FsCreate_Stack;
			break;
		case FS_RENAME:
			pVolumeStack = g_FsRename_Stack;
			break;
		case FS_DELETE:
			pVolumeStack = g_FsDelete_Stack;
			break;
		case FS_WRITE:
			pVolumeStack = g_FsWrite_Stack;
			break;
		}
	
		if(FileName.Length == 0)
			return 0;
		else
			return IsProtectFile(fsType, pVolumeStack, &Volume, &FileName);

	}__finally{
		ReleaseSpinLock(&g_StackLock);
		if(Volume.Buffer)
			ExFreePool(Volume.Buffer);
//		if(Dir.Buffer)
//			ExFreePool(Dir.Buffer);
		if(FileName.Buffer)
			ExFreePool(FileName.Buffer);
	}
}

#pragma endregion 文件过滤处理
