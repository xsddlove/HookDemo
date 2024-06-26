#include "pch.h"
#include "CHook.h"
#include <TlHelp32.h>
#include <stdio.h>
#include <limits.h>
#include <string>
using namespace std;

extern bool revce_page;
DWORD g_ECX = NULL;
DWORD g_HookAddr = 0x0061AED5;
DWORD RetkReciveMsgAddr = 0x0061AED5 + 5;
DWORD OverReciveMsgCallAddr = 0x61EAD0;

void SetSehHook()
{

	//遍历线程 通过openthread获取到线程环境后设置硬件断点
	HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hTool32 != INVALID_HANDLE_VALUE)
	{
		//线程环境结构体
		THREADENTRY32 thread_entry32;

		thread_entry32.dwSize = sizeof(THREADENTRY32);

		HANDLE hHookThread = NULL;

		//遍历线程
		if (Thread32First(hTool32, &thread_entry32))
		{
			do
			{
				//如果线程父进程ID为当前进程ID
				if (thread_entry32.th32OwnerProcessID == GetCurrentProcessId())
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
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
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