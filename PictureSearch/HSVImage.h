#pragma once



#define BASELINE_SIZE  27
#define CCV_VALUE 500

struct ImageItem 
{
	int index;
	int vector;
};


/**
 * HSV image object, transfer the RGB to HSV space
 */
class HSVImage {        
private:

	ImageItem** items;
	CBitmap* bmp;
    CString name;

	int width;
	int height;	

    int hsv[BASELINE_SIZE];   // histogram data      
	int center[BASELINE_SIZE];   
    int continuous[BASELINE_SIZE];   
    int uncontinuous[BASELINE_SIZE];  

public:
    HSVImage(const CString& name, CBitmap* bmp, int adjustWidth, int adjustHeight);
	~HSVImage();
            
	const CString& getName() { return name; }
	CBitmap* getBitmap() { return bmp; }

	void init();
	void build();	

    double getV(unsigned char* pixel);
    double getS(unsigned char* pixel);
    double getH(unsigned char* pixel);
    int quantization(unsigned char* pixel);
    int sum();

	int* getData(int index);

	void buildHSV();
	void buildCCV();
	void buildCenter();

	void stepCCV(int w, int h, int level);
	bool testCCV(int w1, int h1, int w2, int h2);

	double diff(HSVImage* other, int type, int hsv, int ccv, int center);
    double diffHSV(HSVImage* other, int type);
	double diffCCV(HSVImage* other, int type);
	double diffCenter(HSVImage* other, int type);
};



