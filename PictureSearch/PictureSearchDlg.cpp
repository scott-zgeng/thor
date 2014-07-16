
// PictureSearchDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PictureSearch.h"
#include "PictureSearchDlg.h"
#include "afxdialogex.h"

#include "HSVImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPictureSearchDlg �Ի���




CPictureSearchDlg::CPictureSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPictureSearchDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_chartIndex = 0;
	m_path = _T("");
	m_centerValue = 50;
	m_ccvValue = 4;
	m_hsvValue = 1;
	//  m_reference = _T("");
	m_diffTypeIdx = 3;
}

void CPictureSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_chartIndex);
	DDV_MinMaxInt(pDX, m_chartIndex, 0, 3);
	DDX_Control(pDX, IDC_PIC_SRC, m_picOrg);
	DDX_Control(pDX, IDC_PIC_DST_1, m_picDst1);
	DDX_Control(pDX, IDC_PIC_DST_2, m_picDst2);
	DDX_Control(pDX, IDC_LIST_LOG, m_log);
	DDX_Control(pDX, IDC_LIST_IMAGE, m_listImage);
	DDX_Text(pDX, IDC_EDIT_PATH, m_path);
	DDX_Text(pDX, IDC_CENTER_VALUE, m_centerValue);
	DDV_MinMaxInt(pDX, m_centerValue, 0, 100);	
	DDX_Text(pDX, IDC_CCV_VALUE, m_ccvValue);
	DDV_MinMaxInt(pDX, m_ccvValue, 0, 100);
	DDX_Text(pDX, IDC_HSV_VALUE, m_hsvValue);
	DDV_MinMaxInt(pDX, m_hsvValue, 0, 100);	
	DDX_Control(pDX, IDC_CHART_SRC, m_chartSrc);
	DDX_Control(pDX, IDC_CHART_DST_1, m_chartDst1);
	DDX_Control(pDX, IDC_CHART_DST_2, m_chartDst2);
	//  DDX_CBString(pDX, IDC_COMBO_REF, m_reference);
	DDX_Control(pDX, IDC_COMBO_REF, m_reference);
	//  DDX_Control(pDX, IDC_COMBO_DIFF_TYPE, m_diffType);
	DDX_Control(pDX, IDC_COMBO_DIFF_TYPE, m_diffType);
}

BEGIN_MESSAGE_MAP(CPictureSearchDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_DIR, &CPictureSearchDlg::OnBnClickedOpenDir)
	ON_LBN_DBLCLK(IDC_LIST_IMAGE, &CPictureSearchDlg::OnDblclkListImage)
	ON_BN_CLICKED(IDC_RADIO1, &CPictureSearchDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPictureSearchDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CPictureSearchDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CPictureSearchDlg::OnBnClickedRadio4)
	ON_CBN_SELCHANGE(IDC_COMBO_REF, &CPictureSearchDlg::OnSelchangeComboRef)
	ON_COMMAND(IDOK, &CPictureSearchDlg::OnIdok)
	ON_COMMAND(IDNO, &CPictureSearchDlg::OnIdno)
	ON_CBN_SELCHANGE(IDC_COMBO_DIFF_TYPE, &CPictureSearchDlg::OnSelchangeComboDiffType)
END_MESSAGE_MAP()


// CPictureSearchDlg ��Ϣ�������

