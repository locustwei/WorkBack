
#include <fltKernel.h>
#include "CtrlDevice.h"
#include "filedisk.h"
#include "Driver.h"
#include "FltCallback.h"
#include "FsData.h"
#include "..\..\WiseRegNotify\DirverUtils.h"

#pragma region 隐藏文件

/*
 *	Windows XP 以上版本
 */
/*
int ScanHideFile_id(PFILE_ID_BOTH_DIR_INFORMATION  pInfo, ULONG length, PCWSTR pFile)
{
	int modified, len;
	PFILE_ID_BOTH_DIR_INFORMATION currentFileInfo = pInfo, nextFileInfo = NULL, previousFileInfo = NULL;

	if(pInfo==NULL)
		return 0;
	len = wcslen(pFile) * sizeof(WCHAR);
	do 
	{
		if(currentFileInfo->NextEntryOffset!=0)
			nextFileInfo = (PFILE_ID_BOTH_DIR_INFORMATION)((PCHAR)(currentFileInfo) + currentFileInfo->NextEntryOffset);
		else
			nextFileInfo = NULL;


		if(currentFileInfo->FileNameLength==len && _wcsnicmp(currentFileInfo->FileName, pFile, len / sizeof(WCHAR))==0){ 
			modified = 1;
			if(previousFileInfo == NULL){ //第一个文件
				if(currentFileInfo->NextEntryOffset == 0){ //只有一个要隐藏的文件
					modified = 2; //STATUS_NO_MORE_FILES
					break;
				}else{ //把下一个Copy上来
					RtlCopyMemory(currentFileInfo, nextFileInfo, length - currentFileInfo->NextEntryOffset);
					nextFileInfo = currentFileInfo;
					currentFileInfo = NULL;
				}
			}else{ //前一个指向下一个
				if(currentFileInfo->NextEntryOffset==0){//最后一个
					previousFileInfo->NextEntryOffset = 0;
					break;
				}else
					previousFileInfo->NextEntryOffset = previousFileInfo->NextEntryOffset + currentFileInfo->NextEntryOffset;

				currentFileInfo = previousFileInfo;
			}
		}

		previousFileInfo = currentFileInfo;		
		currentFileInfo = nextFileInfo;
	} while (currentFileInfo!=NULL);

	return modified;
}
*/
/*
 *	Windows XP
 */
/*
int ScanHideFile(PFILE_BOTH_DIR_INFORMATION pInfo, ULONG length, PCWSTR pFile)
{
	int modified=0, len;
	PFILE_BOTH_DIR_INFORMATION currentFileInfo = pInfo, nextFileInfo = NULL, previousFileInfo = NULL;

	if(pInfo==NULL)
		return 0;
	len = wcslen(pFile) * sizeof(WCHAR);

	do 
	{
		if(currentFileInfo->NextEntryOffset!=0)
			nextFileInfo = (PFILE_BOTH_DIR_INFORMATION)((PCHAR)(currentFileInfo) + currentFileInfo->NextEntryOffset);
		else
			nextFileInfo = NULL;

		if(currentFileInfo->FileNameLength==len && _wcsnicmp(currentFileInfo->FileName, pFile, len / sizeof(WCHAR))==0){ 

			modified = 1;
			if(previousFileInfo == NULL){ //第一个文件
				if(currentFileInfo->NextEntryOffset == 0){ //只有一个要隐藏的文件
					modified = 2; //STATUS_NO_MORE_FILES

					break;
				}else{ //把下一个Copy上来
					RtlCopyMemory(currentFileInfo, nextFileInfo, length - currentFileInfo->NextEntryOffset);
					nextFileInfo = currentFileInfo;
					currentFileInfo = NULL;
				}
			}else{ //前一个指向下一个
				if(currentFileInfo->NextEntryOffset==0){//最后一个
					previousFileInfo->NextEntryOffset = 0;
					break;
				}else
					previousFileInfo->NextEntryOffset = previousFileInfo->NextEntryOffset + currentFileInfo->NextEntryOffset;

				currentFileInfo = previousFileInfo;
			}
		}
		previousFileInfo = currentFileInfo;		
		currentFileInfo = nextFileInfo;
	} while (currentFileInfo!=NULL);

	return modified;
}
*/

