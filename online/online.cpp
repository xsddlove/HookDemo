// online.cpp: 定义 DLL 的初始化例程。
//

#include "pch.h"
#include "framework.h"
#include "online.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable : 4996)
//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为以下项中的第一个语句:
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// ConlineApp

BEGIN_MESSAGE_MAP(ConlineApp, CWinApp)
END_MESSAGE_MAP()


// ConlineApp 构造

ConlineApp::ConlineApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 ConlineApp 对象

ConlineApp theApp;



#include"m_Dialog.h"
m_Dialog abc;
DWORD WINAPI 显示窗口(LPVOID arg) //25DC 线程 CALLBACK 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	abc.DoModal();//DoModal 以模态的方式显示窗口 //卡在这里
	//释放掉A031_MFC_DLL
	FreeLibraryAndExitThread(theApp.m_hInstance, 123);
	return 1;
}

// ConlineApp 初始化

BOOL ConlineApp::InitInstance()
{
	CWinApp::InitInstance();
	DWORD 线程ID = 0;
	::CreateThread(0, 0, 显示窗口, 0, 0, &线程ID);  //创建一个显示窗口的线程
	return TRUE;
}