BOOL CPictureSearchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	
	m_diffType.AddString(_T("��������"));
	m_diffType.AddString(_T("ŷʽ����"));
	m_diffType.AddString(_T("Chebychv����"));
	m_diffType.AddString(_T("�Զ��㷨(Ȩ����Ч)"));
	m_diffType.SetCurSel(m_diffTypeIdx);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CPictureSearchDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPictureSearchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CPictureSearchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPictureSearchDlg::OnBnClickedOpenDir()
{
	CFolderPickerDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;

	m_path = dlg.GetFolderPath();	
	UpdateData(FALSE);
	CFileFind finder;

	CString szFileToFind; 
	szFileToFind.Format(_T("%s\\*.BMP"), m_path);	

	BOOL bWorking = finder.FindFile(szFileToFind);

	for (int i = 0; i < m_listImage.GetCount(); i++)
		m_listImage.DeleteString(i);
	
	m_manager.clean();

	for (int i = 0; i < m_reference.GetCount(); i++)
		m_reference.DeleteString(i);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();		

		CString file = finder.GetFilePath();
		CString name = finder.GetFileName();
		m_listImage.AddString(name);
		m_reference.AddString(name);
		
		CBitmap* bmp = new CBitmap;				
		bmp->m_hObject = (HBITMAP)LoadImage(NULL, file, IMAGE_BITMAP, 0, 0,
			LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE); 

		m_manager.add(name, bmp);
	} 

	m_manager.build();
}


void CPictureSearchDlg::OnDblclkListImage()
{	
	UpdateData(TRUE);

	int idx = m_listImage.GetCurSel();
	if (idx == -1) return;

	CString name;
	m_listImage.GetText(idx, name);
	

	HSVImage* src = m_manager.getHSV(name);
	if (src == NULL) {
		WriteDebugLog(_T("get find org pic"));
		return;
	}


	HSVImage* dst1 = NULL;
	
	dst1 = m_manager.find(src, m_diffTypeIdx, m_hsvValue, m_ccvValue, m_centerValue);
	if (dst1 == NULL) {
		WriteDebugLog(_T("get find dst pic"));
		return;
	}
	
	CBitmap* bmp1 = src->getBitmap();
	CBitmap* bmp2 = dst1->getBitmap();
	

	m_picOrg.SetBitmap((*bmp1));
	m_picDst1.SetBitmap((*bmp2));
	

	m_chartSrc.SetNumberOfBars(BASELINE_SIZE);
	m_chartDst1.SetNumberOfBars(BASELINE_SIZE);
	


	for (int i = 0 ; i < BASELINE_SIZE; i++) {
		m_chartSrc.SetBarValue(i, src->getData(m_chartIndex)[i]);
		m_chartDst1.SetBarValue(i, dst1->getData(m_chartIndex)[i]);
	
	}

}


void CPictureSearchDlg::OnBnClickedRadio1()
{
	m_chartIndex = 0;
	OnDblclkListImage();
	OnSelchangeComboRef();
}


void CPictureSearchDlg::OnBnClickedRadio2()
{
	m_chartIndex = 1;
	OnDblclkListImage();
	OnSelchangeComboRef();
}


void CPictureSearchDlg::OnBnClickedRadio3()
{
	m_chartIndex = 2;
	OnDblclkListImage();
	OnSelchangeComboRef();
}


void CPictureSearchDlg::OnBnClickedRadio4()
{
	m_chartIndex = 3;
	OnDblclkListImage();
	OnSelchangeComboRef();
}


void CPictureSearchDlg::OnSelchangeComboRef()
{
	CString name;
	int idx = m_reference.GetCurSel();
	if (idx == -1) return;

	m_reference.GetLBText(idx, name);
	HSVImage* dst = m_manager.getHSV(name);
	if (dst == NULL) {
		WriteDebugLog(_T("get find dst pic"));
		return;
	}
	
	CBitmap* bmp = dst->getBitmap();

	m_picDst2.SetBitmap((*bmp));	
	m_chartDst2.SetNumberOfBars(BASELINE_SIZE);


	for (int i = 0 ; i < BASELINE_SIZE; i++) {
		m_chartDst2.SetBarValue(i, dst->getData(m_chartIndex)[i]);
	}
}


void CPictureSearchDlg::OnIdok()
{
	// TODO: �ڴ���������������
}


void CPictureSearchDlg::OnIdno()
{
	// TODO: �ڴ���������������
}


void CPictureSearchDlg::OnSelchangeComboDiffType()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_diffTypeIdx = m_diffType.GetCurSel();
}
