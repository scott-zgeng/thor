
// PictureSearch.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPictureSearchApp:
// �йش����ʵ�֣������ PictureSearch.cpp
//

class CPictureSearchApp : public CWinApp
{
public:
	CPictureSearchApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPictureSearchApp theApp;

void WriteDebugLog(CString log);