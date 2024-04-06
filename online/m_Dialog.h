#pragma once
#include "afxdialogex.h"


// m_Dialog 对话框

class m_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(m_Dialog)

public:
	m_Dialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~m_Dialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_list;
	afx_msg void OnBnClickedButton2();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	CString m_edit;
	afx_msg void OnBnClickedButton4();
};

extern m_Dialog abc;