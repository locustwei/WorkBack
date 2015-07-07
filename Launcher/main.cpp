//

#include "windows.h" 
#include <tchar.h>
#include <stdio.h>

//注入线程函数参数结构。由于线程运行在MT4进程空间，所有变量必须预先写入MT4进程内存中。
typedef struct _RemoteParam {  
	wchar_t szDllName[100];//dll名称
    char szProcName[100]; // 函数名称   
	DWORD pGetProcess;//GetProcess函数地址
    DWORD pLoadLibrary;//LoadLibrary函数的入口地址  
	DWORD dwThreadId;
} RemoteParam, * PRemoteParam;  

typedef HMODULE (__stdcall * LOADLIBRARY)(LPCTSTR);  
typedef FARPROC (__stdcall * GETPROCADDRESS) (HMODULE, LPCSTR);
typedef int (__stdcall * RUNPLUGIN)(DWORD);

//注入MT4进程的线程函数定义
//注入后加载MT4Plug.dll，调用RunMt4Plug函数。
int __stdcall threadProc(PRemoteParam param)  
{  
	if(param!=NULL){
		LOADLIBRARY pLoadLibrary = (LOADLIBRARY)(param->pLoadLibrary);
		if(!pLoadLibrary){
			//pMessageBox(NULL, param->szDllName, NULL, 0);
			return 1;
		}
		HMODULE hMt4Plug=pLoadLibrary(param->szDllName);
		if(!hMt4Plug){
			//pMessageBox(NULL, param->szDllName, NULL, 0);
			return 2;
		}
		GETPROCADDRESS pGetProcAddress=(GETPROCADDRESS)param->pGetProcess;
		if(!pGetProcAddress){
			//pMessageBox(NULL, param->szDllName, NULL, 0);
			return 3;
		}
		RUNPLUGIN pRunlugin=(RUNPLUGIN)pGetProcAddress(hMt4Plug, param->szProcName);
		if(!pRunlugin){
			//pMessageBox(NULL, param->szDllName, NULL, 0);
			return 4;
		}

		((RUNPLUGIN)pRunlugin)(param->dwThreadId);

		
		/*
		while (((GETMESSAGE)(param->pGetMessaage))(&param->msg, NULL, 0, 0))
		{
			((TRANSLATEMESSAGE)(param->pTranslateMessage))(&param->msg);
			((DISPATCHMESSAGE)(param->pDispatchMessage))(&param->msg);
		}
		*/
	}
	
    return 0;  
}  
/*
//根据进程名称得到进程ID,如果有多个运行实例的话，返回第一个枚举到的进程的ID   
DWORD processNameToId(LPCTSTR lpszProcessName)  
{  
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  
    PROCESSENTRY32 pe;  
    pe.dwSize = sizeof(PROCESSENTRY32);  
    if (!Process32First(hSnapshot, &pe)) {  
        printf(NULL,   
            "The frist entry of the process list has not been copyied to the buffer",   
           "Notice", MB_ICONINFORMATION | MB_OK);  
        return 0;  
    }  
    while (Process32Next(hSnapshot, &pe)) {  
        if (!strcmp(lpszProcessName, pe.szExeFile)) {  
            return pe.th32ProcessID;  
        }  
    }  
   
    return 0;  
}  
*/

//提升进程访问权限   
bool enableDebugPriv(void)  
{  
    HANDLE hToken;  
    LUID sedebugnameValue;  
    TOKEN_PRIVILEGES tkp;  
    
    if (!OpenProcessToken(GetCurrentProcess(),   
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {  
        return false;  
    }  
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) {  
        CloseHandle(hToken);  
        return false;  
    }  
    tkp.PrivilegeCount = 1;  
    tkp.Privileges[0].Luid = sedebugnameValue;  
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL)) {  
        CloseHandle(hToken);  
        return false;  
    }  
    return true;  
}  
  

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwThreadSize=8192; //注入线程函数的字节数。不计算具体大小，数字大点，小了容易出错。

	if(argc<2){
		printf("请输入宿主程序！");
		getchar();
		return 0;
	}
		

	//提升权限。使用CreateProcess运行子进程此函数可省去
	//if(!enableDebugPriv()){
		//printf(NULL, _T("提升权限失败"), _T("错误！"), MB_ICONERROR | MB_OK);
		//return 0;
	//}

	/***创建被注入的进程******/
	PROCESS_INFORMATION procInfo;
	STARTUPINFO starupInfo;

	ZeroMemory(&procInfo, sizeof(procInfo));
	ZeroMemory(&starupInfo, sizeof(starupInfo));
	starupInfo.cb=sizeof(starupInfo);
	if(!CreateProcess(argv[1], NULL, NULL, NULL, true, CREATE_SUSPENDED, NULL, NULL, &starupInfo, &procInfo)){
		printf("未找到程序");
		return 0;
	}
	HANDLE hTargetProcess = procInfo.hProcess;
	
	Sleep(1000); //等待进程运行起来。
	getchar();
	
	ResumeThread(procInfo.hThread);
	return 0;

    //在宿主进程中为线程体开辟一块存储区域   
    //在这里需要注意MEM_COMMIT | MEM_RESERVE内存非配类型以及PAGE_EXECUTE_READWRITE内存保护类型   
    void* pRemoteThread = VirtualAllocEx(hTargetProcess, 0, dwThreadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);  
    if (!pRemoteThread) {  
        printf("宿主进程分配内存失败 !");  
        return 0;  
    }  
    //将线程体拷贝到宿主进程中   
    if (!WriteProcessMemory(hTargetProcess,   pRemoteThread, &threadProc, dwThreadSize, 0)) {  
        printf("向宿主进程写入线程函数失败 !");  
        return 0;  
    }  
	
	WCHAR p[255] = {0};
	DWORD len  = GetModuleFileName(NULL, p, 255);
	for(int i=len; ;i--)
		if(p[i]=='\\')
			break;
		else
			p[i] = 0;
    //线程参数赋值   
    RemoteParam remoteData;  
    ZeroMemory(&remoteData, sizeof(RemoteParam));  
	remoteData.dwThreadId=procInfo.dwThreadId;
	HINSTANCE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	wcscpy_s(remoteData.szDllName, p);
	wcscat_s(remoteData.szDllName, L"AddinTDX.dll");;
	strcpy_s(remoteData.szProcName, "RunPlugin");
	remoteData.pLoadLibrary = (DWORD)GetProcAddress(hKernel32, "LoadLibraryW"); 
	remoteData.pGetProcess = (DWORD)GetProcAddress(hKernel32, "GetProcAddress");

	
    //线程参数写入宿主进程中   
    RemoteParam* pRemoteParam = (RemoteParam*)VirtualAllocEx( hTargetProcess , 0, sizeof(RemoteParam), MEM_COMMIT, PAGE_READWRITE); 
    if (!pRemoteParam) {  
        printf("宿主进程分配参数地址失败 !");  
        return 0;  
    }

    if (!WriteProcessMemory(hTargetProcess , pRemoteParam, &remoteData, sizeof(RemoteParam), 0)) {  
        printf("向宿主进程写入参数失败 !");  
        return 0;  
    }  

    //在宿主进程运行线程   
    HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess, NULL, 0, (DWORD (__stdcall *)(void *))pRemoteThread, pRemoteParam, 0, NULL);  
    if (!hRemoteThread) {  
        printf("启动插件线程失败 !");  
        return 0;   
    }  

	
	
    CloseHandle(hRemoteThread);  
	CloseHandle(hTargetProcess);
	return 0;
}

