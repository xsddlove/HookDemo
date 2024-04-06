#include "pch.h"
#include "CHook.h"
#include <TlHelp32.h>
#include <stdio.h>
#include <limits.h>
#include <string>
#include <windows.h>
#include<iostream>
#include <process.h>
using namespace std;

//#define WxReciveMessage 0x40D3BA			//接收消息  1 
//#define WxReciveMessageCall 0x95320			//接收消息  1 
//聊天记录偏移
//#define MsgTypeOffset 0x38				//消息类型的偏移 1
//#define IsPhoneMsg 0x3C					//是否是手机发的消息 1
//#define MsgContentOffset 0x70			//消息内容的偏移 1
//#define MsgSourceOffset 0x1D8			//消息来源的偏移 1
//#define WxidOffset 0x48					//微信ID/群ID偏移 1 	
//#define GroupMsgSenderOffset 0x170		//群消息发送者偏移 1
extern bool revce_page;
DWORD g_ECX = NULL;
DWORD g_HookAddr = 0x0061AED5;

DWORD RetkReciveMsgAddr = 0x0061AED5 + 5;

DWORD OverReciveMsgCallAddr = 0x61EAD0;

void SetSehHook()
{

	//遍历线程 通过openthread获取到线程环境后设置硬件断点
	HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hTool32!= INVALID_HANDLE_VALUE)
	{
		//线程环境结构体
		THREADENTRY32 thread_entry32;

		thread_entry32.dwSize = sizeof(THREADENTRY32);

		HANDLE hHookThread = NULL;

		//遍历线程
		if (Thread32First(hTool32,&thread_entry32))
		{
			do 
			{
				//如果线程父进程ID为当前进程ID
				if (thread_entry32.th32OwnerProcessID==GetCurrentProcessId())
				{
					hHookThread = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, thread_entry32.th32ThreadID);
					

					SuspendThread(hHookThread);//暂停线程

					//设置硬件断点
					CONTEXT thread_context = { CONTEXT_DEBUG_REGISTERS };
					thread_context.Dr0 = g_HookAddr;
					thread_context.Dr7 = 0x405;

					//设置线程环境 这里抛异常了

					DWORD oldprotect;
					VirtualProtect((LPVOID)g_HookAddr, 5, PAGE_EXECUTE_READWRITE, &oldprotect);//修改PTE p=1 r/w1=0

					SetThreadContext(hHookThread, &thread_context);

					ResumeThread(hHookThread);//线程跑起来吧~~~


					CloseHandle(hHookThread);
				}

			} while (Thread32Next(hTool32, &thread_entry32));
			


		}
		CloseHandle(hTool32);

	}

}
void __declspec(naked) OriginalFunc(void)
{
	__asm
	{
		//调用被覆盖的call
		call OverReciveMsgCallAddr;
		//跳转到返回地址
		jmp RetkReciveMsgAddr;

	}
}
LONG NTAPI  ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{
	
	//printf("进入ExceptionFilter\n");
	//if (revce_page == false)
	//{
	//	printf("标志位未开\n");
	//	return EXCEPTION_CONTINUE_SEARCH;
	//}
	//判断当前异常码是否为硬件断点异常
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP )
	{

		//判断发生异常的地址是否和hook的地址一致
		if ((DWORD)ExceptionInfo->ExceptionRecord->ExceptionAddress == g_HookAddr)
		{

			//获取当前线程上下文
			PCONTEXT pcontext = ExceptionInfo->ContextRecord;
			DWORD page_len = *(DWORD*)((pcontext->Esp) + 4);
			printf("包长 %d \n", *(DWORD*)((pcontext->Esp) + 4));
			printf("ecx = %x \n", pcontext->Ecx);
			if (g_ECX == NULL)
			{
				g_ECX = pcontext->Ecx;
			}
			unsigned char* ptr;
			ptr = (unsigned char*)*(DWORD*)((pcontext->Esp) + 0);
			for (int i = 0; i < page_len; i++) {
				printf("%02X ", *(ptr + i));
			}
			printf("\n");
			//修复EIP 
			pcontext->Eip = (DWORD)&OriginalFunc;

			//异常处理完成 让程序继续执行
			return EXCEPTION_CONTINUE_EXECUTION;

		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
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
void thread_send(DWORD len, char* address) {
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
		0xB8,0x00,0x00,0x00,0x00,       //mov eax
		0x6A,0x00,                      //push 0
		0x6A,0x00,                       //push 参数二  
		0x50,                            //push eax    缓冲区地址
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
	*(DWORD*)&shellcode[8] = (DWORD)address;//修补地址
	*(char*)&shellcode[15] = len;
	*(DWORD*)&shellcode[18] = (DWORD)g_ECX; //ecx对象

	WriteProcessMemory(hProcess, pData, shellcode, sizeof(shellcode), NULL);
	//设置环境快
	ctx.Eip = (DWORD)pData;
	printf("pData %x\n", (DWORD)pData);
	//MessageBoxA(0, 0, 0, 0);
	SetThreadContext(hThread, &ctx);
	//回复线程
	ResumeThread(hThread);
	//释放内存
	VirtualFree(pData, 0x1000, MEM_RELEASE);
}