int ScanHideFile(FILE_INFORMATION_CLASS infocls, PUCHAR pInfo, ULONG length, PCWSTR pFile)
{
	int modified=0, len;
	PUCHAR currentFileInfo = pInfo, nextFileInfo = NULL, previousFileInfo = NULL;
	ULONG NextEntryOffset = 0;
	BOOLEAN equ;

	if(pInfo==NULL)
		return 0;

	len = wcslen(pFile) * sizeof(WCHAR);

	do 
	{
		NextEntryOffset = *(PULONG)currentFileInfo;

		if(NextEntryOffset >= length)
			break;

		if(NextEntryOffset != 0)
			nextFileInfo = currentFileInfo + NextEntryOffset;
		else
			nextFileInfo = NULL;

		equ = FALSE;

		
		switch(infocls){
		case FileBothDirectoryInformation:
		case FileIdBothDirectoryInformation:
			if(Globals.WinVer == 5){
				equ = ((PFILE_BOTH_DIR_INFORMATION)currentFileInfo)->FileNameLength==len && _wcsnicmp(((PFILE_BOTH_DIR_INFORMATION)currentFileInfo)->FileName, pFile, len / sizeof(WCHAR))==0;
			}else if(Globals.WinVer>5)
				equ = ((PFILE_ID_BOTH_DIR_INFORMATION)currentFileInfo)->FileNameLength==len && _wcsnicmp(((PFILE_ID_BOTH_DIR_INFORMATION)currentFileInfo)->FileName, pFile, len / sizeof(WCHAR))==0;
			break;
		case FileFullDirectoryInformation:
			equ = ((PFILE_FULL_DIR_INFORMATION)currentFileInfo)->FileNameLength==len && _wcsnicmp(((PFILE_FULL_DIR_INFORMATION)currentFileInfo)->FileName, pFile, len / sizeof(WCHAR))==0;
			//InfoPrint("FileName %S", ((PFILE_FULL_DIR_INFORMATION)currentFileInfo)->FileName);
			break;
		case FileIdFullDirectoryInformation:
			equ = ((PFILE_ID_FULL_DIR_INFORMATION)currentFileInfo)->FileNameLength==len && _wcsnicmp(((PFILE_ID_FULL_DIR_INFORMATION)currentFileInfo)->FileName, pFile, len / sizeof(WCHAR))==0;
			//InfoPrint("FileName %S", ((PFILE_ID_FULL_DIR_INFORMATION)currentFileInfo)->FileName);
			break;
		default:
			return 0;
		}

		if(equ){
			modified = 1;
			if(previousFileInfo == NULL){ //第一个文件
				if(NextEntryOffset == 0){ //只有一个要隐藏的文件
					modified = 2; //STATUS_NO_MORE_FILES
					break;
				}else{ //把下一个Copy上来
					RtlCopyMemory(currentFileInfo, nextFileInfo, length - NextEntryOffset);
					nextFileInfo = currentFileInfo;
					//currentFileInfo = NULL;
				}
			}else{ //前一个指向下一个
				if(NextEntryOffset==0){//最后一个
					*(PULONG)previousFileInfo = 0;
					break;
				}else
					*(PULONG)previousFileInfo = *(PULONG)previousFileInfo + NextEntryOffset;

				currentFileInfo = previousFileInfo;
			}
		}
		previousFileInfo = currentFileInfo;		
		currentFileInfo = nextFileInfo;
		length -= NextEntryOffset;

	} while (NextEntryOffset);

	return modified;
}

int RemovQueryDirectoryRecord(FILE_INFORMATION_CLASS infocls, PFLT_CALLBACK_DATA Data, int count, PWSTR* pFiles)
{

	PVOID SafeBuffer=NULL;
	int modified = 0, i;

	if(count <= 0)
		return 0;

	if (Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress != NULL){
		SafeBuffer=MmGetSystemAddressForMdlSafe( Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress,NormalPagePriority );            
	}else{
		SafeBuffer=Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;             
	}          
	
	if(SafeBuffer==NULL){
		return 0;
	}	

	for(i=0; i<count; i++){
		modified |= ScanHideFile(infocls, (PUCHAR)SafeBuffer, Data->Iopb->Parameters.DirectoryControl.QueryDirectory.Length, pFiles[i]);
		if(modified == 2)
			break;
	}

	return modified;
}

