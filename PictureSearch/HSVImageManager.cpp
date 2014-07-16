#include "stdafx.h"
#include "afxcoll.h"
#include <math.h>
#include <cfloat>

#include "HSVImage.h"
#include "HSVImageManager.h"
#include "PictureSearch.h"


HSVImageManager::HSVImageManager() 
{
	hsvList = new CMapStringToPtr();
	bmpList = new CMapStringToPtr();
}

HSVImageManager::~HSVImageManager() 
{
	delete hsvList;
	delete bmpList;
}

void HSVImageManager::clean()
{	
	// TODO 实现清理操作
}

HSVImage* HSVImageManager::getHSV(const CString& name)
{
	void* ptr;
	if (!hsvList->Lookup(name, ptr))
		return NULL;

	return (HSVImage*)ptr;
}


BITMAP* HSVImageManager::getBMP(const CString& name)
{
	void* ptr;
	if (!bmpList->Lookup(name, ptr))
		return NULL;

	return (BITMAP*)ptr;
}


void HSVImageManager::add(const CString& name, CBitmap* bmp)
{
	bmpList->SetAt(name, bmp);
}

void HSVImageManager::build() 
{
    int minWidth = INT_MAX;
    int minHeight = INT_MAX;
    

	CString key;
	VOID* ptr;

	POSITION pos = bmpList->GetStartPosition();
	while (pos != NULL) {
		bmpList->GetNextAssoc(pos, key, ptr);
		
		CBitmap* bmp = (CBitmap*)ptr;
		BITMAP bm;
		bmp->GetBitmap(&bm);
		if (minWidth > bm.bmWidth)
			minWidth = bm.bmWidth;

		if (minHeight > bm.bmHeight)
			minHeight = bm.bmHeight;		
	}

	CString log;
	log.Format(_T("minWidth = %d, minHeight = %d"), minWidth, minHeight);
	WriteDebugLog(log);	

	pos = bmpList->GetStartPosition();
	while (pos != NULL) {
		bmpList->GetNextAssoc(pos, key, ptr);

		CBitmap* bmp = (CBitmap*)ptr;
		HSVImage* hsv = new HSVImage(key, bmp, minWidth, minHeight);
		hsv->build();
		hsvList->SetAt(key, hsv);

	}

	log.Format(_T("hsvList GetCount %d"), hsvList->GetCount());
	WriteDebugLog(log);	
}


HSVImage* HSVImageManager::find(HSVImage* src, int type, int hsv, int ccv, int center)
{	
	if (type >= 0 && type < 3)
		return findInner(src, type, hsv, ccv, center);
	
	/*
	HSVImage* dst;
	double d;
	
	dst = findInner(src, 1, 1, 0, 0);
	d = src->diff(dst, 1, 1, 0, 0);
	if (d * 5.0 < src->sum()) {
		WriteDebugLog(_T("1, 1, 0, 0"));
		return dst;
	}*/

	HSVImage* dst1 = findInner(src, 1, 1, 4, 50);
	HSVImage* dst2 = findInner(src, 2, 1, 4, 50);

	double d1 = src->diff(dst1, 1, 1, 4, 50);
	double d2 = src->diff(dst2, 2, 1, 4, 50);
	
	if (2.3 * d2 < d1)
		return dst2;

	return dst1;
}

HSVImage* HSVImageManager::findInner(HSVImage* src, int type, int hsv, int ccv, int center)
{        	
    // calculate the diff
    double diff = DBL_MAX;
        

    
	CString key;
	VOID* ptr;
	HSVImage* dst = NULL; 

	POSITION pos = hsvList->GetStartPosition();
	while (pos != NULL) {
		hsvList->GetNextAssoc(pos, key, ptr);

		if (key == src->getName())
			continue;
		
		double d = src->diff((HSVImage*)ptr, type, hsv, ccv, center);

		if (diff > d) {
			diff = d;			
			dst = (HSVImage*)ptr;                
		}
	}

	CString log;
	log.Format(_T("src %s, dst %s, diff %lf"), src->getName(), dst->getName(), diff);    
	WriteDebugLog(log);
	return dst;
}
    

void HSVImageManager::test() {

    


}


