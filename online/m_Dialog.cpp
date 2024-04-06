// m_Dialog.cpp: 实现文件
//

#include "pch.h"
#include "online.h"
#include "afxdialogex.h"
#include "m_Dialog.h"
#include <Windows.h>
#include "Process.h"

#pragma warning(disable : 4996)
// m_Dialog 对话框

IMPLEMENT_DYNAMIC(m_Dialog, CDialogEx)

m_Dialog::m_Dialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_edit(_T(""))
{

}

m_Dialog::~m_Dialog()
{
}

void m_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_list, m_list);
	DDX_Control(pDX, IDC_list, m_list);
	DDX_Text(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(m_Dialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON2, &m_Dialog::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &m_Dialog::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON1, &m_Dialog::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &m_Dialog::OnEnChangeEdit1)
    ON_BN_CLICKED(IDC_BUTTON4, &m_Dialog::OnBnClickedButton4)
END_MESSAGE_MAP()


// m_Dialog 消息处理程序

#include"Chook.h"
//HANDLE m_handle = NULL;
void m_Dialog::OnBnClickedButton2()
{
    //AllocConsole();
    //freopen("CONOUT$", "w", stdout);
    //::CreateThread(0, 0, fun, 0, 0,0);  //创建一个显示窗口的线程
    
    //AddVectoredExceptionHandler(1, (PVECTORED_EXCEPTION_HANDLER)ExceptionFilter); // 添加VEH异常处理
    SetSehHook();
    //printf("dll注入成功\n");
    
}

void m_Dialog::OnBnClickedButton3()
{
	//// TODO: 在此添加控件通知处理程序代码
	//if (m_handle != NULL)
	//{
	//	RemoveVectoredExceptionHandler(m_handle);
	//}
	
}


BOOL m_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_list.ModifyStyle(0, LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_ALIGNLEFT | WS_VSCROLL | WS_BORDER);

	// 添加列头
	m_list.InsertColumn(0, _T("行号"), LVCFMT_LEFT, 50);
	//m_list.InsertColumn(1, _T("包长"), LVCFMT_LEFT, 50);
	m_list.InsertColumn(1, _T("包内容"), LVCFMT_LEFT, 300);

	// 插入数据
	//m_list.InsertItem(0, _T("行1"));
	//m_list.SetItemText(0, 1, _T("数据1"));

	//m_list.InsertItem(1, _T("行2"));
	//m_list.SetItemText(1, 1, _T("数据3"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

extern DWORD g_ECX;


__declspec(naked) void YourFunctionName(char* buffer)
{
    __asm {
        pushad
        pushfd
        push 0
        push 7
        push buffer
        mov esi,0x9386860   // 9386860
        mov ecx, esi
        mov eax, 0x61EAD0
        call eax
        popfd
        popad
        retn
    }
}

void thread_send(DWORD len, char* address);
void* send_page(char* charPtr, DWORD length, char* page);
void m_Dialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CString STR =  m_edit;
    CStringA strA(STR);
    char* charPtr = (char *)strA.GetString();
    DWORD length = strlen(charPtr);
    //缓冲区
    char* buffer = (char*)malloc(100);
    memset(buffer, 0, 100);
    char* page_address = (char*)send_page(charPtr, length, buffer);
    //printf("page_address %x",(DWORD)page_address);
    // 拷贝
    //memmove(buffer, page_address, length);
    thread_send(length, page_address);
    Sleep(500);
    free(buffer);

    //char* buffer = (char*)malloc(length+1);
    //// 拷贝数据
    //memcpy(buffer, page_address, length+1);
    ////发包
    //YourFunctionName(buffer);
    //free(buffer);
}


void m_Dialog::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void* send_page(char* charPtr,DWORD length,char* page) {
    char send_page[100] = { 0 };
    
    memmove(send_page, charPtr, length);
    //scanf("%s", send_page);
    char temp = 0;
    //char page[100] = { 0 };
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

void thread_send(DWORD len,char* address) {
    //1,获取进程
    DWORD pid = getpid();

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);
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
    PCHAR pData = (PCHAR)VirtualAllocEx(hProcess,NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    //修补eip
    *(DWORD*)&shellcode[1] = (DWORD)ctx.Eip;
    *(DWORD*)&shellcode[8] = (DWORD)address;//修补地址
    *(char*)&shellcode[15] = len;
    *(DWORD*)&shellcode[18] = (DWORD)g_ECX; //ecx对象

    WriteProcessMemory(hProcess, pData,shellcode,sizeof(shellcode),NULL);
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



void m_Dialog::OnBnClickedButton4()
{
    // TODO: 在此添加控件通知处理程序代码

}
