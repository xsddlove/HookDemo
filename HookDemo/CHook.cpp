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

//#define WxReciveMessage 0x40D3BA			//������Ϣ  1 
//#define WxReciveMessageCall 0x95320			//������Ϣ  1 
//�����¼ƫ��
//#define MsgTypeOffset 0x38				//��Ϣ���͵�ƫ�� 1
//#define IsPhoneMsg 0x3C					//�Ƿ����ֻ�������Ϣ 1
//#define MsgContentOffset 0x70			//��Ϣ���ݵ�ƫ�� 1
//#define MsgSourceOffset 0x1D8			//��Ϣ��Դ��ƫ�� 1
//#define WxidOffset 0x48					//΢��ID/ȺIDƫ�� 1 	
//#define GroupMsgSenderOffset 0x170		//Ⱥ��Ϣ������ƫ�� 1
extern bool revce_page;
DWORD g_ECX = NULL;
DWORD g_HookAddr = 0x0061AED5;

DWORD RetkReciveMsgAddr = 0x0061AED5 + 5;

DWORD OverReciveMsgCallAddr = 0x61EAD0;

void SetSehHook()
{

	//�����߳� ͨ��openthread��ȡ���̻߳���������Ӳ���ϵ�
	HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hTool32!= INVALID_HANDLE_VALUE)
	{
		//�̻߳����ṹ��
		THREADENTRY32 thread_entry32;

		thread_entry32.dwSize = sizeof(THREADENTRY32);

		HANDLE hHookThread = NULL;

		//�����߳�
		if (Thread32First(hTool32,&thread_entry32))
		{
			do 
			{
				//����̸߳�����IDΪ��ǰ����ID
				if (thread_entry32.th32OwnerProcessID==GetCurrentProcessId())
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
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP )
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
	//1,��ȡ����
	DWORD pid = getpid();

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	//2,�ҵ����߳�
	DWORD Main_THREAD = EnumerateThreads(pid);
	//3,�����߳�
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, Main_THREAD);
	//4�����߳�
	DWORD suspendCount = SuspendThread(hThread);
	//5��ȡ�̻߳�����
 // ��ȡ�̻߳�����
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.ContextFlags = CONTEXT_ALL;
	if (GetThreadContext(hThread, &ctx) == 0) {
		printf("��ȡ�̻߳�����ʧ�ܣ������룺%d\n", GetLastError());
	}
	//printf("\t[+] ��ǰ�Ĵ���ֵ��\n\t\tEIP[0x%08x] ESP[0x%08x]\n", ctx.Eip, ctx.Esp);
	//����shellcode
	char shellcode[] = {
		0x68,0x00,0x00,0x00,0x00,       //push eip���ص�ַ
		0x60,                           //pushad
		0x9C,                           //pushfd
		0xB8,0x00,0x00,0x00,0x00,       //mov eax
		0x6A,0x00,                      //push 0
		0x6A,0x00,                       //push ������  
		0x50,                            //push eax    ��������ַ
		0xBE,0x00,0x00,0x00,0x00,        //mov esi,��־λ
		0x8B,0xCE,                       //mov ecx,esi
		0xB8,0xD0,0xEA,0x61,0x00,        //mov eax,0x61EAD0
		0xFF,0XD0,                       //call eax
		0x9D,                            //popfd
		0x61,                            //popad
		0xC3                             //ret
	};
	//����ռ�shellcode
	PCHAR pData = (PCHAR)VirtualAllocEx(hProcess, NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//�޲�eip
	*(DWORD*)&shellcode[1] = (DWORD)ctx.Eip;
	*(DWORD*)&shellcode[8] = (DWORD)address;//�޲���ַ
	*(char*)&shellcode[15] = len;
	*(DWORD*)&shellcode[18] = (DWORD)g_ECX; //ecx����

	WriteProcessMemory(hProcess, pData, shellcode, sizeof(shellcode), NULL);
	//���û�����
	ctx.Eip = (DWORD)pData;
	printf("pData %x\n", (DWORD)pData);
	//MessageBoxA(0, 0, 0, 0);
	SetThreadContext(hThread, &ctx);
	//�ظ��߳�
	ResumeThread(hThread);
	//�ͷ��ڴ�
	VirtualFree(pData, 0x1000, MEM_RELEASE);
}






