#pragma once

#include <ntddk.h>
//#include <ntstrsafe.h>

typedef struct _SPIN_LOCK   //зда§Ыј
{
	KSPIN_LOCK  SpinLock;
	KIRQL       Irql;
} SPIN_LOCK, *PSPIN_LOCK;

#define AcquireSpinLock(_SpinLock)  KeAcquireSpinLock(&(_SpinLock)->SpinLock, &(_SpinLock)->Irql)
#define ReleaseSpinLock(_SpinLock)  KeReleaseSpinLock(&(_SpinLock)->SpinLock,(_SpinLock)->Irql)

#ifndef TAG_NAME
#define TAG_NAME 'iwse'
#endif

#define MM_ALLOC(size) ExAllocatePoolWithTag(NonPagedPool, size, TAG_NAME)
#define MM_FREE(p) ExFreePoolWithTag(p, TAG_NAME); p = NULL

#define InfoPrint(str, ...)                 \
	DbgPrint("%S: "##str"\n",             \
	DRIVER_NAME,                 \
	__VA_ARGS__)

#define ErrorPrint(str, ...)                \
	DbgPrint("%S: %u: "##str"\n",         \
	DRIVER_NAME,                 \
	__LINE__,                    \
	__VA_ARGS__)

#define SYSTEM_ROOT_SYMBOLIC		L"\\SystemRoot"

typedef NTSTATUS (*EnumerateRegValueKeyCallback)(PVOID);

NTSTATUS ZwQueryInformationProcess(
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	UINT32 ProcessInformationLength,
	PUINT32 ReturnLength
	);
NTSTATUS GetSymbolicLink(PUNICODE_STRING SymbolicLinkName, PUNICODE_STRING SymbolicLink);
PUNICODE_STRING GetProcessImageFileName(HANDLE pid);
PUNICODE_STRING GetObjectNameString(PVOID pObject);

BOOLEAN CopyUnicodeString(PUNICODE_STRING pDest, PUNICODE_STRING pSource);
BOOLEAN UnicodeStringCatString(PUNICODE_STRING pDest, PWCH pSource, UINT32 nMax);
BOOLEAN UnicodeStringCat(PUNICODE_STRING pDest, PUNICODE_STRING pSource);
BOOLEAN UnicodeStringCopyString(PUNICODE_STRING pDest, PWCH pSource, UINT32 nMax);

NTSTATUS CreateRegKey(LPWSTR KeyName, HANDLE* hKey);
NTSTATUS ClearRegisterKeyValues(HANDLE hKey);
NTSTATUS GetRegKeyValue(LPWSTR KeyName, LPWSTR ValueName, KEY_VALUE_INFORMATION_CLASS ValueClass, PVOID* pData);

NTSTATUS GetSystemRootPath(PUNICODE_STRING SystemRootPath);

VOID FreeUtilsMemory(PVOID pMemory);