#pragma region 兼容就系统
/*
#define  HideFileName_FAT   L"..."  
#define  HideFileName_NTFS  L"...."
#define HideFileName_XP     L"\0x7F\0x7F,0x7F\0x7F\0x7F\0x7F\0x7F\0x7F"

//const wchar_t HideFileName_XP[]={0x7F, 0x7F,0x7F, 0x7F,0x7F, 0x7F,0x7F, 0x7F};//

#define  NameLen_FAT 6
#define  NameLen_NTFS 8
#define  NameLen_XP 16

PWSTR ROOTPATHS[] = {HideFileName_FAT, HideFileName_NTFS, HideFileName_XP};

BOOLEAN IsHidePath(PFLT_FILE_NAME_INFORMATION pNameInfo)
{
	ULONG nLength = pNameInfo->Name.Length - pNameInfo->Volume.Length;

	//DbgPrint("IsHidePath");

	if(nLength > NameLen_FAT){
		if(_wcsnicmp((PWSTR)(pNameInfo->Name.Buffer + pNameInfo->Volume.Length / sizeof(WCHAR) + 1), HideFileName_FAT, NameLen_FAT / sizeof(WCHAR)) == 0)
			return TRUE;
	}
	if(nLength > NameLen_NTFS){
		if(_wcsnicmp((PWSTR)(pNameInfo->Name.Buffer + pNameInfo->Volume.Length / sizeof(WCHAR) + 1), HideFileName_NTFS, NameLen_NTFS / sizeof(WCHAR)) == 0)
			return TRUE;
	}
	if(nLength > NameLen_XP){
		if(_wcsnicmp((PWSTR)(pNameInfo->Name.Buffer + pNameInfo->Volume.Length / sizeof(WCHAR) + 1), HideFileName_XP, NameLen_XP / sizeof(WCHAR)) == 0)
			return TRUE;
	}
	return FALSE;
}

NTSTATUS ProcessSpecialPath(PFLT_CALLBACK_DATA Data, PFLT_FILE_NAME_INFORMATION NameInfo)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int modified = 0;

	if(NameInfo->Name.Length == NameInfo->Volume.Length + sizeof(WCHAR)){ // 根目录
		modified = RemovQueryDirectoryRecord(Data, 3, ROOTPATHS);
		if(modified == 2){
			if(Globals.WinVer > 5)    //xp 对 “..."目录处理特殊
				Status = STATUS_NO_MORE_FILES;
		}else if (modified == 1)
			Status = STATUS_SINGLE_STEP;  //后续处理
		
	}else{
		if(IsHidePath(NameInfo))
			Status = STATUS_NO_MORE_FILES;
	}

	return Status;
}
*/
#pragma endregion 兼容就系统

