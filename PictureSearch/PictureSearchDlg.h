
// PictureSearchDlg.h : ͷ�ļ�
//

#pragma once

#include "GraphCtrl.h"
#include "HSVImageManager.h"

// CPictureSearchDlg �Ի���
class CPictureSearchDlg : public CDialogEx
{
// ����
public:
	CPictureSearchDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PICTURESEARCH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
