#pragma once
#define IMAGEPROCESSDLL_API extern "C" __declspec(dllexport) 
#include <atltypes.h>
#include <vector>
using namespace std;

/********识别过程中可能的返回结果*******/
#define NO_PICTURE 1100   //没有图片数据
#define NOT_FOUND_BOW 1101  //表示没有定位出弓
#define BOW_HEIGHT_ERROR 1102   //表示弓的位置错误
#define NOT_FOUND_LINE 1103    //表示没有检测出任何直线
#define NOT_FOUND_OBLIQUE 1104   //表示没有找到倾斜线条（即接触网）
#define NOT_FOUND_CROSSPOINT 1105  //表示没有找到弓网接触点

enum BOW_STATUS{BOW_DESCENDING=-1,BOW_MOTIONLESS,BOW_RISING,BOW_NotExist};  //弓的运行状态(升、降、静止、无弓)

struct NET_LINE   //弓网直线结构体
{
	int x1;
	int y1;
	int x2;
	int y2;

	double k;  //斜率
	double b;  //截距

	int number; //若该直线是合并后的直线，则表示该组（条）直线包含的原始直线数量
	int groupNumber; //索引，表示当前直线归为哪一类

	NET_LINE()   //结构体的构造函数
	{
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
	
		k = 0;
		b = 0;

		number = 1;  //默认数目是1 （直线本身算作一条）
		groupNumber = 0;
	}
};

struct RecognitionResult  
{
	bool Bow_Exist; //表示当前帧图像中是否有弓

	int midPx;  //受电弓中点的纵坐标
	int midPy;  //受电弓中点的横坐标

	int crossPx;
	int crossPy;

	int pixel_BowHeight;  //受电弓的导高值(从下往上计算的像素高度)
	int pixel_PullOut; //接触网的拉出值(像素单位)

	double real_BowHeight;  //实际的导高值
	double real_PullOut; //实际的拉出值	

	std::vector<NET_LINE> net_Lines; //筛选合并出的接触网

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



IMAGEPROCESSDLL_API void __stdcall AdjustProcessParameters(int first_Top,int first_Height,int first_Left,int first_Width);//调整上下左右裁掉的行数与列数
IMAGEPROCESSDLL_API bool __stdcall createInst(UINT hwnd); //创建窗口句柄
IMAGEPROCESSDLL_API bool __stdcall setImgSize(int w,int h);  //设置图像尺寸
IMAGEPROCESSDLL_API bool __stdcall setTranNO(LPCTSTR tranno); //设置车号,根据车号选择相应的参数
IMAGEPROCESSDLL_API int __stdcall setRGB(unsigned char* RGB, UINT frameNumber, int* PullOutArray, int* HeightArray, RECT* bowRect); //传递RGB图像及其帧号
IMAGEPROCESSDLL_API void __stdcall ReRreshImage(); //刷新图像