FLT_POSTOP_CALLBACK_STATUS PostDirCtrlCallBack ( 
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in_opt PVOID CompletionContext,
	__in FLT_POST_OPERATION_FLAGS Flags )
{
	PFLT_FILE_NAME_INFORMATION NameInfo = NULL;
	NTSTATUS Status;
	int count = 0, modified = 0;
	PWSTR* pFiles = NULL;

	UNREFERENCED_PARAMETER( FltObjects );
	UNREFERENCED_PARAMETER( CompletionContext );

	
	if(KeGetCurrentIrql() > APC_LEVEL)
		return FLT_POSTOP_FINISHED_PROCESSING;
	
	if( FlagOn( Flags, FLTFL_POST_OPERATION_DRAINING ) ){
		return FLT_POSTOP_FINISHED_PROCESSING;
	}

	if( Data->Iopb->MinorFunction == IRP_MN_QUERY_DIRECTORY && 
		(Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass == FileBothDirectoryInformation || 
			Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass ==FileIdBothDirectoryInformation ||
			Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass ==FileFullDirectoryInformation ||
			Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass ==FileIdFullDirectoryInformation) && 
		Data->Iopb->Parameters.DirectoryControl.QueryDirectory.Length > 0 &&
		NT_SUCCESS(Data->IoStatus.Status)){

		if(g_WiseProcessId && PsGetCurrentProcessId() == g_WiseProcessId)
			return FLT_POSTOP_FINISHED_PROCESSING;

		do 
		{
			Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &NameInfo);
				
			if(!NT_SUCCESS(Status) || NameInfo == NULL)
				break;

			Status = FltParseFileNameInformation(NameInfo);
			if(!NT_SUCCESS(Status)){
				break;
			}
				//Status = ProcessSpecialPath(Data, NameInfo);  //老系统处理特殊目录
				// 				switch(Status){
				// 				case STATUS_SUCCESS:
				// 				case STATUS_SINGLE_STEP:
			
			//InfoPrint("PostDirCtrlCallBack-------------------------------------");
			//InfoPrint("GetFsFiles %wZ %wZ %wZ",  &NameInfo->Volume, &NameInfo->ParentDir, &NameInfo->Name);
			count = GetFsFiles(FS_HIDE, &NameInfo->Volume, &NameInfo->ParentDir, &NameInfo->Name, &pFiles);
			
			//InfoPrint("PostDirCtrlCallBack*************************************");

			if(count == -1){
				Data->IoStatus.Status = STATUS_NO_MORE_FILES;
			}else if(count > 0){
				modified = RemovQueryDirectoryRecord(Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass, Data, count, pFiles);
			}
			if(pFiles)
				ExFreePool(pFiles);	
			if(modified == 2){
				Data->IoStatus.Status = STATUS_NO_MORE_FILES;
				Data->IoStatus.Information = 0;
			}
			else if(modified == 1 || Status == STATUS_SINGLE_STEP){
				FltSetCallbackDataDirty( Data );
			}

				// 					break;
				// 				case STATUS_NO_MORE_FILES:
				// 					Data->IoStatus.Status = Status;
				// 					break;
				// 				}
		} while (FALSE);

		if(NameInfo)
			FltReleaseFileNameInformation(NameInfo);
	}
	
	return FLT_POSTOP_FINISHED_PROCESSING;
}

#pragma endregion 隐藏文件

#pragma region 文件保护

FLT_PREOP_CALLBACK_STATUS PreFileSetInformation(
	__inout PFLT_CALLBACK_DATA    Data,
	__in    PCFLT_RELATED_OBJECTS FltObjects,
	OUT   PVOID                 *CompletionContext
	)
{
	NTSTATUS Status;
	FLT_PREOP_CALLBACK_STATUS ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
	FLT_FILE_NAME_INFORMATION soureFile;
	PFLT_FILE_NAME_INFORMATION NameInfo = NULL;
	FS_FILE_TYPE fst = FS_CREATE;

    UNREFERENCED_PARAMETER( CompletionContext );

	if(KeGetCurrentIrql() > APC_LEVEL)
		return ret;
	do 
	{
		if(g_WiseProcessId && PsGetCurrentProcessId() == g_WiseProcessId){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}

		if(Data->IoStatus.Status != STATUS_SUCCESS){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}
		
		if(Data->Iopb->Parameters.SetFileInformation.FileInformationClass != FileRenameInformation &&
			Data->Iopb->Parameters.SetFileInformation.FileInformationClass != FileDispositionInformation){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}

		if((FltObjects->FileObject==NULL)||(FltObjects->FileObject->FileName.Length==0)||(FltObjects->FileObject->FileName.Buffer==NULL)){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}


		Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &NameInfo);
		if(!NT_SUCCESS(Status) || NameInfo == NULL){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}

		Status = FltParseFileNameInformation(NameInfo);
		if(!NT_SUCCESS(Status)){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}
		
		switch(Data->Iopb->Parameters.SetFileInformation.FileInformationClass){
		case FileRenameInformation:
			fst = FS_RENAME;
			break;
		case FileDispositionInformation:
			fst = FS_DELETE;
			break;
		};

		if(GetFsFiles(fst, &NameInfo->Volume, &NameInfo->ParentDir, &NameInfo->Name, NULL)){
			Data->IoStatus.Status = STATUS_ACCESS_DENIED;
			ret = FLT_PREOP_COMPLETE;
		}
	} while (FALSE);
	
	if(NameInfo)
		FltReleaseFileNameInformation(NameInfo);

	return ret;
}

