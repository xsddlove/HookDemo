#pragma once
#include "afxdialogex.h"


// m_list 对话框

class m_list : public CDialogEx
{
	DECLARE_DYNAMIC(m_list)

public:
	m_list(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~m_list();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
