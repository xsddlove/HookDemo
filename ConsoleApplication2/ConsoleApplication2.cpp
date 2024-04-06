#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <process.h>

DWORD EnumerateThreads(DWORD processId) {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        std::cout << "CreateToolhelp32Snapshot failed" << std::endl;
        return 0;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32)) {
        std::cout << "Thread32First failed" << std::endl;
        CloseHandle(hThreadSnap);
        return 0;
    }

    std::cout << "Threads in process " << processId << ":" << std::endl;
    do {
        if (te32.th32OwnerProcessID == processId) {
            std::cout << "Thread ID: " << te32.th32ThreadID << std::endl;
            CloseHandle(hThreadSnap);
            return te32.th32ThreadID;
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
}

void thread_send(DWORD len, DWORD address) {
    //1,获取进程
    DWORD pid = 12796;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    //2,找到主线程
    DWORD Main_THREAD = EnumerateThreads(pid);
    //3,打开主线程
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, Main_THREAD);
    //4挂起线程
    DWORD suspendCount = SuspendThread(hThread);
    //5获取线程环境块
 // 获取线程环境块
    CONTEXT ctx;
    memset(&ctx, 0, sizeof(CONTEXT));
    ctx.ContextFlags = CONTEXT_ALL;
    if (GetThreadContext(hThread, &ctx) == 0) {
        printf("获取线程环境块失败，错误码：%d\n", GetLastError());
    }
    //printf("\t[+] 当前寄存器值：\n\t\tEIP[0x%08x] ESP[0x%08x]\n", ctx.Eip, ctx.Esp);
    //配置shellcode
    char shellcode[] = {
        0x68,0x00,0x00,0x00,0x00,       //push eip返回地址
        0x60,                           //pushad
        0x9C,                           //pushfd
        0x6A,0x00,                      //push 0
        0x68,0x00,0x00,0x00,0x00,        //push 参数二  
        0x68,0x00,0x00,0x00,0x00,        //push 缓冲区
        0xBE,0x00,0x00,0x00,0x00,        //mov esi,标志位
        0x8B,0xCE,                       //mov ecx,esi
        0xFF,0XD0,                       //call eax
        0x9D,                            //popfd
        0x61,                            //popad
        0xC3                             //ret
    };
    //申请空间shellcode
    PCHAR pData = (PCHAR)VirtualAllocEx(hProcess,NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    //修补eip
    *(DWORD*)&shellcode[1] = (DWORD)ctx.Eip;
    *(DWORD*)&shellcode[10] = (DWORD)0x1;
    *(DWORD*)&shellcode[15] = (DWORD)0x12345678;//修补地址
    *(DWORD*)&shellcode[20] = (DWORD)0x12345678;

    WriteProcessMemory(hProcess, pData, shellcode, sizeof(shellcode), NULL);
    //设置环境快
    SetThreadContext(hThread, &ctx);
    //回复线程
    ResumeThread(hThread);
}

int main() {
    thread_send(123,123);
    return 0;
}