FLT_PREOP_CALLBACK_STATUS PreFileCreate(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PFLT_FILE_NAME_INFORMATION NameInfo;
	FLT_PREOP_CALLBACK_STATUS ret = FLT_PREOP_SUCCESS_NO_CALLBACK;

	UNREFERENCED_PARAMETER( CompletionContext );
	PAGED_CODE();

	if(KeGetCurrentIrql() > APC_LEVEL)
		return ret;

	if(g_WiseProcessId && PsGetCurrentProcessId() == g_WiseProcessId)
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	do 
	{
		Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT | FLT_FILE_NAME_ALLOW_QUERY_ON_REPARSE, &NameInfo);
		if(!NT_SUCCESS(Status) || NameInfo == NULL){
			break;
		}
		/*
		Status = FltParseFileNameInformation(NameInfo);
		if(!NT_SUCCESS(Status)){
			ret = FLT_PREOP_SUCCESS_NO_CALLBACK;
			break;
		}
		*/
		if(GetFsFiles(FS_CREATE, &NameInfo->Volume, &NameInfo->ParentDir, &NameInfo->Name, NULL)){
			Data->IoStatus.Status = STATUS_NO_SUCH_FILE;
			ret = FLT_PREOP_COMPLETE;
			break;
		}
	} while (FALSE);


	return ret;
}

#pragma endregion 文件保护

