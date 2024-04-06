// m_list.cpp: 实现文件
//

#include "pch.h"
#include "online.h"
#include "afxdialogex.h"
#include "m_list.h"


// m_list 对话框

IMPLEMENT_DYNAMIC(m_list, CDialogEx)

m_list::m_list(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

m_list::~m_list()
{
}

void m_list::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(m_list, CDialogEx)
END_MESSAGE_MAP()


// m_list 消息处理程序
