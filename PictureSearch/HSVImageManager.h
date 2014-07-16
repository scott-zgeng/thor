#pragma once


class HSVImage;
class CMapStringToPtr;

/**
 * image mamanger 
 */
class HSVImageManager 
{	
public:

    CMapStringToPtr* hsvList;
    CMapStringToPtr* bmpList;

	HSVImageManager();
	~HSVImageManager();  

	HSVImage* getHSV(const CString& name);
	BITMAP* getBMP(const CString& name);
    
	void clean();
    void add(const CString& name, CBitmap* bmp); 
        
    void build();    
    HSVImage* findInner(HSVImage* src, int type, int hsv, int ccv, int center);
	HSVImage* find(HSVImage* src, int type, int hsv, int ccv, int center);
    
	void test();
    
};


