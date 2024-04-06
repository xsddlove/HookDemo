#include <string>
#include <windows.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib,"ntdll.lib")

using namespace std;

//根据进程名字获取pid
DWORD GetPidFromName(wstring wsProcessName) {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	PROCESSENTRY32W pe = { sizeof(pe) };
	BOOL bOk;
	for (bOk = Process32FirstW(hSnapshot, &pe); bOk; bOk = Process32NextW(hSnapshot, &pe)) {
		wstring  wsNowProcName = pe.szExeFile;
		if (StrStrI(wsNowProcName.c_str(), wsProcessName.c_str()) != NULL) {
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	CloseHandle(hSnapshot);
	return 0;
}
//把wcCacheInDllPath DLL文件注入进程wsProcessName
BOOL Injection_APC(const wstring& wsProcessName, const WCHAR wcCacheInDllPath[]) {

	DWORD dwProcessId = GetPidFromName(wsProcessName);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	PVOID  lpData = VirtualAllocEx(hProcess,
		NULL,
		1024,
		MEM_COMMIT,
		PAGE_EXECUTE_READWRITE);
	DWORD dwRet;

	//在远程进程申请空间中写入待注入DLL的路径  
	WriteProcessMemory(hProcess,
		lpData,
		(LPVOID)wcCacheInDllPath,
		MAX_PATH, NULL);

	CloseHandle(hProcess);
	//开始注入
	THREADENTRY32 te = { sizeof(THREADENTRY32) };
	//得到线程快照  
	HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	BOOL bStat = FALSE;
	//得到第一个线程  
	if (Thread32First(handleSnap, &te)) {
		do {
			//进行进程ID对比  
			if (te.th32OwnerProcessID == dwProcessId) {
				//得到线程句柄  
				HANDLE handleThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
				if (handleThread) {
					//向线程插入APC  
					DWORD dwRet = QueueUserAPC(
						(PAPCFUNC)LoadLibraryW,
						handleThread,
						(ULONG_PTR)lpData);
					if (dwRet > 0) {
						bStat = TRUE;
					}
					//关闭句柄  
					CloseHandle(handleThread);
				}
			}
		}
		//循环下一个线程 
		while (Thread32Next(handleSnap, &te));
	}
	CloseHandle(handleSnap);
	return bStat;
}


int main(int argc, char* argv[]) {
	Injection_APC(L"TTY3D.exe", L"C:\\Users\\Administrator\\Downloads\\HookDemo\\Debug\\HookDemo.dll");
	return 0;
}