NTSTATUS Unload (__in FLT_FILTER_UNLOAD_FLAGS Flags)
{
	UNREFERENCED_PARAMETER( Flags );  //无意义操作只是为消除警告信息

	PAGED_CODE();  //确保调用线程运行在一个允许分页的足够低IRQL级别。（仅Debug时有意义）

	if(Globals.FilterHandle){
		FltUnregisterFilter( Globals.FilterHandle );
		Globals.FilterHandle = NULL;
	}
	
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

FLT_OPERATION_REGISTRATION Callbacks[] = {
	
	{
		IRP_MJ_CREATE,
		0,
		PreFileCreate,
		NULL,
		NULL
	},
	
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
	
	{
		IRP_MJ_SET_INFORMATION,
		0,
		PreFileSetInformation,
		NULL,
		NULL
	},
	
	{ IRP_MJ_OPERATION_END }
};

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

NTSTATUS RegisiterFltCallback(PDRIVER_OBJECT DriverObject)
{
	NTSTATUS Status = FltRegisterFilter( DriverObject,&FilterRegistration,&Globals.FilterHandle );

	if (NT_SUCCESS( Status )) {
		LoadFsData();
		Status = FltStartFiltering( Globals.FilterHandle );   //驱动开始工作
	}
	return Status;
}

/*
FLT_POSTOP_CALLBACK_STATUS
	FL_-1back (
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in PVOID CompletionContext,
	__in FLT_POST_OPERATION_FLAGS Flags
	)
{


	PFILE_BOTH_DIR_INFORMATION					stDirInfo;
	PFILE_BOTH_DIR_INFORMATION			dir_info;

	if(Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass 
		== 3)
	{
#ifdef _ERROR_HIDE
		DbgPrint("ClassInformation  = 3\n");
#endif
		ptr = buffer;
		stDirInfo = 
			(PFILE_BOTH_DIR_INFORMATION)Data->Iopb->Parameters.DirectoryControl.QueryDirector
			y.DirectoryBuffer;		
		while(1)
		{
#ifdef _ERROR_HIDE
			DbgPrint("ClassInformation : Count --- > %d\n",iCount);
#endif

			iCount++;
			ProbeForRead(&stDirInfo->FileName[0],sizeof(WCHAR),1);
			uStr.Length = (USHORT)stDirInfo->FileNameLength;
			uStr.MaximumLength = uStr.Length;
			uStr.Buffer = &stDirInfo->FileName[0];

			if(stDirInfo->FileNameLength < 512 && stDirInfo->FileNameLength > 0)
			{
				uStrPath.Length = 0 ; 	
				uStrPath.MaximumLength = nameInfo->Name.Length + 520;
				uStrPath.Buffer = 
					(PWSTR)ExAllocatePoolWithTag(NonPagedPool,nameInfo->Name.Length + 520,'fst2');	

				uStrPathNext.Length = 0 ; 	
				uStrPathNext.MaximumLength = nameInfo->Name.Length + 520;
				uStrPathNext.Buffer = 
					(PWSTR)ExAllocatePoolWithTag(NonPagedPool,nameInfo->Name.Length + 520,'gst3');	

				if(uStrPathNext.Buffer != NULL && uStrPath.Buffer != NULL)
				{
					RtlAppendUnicodeStringToString(&uStrPath,&uStrGuidName);
					RtlAppendUnicodeStringToString(&uStrPath,&nameInfo->ParentDir);
					RtlAppendUnicodeStringToString(&uStrPath,&nameInfo->FinalComponent);
					RtlAppendUnicodeStringToString(&uStrPath,&uStEmpty);
					RtlAppendUnicodeStringToString(&uStrPath,&uStr);

					RtlAppendUnicodeStringToString(&uStrPathNext,&uStrGuidName);
					RtlAppendUnicodeStringToString(&uStrPathNext,&nameInfo->ParentDir);
					RtlAppendUnicodeStringToString(&uStrPathNext,&nameInfo->FinalComponent);
					RtlAppendUnicodeStringToString(&uStrPathNext,&uStr);

					flag = TRUE;

					if(Traverseinto_Hide_List(uStr,uStrPath,uStrPathNext)||
						Traverseinto_Stealth_List(uStrPath,uStrPathNext) || 
						Traverseinto_LockerHide_List(uStrPath,uStrPathNext))
					{	
						bCompared = TRUE;
						ExFreePoolWithTag(uStrPathNext.Buffer,'gst3');
						ExFreePoolWithTag(uStrPath.Buffer,'fst2');

						if (stDirInfo->NextEntryOffset==0)
						{
#ifdef _ERROR_HIDE
							DbgPrint("break.......\n");
#endif
							break;																
						} 
						stDirInfo=(PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo)+stDirInfo->NextEntryOffset);							
						continue;							
					}
					ExFreePoolWithTag(uStrPathNext.Buffer,'gst3');
					ExFreePoolWithTag(uStrPath.Buffer,'fst2');

					dir_info=(PFILE_BOTH_DIR_INFORMATION)ptr;
					RtlCopyMemory((PVOID)ptr, (PVOID)stDirInfo, 
						sizeof(*stDirInfo)-sizeof(WCHAR)+stDirInfo->FileNameLength);
					dir_info->NextEntryOffset=stDirInfo->NextEntryOffset;

					if (stDirInfo->NextEntryOffset!=0)
					{
						bytesreturned += stDirInfo->NextEntryOffset;			
					}
					else
					{
						bytesreturned += 
							sizeof(*stDirInfo)-sizeof(WCHAR)+stDirInfo->FileNameLength;				
					}
					ptr += stDirInfo->NextEntryOffset;
					if (stDirInfo->NextEntryOffset==0)
					{
						break;
					}
					stDirInfo=(PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo)+stDirInfo->Nex
						tEntryOffset);
				}// memory  allocation checking
				else
				{
					flag = FALSE;
					break;
				}
			}// file length checking		
		}// while end
		if(flag)
		{
#ifdef _ERROR_HIDE
			DbgPrint("Bytesretured......%d\n",bytesreturned);
#endif
			if (bytesreturned>0) 
				dir_info->NextEntryOffset=0;

			if(gMajOSVersion == 5 && (iCount<=2 && bCompared) && 
				Data->Iopb->OperationFlags != SL_RETURN_SINGLE_ENTRY)
			{
				Data->IoStatus.Status = STATUS_NO_MORE_FILES; // for single entry
			}
			else if(gMajOSVersion == 6 && iCount<=3 && bCompared && 
				Data->Iopb->OperationFlags != SL_RETURN_SINGLE_ENTRY)
				Data->IoStatus.Status = STATUS_NO_MORE_FILES; // for single entry


			Data->IoStatus.Information=bytesreturned;
			(ULONG)Data->Iopb->Parameters.DirectoryControl.QueryDirectory.Length = 
				bytesreturned;				
			RtlCopyMemory((PVOID)Data->Iopb->Parameters.DirectoryControl.QueryDirectory
				.DirectoryBuffer, (PVOID)buffer, bytesreturned);

		}
		if (NULL != nameInfo) 
		{
			FltReleaseFileNameInformation( nameInfo );
			nameInfo = NULL;
		}				
	} //PFILE_BOTH_DIR_INFORMATION
}
*/
