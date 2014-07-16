
// PictureSearchDlg.h : 头文件
//

#pragma once

#include "GraphCtrl.h"
#include "HSVImageManager.h"

// CPictureSearchDlg 对话框
class CPictureSearchDlg : public CDialogEx
{
// 构造
public:
	CPictureSearchDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PICTURESEARCH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpenDir();
	int m_chartIndex;
	CStatic m_picOrg;
	CStatic m_picDst1;
	CStatic m_picDst2;
	CListBox m_log;
	CListBox m_listImage;
	CString m_path;
	int m_centerValue;
	int m_ccvValue;
	int m_hsvValue;

	HSVImageManager m_manager;
	afx_msg void OnDblclkListImage();
//	CStatic m_chartSRC;
	CGraphCtrl m_chartSrc;
	CGraphCtrl m_chartDst1;
	CGraphCtrl m_chartDst2;
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
//	CString m_reference;
	CComboBox m_reference;
	afx_msg void OnSelchangeComboRef();
	afx_msg void OnIdok();
	afx_msg void OnIdno();
//	CComboBox m_diffType;
	CComboBox m_diffType;
	afx_msg void OnSelchangeComboDiffType();

	int m_diffTypeIdx;
};
