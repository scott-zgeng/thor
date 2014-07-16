// GraphCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "GraphCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGraphCtrl

CGraphCtrl::CGraphCtrl()
{
	NumOfBars = 27;
	offset = 10;
	unit = UCM;
	bar = new int[NumOfBars];
	BC = new COLORREF[NumOfBars];
	for(UINT i=0;i<NumOfBars;i++)
	{
		bar[i] = 0; //i*UCM;
		BC[i] = RGB(180,180,180);
	}
}

CGraphCtrl::~CGraphCtrl()
{
}


BEGIN_MESSAGE_MAP(CGraphCtrl, CStatic)
	//{{AFX_MSG_MAP(CGraphCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphCtrl message handlers

BOOL CGraphCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CStatic::OnNotify(wParam, lParam, pResult);
}

BOOL CGraphCtrl::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

LRESULT CGraphCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CStatic::WindowProc(message, wParam, lParam);
}

BOOL CGraphCtrl::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rc;
	GetClientRect (rc);
	BGC = RGB(224,231,240);
	int Width = rc.Width();
	int Height= rc.Height();

	int bx=0, by=0;

	UINT i;
	for(i=0;i<NumOfBars;i++)
		if(bar[i]>Height-offset) bar[i] = bar[i] % (Height-offset);

//	KillTimer(1);
	pDC->FillSolidRect(0,0,Width,Height,BGC);

	pDC->FillSolidRect(offset-2,0,2,Height-(offset-2),RGB(0,0,0));
	pDC->FillSolidRect(offset,Height-offset,Width-offset,2,RGB(0,0,0));

	for(i=1;i<=(Height-offset)/(unit/2);i++)
	{
		if(i%2 != 0)
		{
			pDC->MoveTo(offset-2,(Height-offset) - i*(unit/2));
			pDC->LineTo(offset-6,(Height-offset) - i*(unit/2));
		}
		else
		{
			pDC->MoveTo(offset-2,(Height-offset) - i*(unit/2));
			pDC->LineTo(offset-10,(Height-offset) - i*(unit/2));
		}
	}

	for(i=0;i<NumOfBars;i++)
	{
		bx = ((Width-offset)/NumOfBars)*i+offset;
		pDC->FillSolidRect(bx, Height-bar[i]-offset,(Width-offset)/NumOfBars,bar[i],BC[i]);
		pDC->Draw3dRect(bx, Height-bar[i]-offset,(Width-offset)/NumOfBars,bar[i],RGB(255,255,255),BC[i]);
	}

	return CStatic::OnEraseBkgnd(pDC);
}

void CGraphCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	
	CStatic::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CGraphCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CStatic::OnPaint() for painting messages
}

void CGraphCtrl::OnTimer(UINT nIDEvent) 
{
	KillTimer(1);
	for(UINT i=0;i<NumOfBars;i++)
	{
		bar[i] += rand()/3;
	}
	Invalidate(1);
	
	CStatic::OnTimer(nIDEvent);
}

int CGraphCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	return 0;
}

void CGraphCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;
	CDC *pDC = CDC::FromHandle(lpcd->hdc);
	switch(lpcd->dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
			//return;
		case CDDS_ITEMPREPAINT:
			if (lpcd->dwItemSpec == TBCD_THUMB)
			{
				*pResult = CDRF_DODEFAULT;
				break;
			}
			break;
	}
}

int CGraphCtrl::SetNumberOfBars(int num)
{
	CRect rc;
	GetClientRect (rc);

	if(num>rc.Width())
		return -1;

	NumOfBars = num;
	Invalidate();
	return NumOfBars;
}

void CGraphCtrl::SetBarValue(int index, int val)
{
	if(index<0 || index>=NumOfBars || val<0)
		return;

	bar[index] = val*unit;

	Invalidate();
}


void CGraphCtrl::SetUnit(int unt)
{
	if(unit != unt)
		for(UINT i=0;i<NumOfBars;i++)
		{
			bar[i] = (bar[i]*unt)/unit;
		}

	unit = unt;

	Invalidate();
}

BOOL CGraphCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CStatic::PreCreateWindow(cs);
}
