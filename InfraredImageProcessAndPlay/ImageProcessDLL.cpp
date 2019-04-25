// ImageProcessDLL.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "ImageProcessDLL.h"
#include "ImageProcess.h"

Height_Stagger h_s;

void  __stdcall AdjustProcessParameters(int first_Top,int first_Height,int first_Left,int first_Width)
{
	h_s.AdjustParameters(first_Top,first_Height,first_Left,first_Width);
}
bool __stdcall createInst(UINT hwnd)
{
	return h_s.GetShowWindowHwnd(hwnd);
}

bool __stdcall setImgSize(int w,int h)
{
	return h_s.SetImageSize(w,h);
}

bool __stdcall setTranNO(LPCTSTR tranno)
{
	return h_s.SetTrainName(tranno);
}

int __stdcall setRGB(unsigned char* RGB, UINT frameNumber, int* PullOutArray, int* HeightArray, RECT* bowRect)
{
	return h_s.InfraRedImage_ANALYZE(RGB, frameNumber, PullOutArray, HeightArray, bowRect);
}

void __stdcall ReRreshImage()
{
	h_s.ReRreshImage();
}
