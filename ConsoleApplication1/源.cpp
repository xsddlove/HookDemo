#include <string>
#include <windows.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib,"ntdll.lib")

using namespace std;

//���ݽ������ֻ�ȡpid
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
//��wcCacheInDllPath DLL�ļ�ע�����wsProcessName
BOOL Injection_APC(const wstring& wsProcessName, const WCHAR wcCacheInDllPath[]) {

	DWORD dwProcessId = GetPidFromName(wsProcessName);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	PVOID  lpData = VirtualAllocEx(hProcess,
		NULL,
		1024,
		MEM_COMMIT,
		PAGE_EXECUTE_READWRITE);
	DWORD dwRet;

	//��Զ�̽�������ռ���д���ע��DLL��·��  
	WriteProcessMemory(hProcess,
		lpData,
		(LPVOID)wcCacheInDllPath,
		MAX_PATH, NULL);

	CloseHandle(hProcess);
	//��ʼע��
	THREADENTRY32 te = { sizeof(THREADENTRY32) };
	//�õ��߳̿���  
	HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	BOOL bStat = FALSE;
	//�õ���һ���߳�  
	if (Thread32First(handleSnap, &te)) {
		do {
			//���н���ID�Ա�  
			if (te.th32OwnerProcessID == dwProcessId) {
				//�õ��߳̾��  
				HANDLE handleThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
				if (handleThread) {
					//���̲߳���APC  
					DWORD dwRet = QueueUserAPC(
						(PAPCFUNC)LoadLibraryW,
						handleThread,
						(ULONG_PTR)lpData);
					if (dwRet > 0) {
						bStat = TRUE;
					}
					//�رվ��  
					CloseHandle(handleThread);
				}
			}
		}
		//ѭ����һ���߳� 
		while (Thread32Next(handleSnap, &te));
	}
	CloseHandle(handleSnap);
	return bStat;
}


int main(int argc, char* argv[]) {
	Injection_APC(L"TTY3D.exe", L"C:\\Users\\Administrator\\Downloads\\HookDemo\\Debug\\HookDemo.dll");
	return 0;
}
