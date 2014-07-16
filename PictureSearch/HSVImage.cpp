#include "stdafx.h"
#include <math.h>
#include "HSVImage.h"
#include "PictureSearch.h"



double MAX3(double a, double b, double c) {
	return max(max(a, b), c);
}

double MIN3(double a, double b, double c) {
	return min(min(a, b), c);
}


double DiffValue1(int* src, int* dst, int length) 
{
	double diff = 0;
	double sum1 = 0.0;
	double sum2 = 0.0;

	for (int i = 0; i < length; i++) {
		sum1 += src[i];
		sum2 += dst[i];
		diff += abs(sum1 - sum2);
	}   

	return diff;
}

double DiffValue2(int* src, int* dst, int length) 
{
	double diff = 0;
	double sum1 = 0.0;
	double sum2 = 0.0;

	for (int i = 0; i < length; i++) {
		sum1 += src[i];
		sum2 += dst[i];
		diff += (sum1 - sum2) * (sum1 - sum2);
	}   

	diff = sqrt(diff);
	return diff;
}


double DiffValueMax(int* src, int* dst, int length) 
{
	double diff = 0;
	double sum1 = 0.0;
	double sum2 = 0.0;

	for (int i = 0; i < length; i++) {
		sum1 += src[i];
		sum2 += dst[i];		
		diff = max(abs(sum1 - sum2), diff);
	}   

	return diff;
}

typedef double (*DIFF_FUNC)(int* src, int* dst, int length);

static DIFF_FUNC DiffValues[] = { DiffValue1, DiffValue2, DiffValueMax};



HSVImage::HSVImage(const CString& name, CBitmap* bmp, int adjustWidth, int adjustHeight) 
{
	BITMAP bt;
	bmp->GetBitmap(&bt);

	this->name = name;
	this->bmp = bmp;

	this->width = adjustWidth;
	this->height = adjustHeight;
	
	memset(hsv, 0, sizeof(hsv));
	memset(center, 0, sizeof(center));
	memset(continuous, 0, sizeof(continuous));
	memset(uncontinuous, 0, sizeof(uncontinuous));

	items = new ImageItem* [height];
	for (int h = 0; h < height; h++) {
		items[h] = new ImageItem[width];
		for (int w = 0; w < width; w++) {
			items[h][w].index = 0;
			items[h][w].vector = 0;
		}
	}

	int org_width = bt.bmWidth;
	int org_height = bt.bmHeight;

	int x = (org_width - width) / 2;
	int y = (org_height - height) / 2; 

	unsigned char* px = new unsigned char [bt.bmHeight * bt.bmWidthBytes];
	bmp->GetBitmapBits(bt.bmHeight * bt.bmWidthBytes, px);//读取位图数据
	int PixelBytes = bt.bmBitsPixel / 8; 

	int pos;
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			pos = (h + y)* bt.bmWidthBytes + (w + x) * PixelBytes;
			int idx = quantization(px + pos);            
			items[h][w].index = idx;			
		}            
	}   

	delete[] px;
}


HSVImage::~HSVImage()
{
	for (int h = 0; h < height; h++) {
		if (items[h] != NULL) {
			delete[] items[h];
		}
	}

	if (items != NULL) {
		delete[] items;
	}
}


void HSVImage::buildCCV()
{
	int vectorCount = 0;

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			ImageItem* item = &items[h][w];
			if (item->vector != 0)                
				continue;

			vectorCount++;
			item->vector = vectorCount;
			stepCCV(w, h, 0);  
		}
	}
	
	int limen = height * width / CCV_VALUE;
	
	for (int v = 1; v <= vectorCount; v++) {
		int count = 0;
		int idx = 0;

		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				ImageItem* item = &items[h][w];
				if (item->vector == v) {
					idx = item->index;
					count++;
				}
			}
		}            

		if (count >= limen)
			continuous[idx] += count;
		else
			uncontinuous[idx] += count;		
	}        

}


void HSVImage::stepCCV(int w, int h, int level) 
{
	if (testCCV(w, h, w-1, h-1)) { stepCCV(w-1, h-1, level + 1);}        
	if (testCCV(w, h, w , h-1))  { stepCCV(w,   h-1, level + 1);}            
	if (testCCV(w, h, w+1, h-1)) { stepCCV(w+1, h-1, level + 1);}

	if (testCCV(w, h, w-1, h))   { stepCCV(w-1, h, level + 1);}        
	if (testCCV(w, h, w+1, h))   { stepCCV(w+1, h, level + 1);}

	if (testCCV(w, h, w-1, h+1)) { stepCCV(w-1, h+1, level + 1);}        
	if (testCCV(w, h, w  , h+1)) { stepCCV(w,   h+1, level + 1);}        
	if (testCCV(w, h, w+1, h+1)) { stepCCV(w+1, h+1, level + 1);}        
}


