#include "pch.h"
#include "CHook.h"
#include <Windows.h>
#include <iostream>
#include <thread>
#include <TlHelp32.h>
#include <string>
#pragma warning(disable : 4996)
using namespace std;
bool revce_page = TRUE;
extern DWORD g_ECX ;
HANDLE handle = NULL;
void PrintEnum() {
    cout << "[*] 清理屏幕" << "                        F1" << endl;
    cout << "[*] 收包检测" << "                        F2" << endl;
    cout << "[*] 手动发包" << "                        F3" << endl;
}
void* send_page();
void toggleFeatures() {
    while (true) {
        if (GetAsyncKeyState(VK_F1) & 1) {
            system("cls");
        }
        if (GetAsyncKeyState(VK_F2) & 1) {
            if (handle == NULL)
            {
                break;
            }
            RemoveVectoredExceptionHandler(handle);
        }
        if (GetAsyncKeyState(VK_F3) & 1) {

           // char* page = (char *)send_page();
           // char* memory = new char[0x100];
           // _asm {
           //     push 0;
           //     push 7
           //     push page;
           //     mov ecx, g_ECX;
           //     mov eax, 0x61EAD0;
           //     call eax
           //     retn
           // }

        }
        Sleep(100);
    }
}
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
    return 0;
}
void* send_page() {
    char send_page[100] = { 0 };
    scanf("%s", send_page);
    char temp = 0;
    char page[100] = { 0 };
    int t = 0;
    for (size_t i = 0; i < 100; i++)
    {
        if (send_page[i] == 0)
        {
            page[t] = temp;
            continue;
        }
        if (i % 2 == 0)//第一次进入
        {
            printf("1");
            if (send_page[i] <= 0x40 && send_page[i] >= 0x30)
            {
                temp = (send_page[i] - 0x30) * 0x10;
                printf("%x\n", send_page[i]);
            }

            if (send_page[i] <= 0x46 && send_page[i] >= 0x41)
            {
                temp = (send_page[i] - 0x37) * 0x10;
                printf("%x\n", send_page[i]);
            }

            if (send_page[i] <= 0x66 && send_page[i] >= 0x61)
            {
                temp = (send_page[i] - 0x57) * 0x10;
                printf("%x\n", send_page[i]);
            }
            printf("%x\n", temp);

        }
        if (i % 2 != 0)

        {
            printf("2");
            if (send_page[i] <= 0x40 && send_page[i] >= 0x30)
            {
                page[t] = temp + send_page[i] - 0x30;
            }

            if (send_page[i] <= 0x46 && send_page[i] >= 0x41)
            {
                page[t] = temp + send_page[i] - 0x37;
            }

            if (send_page[i] <= 0x66 && send_page[i] >= 0x61)
            {
                page[t] = temp + send_page[i] - 0x57;
            }
            temp = 0;
            t = t + 1;
        }


    }
    return page;
}

void thread_send(DWORD len, DWORD address) {
    //1,获取进程
    DWORD pid = getpid();

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
        0xB8,0xD0,0xEA,0x61,0x00,        //mov eax,0x61EAD0
        0xFF,0XD0,                       //call eax
        0x9D,                            //popfd
        0x61,                            //popad
        0xC3                             //ret
    };
    //申请空间shellcode
    PCHAR pData = (PCHAR)VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    //修补eip
    *(DWORD*)&shellcode[1] = (DWORD)ctx.Eip;
    *(DWORD*)&shellcode[10] = len;
    *(DWORD*)&shellcode[15] = (DWORD)address;//修补地址
    *(DWORD*)&shellcode[20] = (DWORD)0x9046058;
    
    WriteProcessMemory(hProcess, pData, shellcode, sizeof(shellcode), NULL);
    //ctx.Eip = (DWORD)pData;
    //设置环境快
    printf("pData %x\n",(DWORD)pData);
    
    SetThreadContext(hThread, &ctx);
    //回复线程
    ResumeThread(hThread);
}


DWORD WINAPI fun(LPVOID arg) //25DC 线程 CALLBACK 
{
    thread_send( 0,  0);
    return 1;
}


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //DWORD 线程ID = 0;
        //::CreateThread(0, 0, fun, 0, 0,0);  //创建一个显示窗口的线程
       AllocConsole();
       freopen("CONOUT$", "w", stdout);
       

       AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)ExceptionFilter); // 添加VEH异常处理
       SetSehHook();


    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}




//if (!threadCreated) {
//    threadCreated = true;
//    std::thread myThread(toggleFeatures);
//    myThread.detach();
//}