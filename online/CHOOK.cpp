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

	//�����߳� ͨ��openthread��ȡ���̻߳���������Ӳ���ϵ�
	HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hTool32 != INVALID_HANDLE_VALUE)
	{
		//�̻߳����ṹ��
		THREADENTRY32 thread_entry32;

		thread_entry32.dwSize = sizeof(THREADENTRY32);

		HANDLE hHookThread = NULL;

		//�����߳�
		if (Thread32First(hTool32, &thread_entry32))
		{
			do
			{
				//����̸߳�����IDΪ��ǰ����ID
				if (thread_entry32.th32OwnerProcessID == GetCurrentProcessId())
				{
					hHookThread = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, thread_entry32.th32ThreadID);


					SuspendThread(hHookThread);//��ͣ�߳�

					//����Ӳ���ϵ�
					CONTEXT thread_context = { CONTEXT_DEBUG_REGISTERS };
					thread_context.Dr0 = g_HookAddr;
					thread_context.Dr7 = 0x405;

					//�����̻߳��� �������쳣��

					DWORD oldprotect;
					VirtualProtect((LPVOID)g_HookAddr, 5, PAGE_EXECUTE_READWRITE, &oldprotect);//�޸�PTE p=1 r/w1=0

					SetThreadContext(hHookThread, &thread_context);

					ResumeThread(hHookThread);//�߳���������~~~


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
		//���ñ����ǵ�call
		call OverReciveMsgCallAddr;
		//��ת�����ص�ַ
		jmp RetkReciveMsgAddr;

	}
}


LONG NTAPI  ExceptionFilter(PEXCEPTION_POINTERS ExceptionInfo)
{

	//printf("����ExceptionFilter\n");
	//if (revce_page == false)
	//{
	//	printf("��־λδ��\n");
	//	return EXCEPTION_CONTINUE_SEARCH;
	//}
	//�жϵ�ǰ�쳣���Ƿ�ΪӲ���ϵ��쳣
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
	{

		//�жϷ����쳣�ĵ�ַ�Ƿ��hook�ĵ�ַһ��
		if ((DWORD)ExceptionInfo->ExceptionRecord->ExceptionAddress == g_HookAddr)
		{

			//��ȡ��ǰ�߳�������
			PCONTEXT pcontext = ExceptionInfo->ContextRecord;
			DWORD page_len = *(DWORD*)((pcontext->Esp) + 4);
			printf("���� %d \n", *(DWORD*)((pcontext->Esp) + 4));
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
			//�޸�EIP 
			pcontext->Eip = (DWORD)&OriginalFunc;

			//�쳣������� �ó������ִ��
			return EXCEPTION_CONTINUE_EXECUTION;

		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}