bool HSVImage::testCCV(int w1, int h1, int w2, int h2) 
{
	if (w2 < 0 || w2 >= width || h2 < 0 || h2 >= height) 
		return false;        

	ImageItem* item1 = &items[h1][w1]; 
	ImageItem* item2 = &items[h2][w2]; 
	if (item1->index != item2->index)
		return false;

	if (item2->vector != 0)
		return false;        

	item2->vector = item1->vector;        
	return true;
}

void HSVImage::buildHSV()
{
	int idx = 0;
	for (int h = 0; h < height  ; h++) {
		for (int w = 0; w < width ; w++) {
			idx = items[h][w].index;
			hsv[idx]++;			
		}            
	}  
}

void HSVImage::buildCenter() 
{	
	int c_width = width / 3;
	int c_height = height / 3;
	int c_x = (width - c_width) / 2;
	int c_y = (height - c_height) / 2;

	int idx = 0;
	for (int h = 0; h < c_height  ; h++) {
		for (int w = 0; w < c_width; w++) {
			idx = items[h + c_y][w + c_x].index;
			center[idx]++;
		}            
	}    
}


void HSVImage::build()
{ 
	buildHSV();
    buildCCV();
	buildCenter();	
}
  
    
double HSVImage::getV(unsigned char* pixel) 
{
	double r = pixel[0];
	double g = pixel[1];
	double b = pixel[2];
                
    return MAX3(r, g, b) / 255.0;                               
}
    
double HSVImage::getS(unsigned char* pixel) 
{
	double r = pixel[0];
	double g = pixel[1];
	double b = pixel[2];
        
    double v_max = MAX3(r, g, b);
    if (v_max == 0)
        return 0;
        
    double v_min = MIN3(r, g, b);

    return (v_max - v_min) / v_max;
}
        
double HSVImage::getH(unsigned char* pixel) {
    double r = pixel[0];
    double g = pixel[1];
    double b = pixel[2];
        
    double v_max = MAX3(r, g, b);
    double v_min = MIN3(r, g, b);  
    double v_diff = v_max - v_min;
        
    if (v_diff <= 0.000001)
        return 0.0;
        
    double h;
    if (r == v_max) {
        if (g == v_min)        
            h = 300.0 + 60.0 * (v_max - b) / v_diff;
        else
            h = 60.0 - 60.0 * (v_max - g) / v_diff;
    } else if (g == v_max){
        if (b == v_min)
            h = 60.0 + 60.0 * (v_max - r) / v_diff;
        else 
            h = 180.0 - 60.0 * (v_max - b) / v_diff;
    } else {  // b == v_max
        if (r == v_min)
            h = 180.0 + 60.0 * (v_max - g) / v_diff;
        else
            h = 300.0 - 60.0 * (v_max - r) / v_diff;                            
    }
    return h;
}
    
int HSVImage::quantization(unsigned char* pixel) {
    double v = getV(pixel);
    double s = getS(pixel);
    double h = getH(pixel); 

    if (v < 0.2) 
        return 0;

    if (s < 0.2) 
        return (int)(floor(10 * v - 2) + 1);
        
    int H;
        
    if (h > 327 || h <= 27)
        H = 0;
    else if (h > 27 && h <= 56)
        H = 1;
    else if (h > 56 && h <= 159)
        H = 2;
    else if (h > 159 && h <= 202)
        H = 3;
    else if (h > 202 && h <= 280)
        H = 4;
    else 
        H = 5;

    if (v <= 0.4)
        return H + 9;            
                    
    if (s < 0.5)
        return 2 * H + 15;
        
    return 2 * H + 16;
}
    

int HSVImage::sum() {
    int sum = 0;
        
    for (int i =0 ; i < BASELINE_SIZE; i++) {
        sum += hsv[i];
    }
    return sum;        
}
    
int* HSVImage::getData(int index)
{
	switch (index){
	case 0: return hsv;
	case 1: return continuous;
	case 2: return uncontinuous;
	case 3: return center;
	default: return hsv;
	}
}

double HSVImage::diff(HSVImage* other, int type, int hsv, int ccv, int center)
{
	return hsv* diffHSV(other, type) +  ccv * diffCCV(other, type) +  center * diffCenter(other, type);
}

    
double HSVImage::diffHSV(HSVImage* other, int type) {
	return DiffValues[type](this->hsv, other->hsv, BASELINE_SIZE);
}    

double HSVImage::diffCCV(HSVImage* other, int type) 
{
	double d1 = DiffValues[type](this->continuous, other->continuous, BASELINE_SIZE);
	double d2 = DiffValues[type](this->uncontinuous, other->uncontinuous, BASELINE_SIZE);
	return d1 + d2;
}    


double HSVImage::diffCenter(HSVImage* other, int type) 
{
	return DiffValues[type](this->center, other->center, BASELINE_SIZE);	
}    

