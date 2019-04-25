#pragma once
#define IMAGEPROCESSDLL_API extern "C" __declspec(dllexport) 
#include <atltypes.h>
#include <vector>
using namespace std;

/********ʶ������п��ܵķ��ؽ��*******/
#define NO_PICTURE 1100   //û��ͼƬ����
#define NOT_FOUND_BOW 1101  //��ʾû�ж�λ����
#define BOW_HEIGHT_ERROR 1102   //��ʾ����λ�ô���
#define NOT_FOUND_LINE 1103    //��ʾû�м����κ�ֱ��
#define NOT_FOUND_OBLIQUE 1104   //��ʾû���ҵ���б���������Ӵ�����
#define NOT_FOUND_CROSSPOINT 1105  //��ʾû���ҵ������Ӵ���

enum BOW_STATUS{BOW_DESCENDING=-1,BOW_MOTIONLESS,BOW_RISING,BOW_NotExist};  //��������״̬(����������ֹ���޹�)

struct NET_LINE   //����ֱ�߽ṹ��
{
	int x1;
	int y1;
	int x2;
	int y2;

	double k;  //б��
	double b;  //�ؾ�

	int number; //����ֱ���Ǻϲ����ֱ�ߣ����ʾ���飨����ֱ�߰�����ԭʼֱ������
	int groupNumber; //��������ʾ��ǰֱ�߹�Ϊ��һ��

	NET_LINE()   //�ṹ��Ĺ��캯��
	{
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
	
		k = 0;
		b = 0;

		number = 1;  //Ĭ����Ŀ��1 ��ֱ�߱�������һ����
		groupNumber = 0;
	}
};

struct RecognitionResult  
{
	bool Bow_Exist; //��ʾ��ǰ֡ͼ�����Ƿ��й�

	int midPx;  //�ܵ繭�е��������
	int midPy;  //�ܵ繭�е�ĺ�����

	int crossPx;
	int crossPy;

	int pixel_BowHeight;  //�ܵ繭�ĵ���ֵ(�������ϼ�������ظ߶�)
	int pixel_PullOut; //�Ӵ���������ֵ(���ص�λ)

	double real_BowHeight;  //ʵ�ʵĵ���ֵ
	double real_PullOut; //ʵ�ʵ�����ֵ	

	std::vector<NET_LINE> net_Lines; //ɸѡ�ϲ����ĽӴ���

	RecognitionResult()
	{
		Bow_Exist = FALSE;

		midPx = 240;
		midPy = 120;

		crossPx = 0;
		crossPy = 0;

		pixel_BowHeight =0;
		pixel_PullOut = 0;

		real_BowHeight = 0;
		real_PullOut = 0;
	}
};



IMAGEPROCESSDLL_API void __stdcall AdjustProcessParameters(int first_Top,int first_Height,int first_Left,int first_Width);//�����������Ҳõ�������������
IMAGEPROCESSDLL_API bool __stdcall createInst(UINT hwnd); //�������ھ��
IMAGEPROCESSDLL_API bool __stdcall setImgSize(int w,int h);  //����ͼ��ߴ�
IMAGEPROCESSDLL_API bool __stdcall setTranNO(LPCTSTR tranno); //���ó���,���ݳ���ѡ����Ӧ�Ĳ���
IMAGEPROCESSDLL_API int __stdcall setRGB(unsigned char* RGB, UINT frameNumber, int* PullOutArray, int* HeightArray, RECT* bowRect); //����RGBͼ����֡��
IMAGEPROCESSDLL_API void __stdcall ReRreshImage(); //ˢ��ͼ��