
#pragma once

typedef enum _FS_FILE_TYPE{
	FS_HIDE  = 0x1,
	FS_WRITE = 0x2,
	FS_DELETE = 0x4,
	FS_RENAME = 0x8,
	FS_CREATE = 0x10,
}FS_FILE_TYPE;

#pragma pack(push, 1)

typedef struct _FS_FILE_RECORD{  //隐藏文件记录。
	ULONG cb;                    //记录字节数
	ULONGLONG id;                //
	BOOLEAN bDisable;
	FS_FILE_TYPE pt;
	USHORT nVolume;
	USHORT nPath;
	USHORT nName;
	WCHAR Data[1];
}FS_FILE_RECORD, *PFS_FILE_RECORD;

typedef struct _SET_ENABLE_INPUT{           //设置临时可见
	ULONGLONG id;
	BOOLEAN bDisable;
}SET_ENABLE_INPUT, *PSET_ENABLE_INPUT;

#pragma pack(pop)

typedef struct _FS_FILE_STACK{
	//PFS_FILE_RECORD pFile;
	ULONG RecordId;
	struct _FS_FILE_STACK* pPriv;
	struct _FS_FILE_STACK* pNext;
}FS_FILE_STACK, *PFS_FILE_STACK;

typedef struct _FS_FILE_VOLUME_STACK{
	PWSTR Volume;
	PFS_FILE_STACK pFileStack;
	struct _FS_FILE_VOLUME_STACK *pNext;
	struct _FS_FILE_VOLUME_STACK *pPriv;
}FS_FILE_VOLUME_STACK, *PFS_FILE_VOLUME_STACK;


typedef struct _CONFIG_FILE_HEADER{
	ULONG FileCount;
}CONFIG_FILE_HEADER, *PCONFIG_FILE_HEADER;

#define GETFILE_VOLUME(v) (PCWSTR)((PCHAR)v+v->nVolume)
#define GETFILE_PATH(v) (PCWSTR)((PCHAR)v+v->nPath)
#define GETFILE_NAME(v) (PCWSTR)((PCHAR)v+v->nName)

VOID LoadFsData();
VOID FsFreeData();
NTSTATUS LoadConfigFile();
NTSTATUS SaveConfigFile();
int GetFsFiles(IN FS_FILE_TYPE fsType, IN PUNICODE_STRING pVolume, IN PUNICODE_STRING pParentDir, IN PUNICODE_STRING pName, OUT PWSTR** fileNames);
NTSTATUS EnumFsFiles(OUT PVOID Buffer, ULONG* BufferLength);
NTSTATUS UpdateFsFile(PFS_FILE_RECORD pFile);
NTSTATUS FsEnableAll(BOOLEAN bDisable);
NTSTATUS SetFsFileEnable(ULONGLONG id, BOOLEAN bDisable);
NTSTATUS RemoveFsFile(ULONGLONG id);
NTSTATUS AddFsFile(PFS_FILE_RECORD pHideFile, OUT ULONGLONG* id);

