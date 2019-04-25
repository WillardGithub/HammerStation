#pragma once

#include "cv.h" 
#include "highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "CvvImage.h"
#include <queue>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include "ImageProcessDLL.h"
#include "CvxText.h"
#include "PutText.h"
using namespace cv;
using namespace std;


/********识别过程中可能的返回结果*******/
struct BOW_NET_LINE   //弓网直线结构体
{
	cv::Point pt1;	//直线的左端点
	cv::Point pt2;	//直线的右端点
	double k;  //斜率
	double b;  //截距
	double maxD;  //同组直线中最大的间隔距离

	int number; //若该直线是合并后的直线，则表示该组（条）直线包含的原始直线数量
	int groupNumber;  //索引，表示当前直线归为哪一类

	BOW_NET_LINE()   //结构体的构造函数
	{
		pt1 = cv::Point(0, 0);
		pt2 = cv::Point(0, 0);
		k = 0;
		b = 0;
		number = 1;  //默认数目是1 （直线本身算作一条）
		groupNumber = 0;
		maxD = 0.0;
	}

	BOW_NET_LINE operator= (const BOW_NET_LINE &hangline)
	{
		pt1 = hangline.pt1;
		pt2 = hangline.pt2;
		k = hangline.k;
		b = hangline.b;
		number = hangline.number;
		groupNumber = hangline.groupNumber;
		return *this;
	}
};

struct BOW_NET_ANGLE   //弓网夹角结构体
{
	BOW_NET_LINE line1, line2;	// 夹角的两边,line1表示水平线，line2 表示斜线
	int angle;   // 角度值（非弧度值）
	cv::Point cross; // 交点

	BOW_NET_ANGLE()
	{
		line1 = BOW_NET_LINE();
		line2 = BOW_NET_LINE();
		angle = 0;
		cross = cv::Point(0, 0);
	}

	BOW_NET_ANGLE operator= (const BOW_NET_ANGLE &hangangle)
	{
		line1 = hangangle.line1;
		line2 = hangangle.line2;
		angle = hangangle.angle;
		cross = hangangle.cross;
		return *this;
	}
};

struct ImageParameter
{
	int pre_nWidth;  //前一帧图像的宽度
	int pre_nHeight; //前一帧图像的高度

	int cur_nWidth;  //当前帧图像的宽度
	int cur_nHeight; //当前帧图像的高度

	/*****第一次裁剪的边界（裁掉原始图像的上下左右边界的无用信息，将此图作为需要处理的原图）*****/
	int nFirstCut_TopEdge;   //起始行
	int nFirstCut_Height;  //裁剪的高度（行数）
	int nFirstCut_LeftEdge;  //起始列
	int nFirstCut_Width;   //裁剪的宽度（列数）

	/*****第二次裁剪的边界（在第一次裁剪后的基础上进一步缩小区域，用于确定受电弓位置）*****/
	int nSecondCut_TopEdge;
	int nSecondCut_Height;
	int nSecondCut_LeftEdge; 
	int nSecondCut_Width;

	/*****第三次裁剪的边界（根据受电弓的位置的高低再在第一次裁剪的原图基础上裁剪不同大小的ROI区域用于找到接触线）*****/
	int nThirdCut_LeftEdge;
	int nThirdCut_Width;


	/****训练器识别受电弓****/
	CascadeClassifier cascade; //创建级联分类器对象 
	String cascadeName; //训练器生成的xml文件
	bool IsLoadXMLFile; //是否已加载分类器XML文档
	vector<Rect> bow_rects; //训练器识别到的受电弓区域的矩形块个数 

	short* pCountHArray; //行数组
	
	Mat src_MatRgb; //原始三通道数据
	Mat src_Mat; //原始数据直接截掉图像的四个边界后的图像(灰度图)

	Mat inPut_PreProcessMat;  //预处理过程中用于拷贝src_Mat
	Mat input_PreProcessROIMat;  //预处理过程中截取src_Mat的中间部分
	Mat input_PreProcessROIBinMat;  //ROI区域的二值图像


	Mat input_PreProcessROICannyMat;  //ROI区域的Canny图像(在找不到导高时，一般在桥空或山洞时目标变暗会找不到弓，需要Canny算子得出边界图在确定弓的位置)

	Mat inPut_RecognizeMat;  //弓网识别过程中用于拷贝src_Mat
	Mat inPut_RecognizeROIMat;   //根据预处理计算出的受电弓位置截取的受电弓的上方区域图像
	Mat inPut_RecognizeROIBinMat; //截取弓上方区域的二值图像
	Mat gauss_Mat; //Canny算子作用前的高斯模糊

	int cut_UpRows; //根据弓的位置截掉的行数
	int cut_LeftCols; //根据弓的位置截掉的列数
	
	vector<cv::Vec4i> dectecedLines; //ROI图像检测到的所有的直线
	vector<BOW_NET_LINE> detectedInclines; //ROI图像筛选出的倾斜线条
	vector<BOW_NET_LINE> ComInclines; //ROI图像合并后的直线
	BOW_NET_LINE temp_Incline; //保存选中的倾斜直线
	cv::Point ptI1,ptI2; //保存倾斜直线的两个端点
	BOW_NET_LINE lineteamI; //倾斜直线分组时的临时缓存变量

	BOOL Is_LineSatisfied; //是否找到满足条件的斜线
	double vlK;  //定义垂直线的斜率

	int MaxValue_Pixel; //记录最大像素值
	int MaxCount_Pixel; //记录最多像素值

	Point cP; //用于记录弓网接触点(在弓上方的ROI图像下的坐标)

	vector<int> bow_station; //记录每一帧弓的位置
	vector<int> stateRecord; //记录每间隔两帧弓位置比较的结果

	unsigned char* pixelImage; //每帧图像RGB数组的缓冲区
	int* pHist; //统计灰度值个数

	ImageParameter()
	{
		pre_nHeight = 240;
		pre_nWidth = 320;

		cur_nHeight = 240;
		cur_nWidth = 320;

		pCountHArray = (short*)malloc(sizeof(short)*cur_nHeight); //分配行数组

		nFirstCut_TopEdge = 30;     
		nFirstCut_Height = 180;
		nFirstCut_LeftEdge = 30;
		nFirstCut_Width = 260;

		//nFirstCut_TopEdge = 0;
		//nFirstCut_Height = 240;
		//nFirstCut_LeftEdge = 0;
		//nFirstCut_Width = 320;


		src_Mat = cvCreateMat(nFirstCut_Height,nFirstCut_Width,CV_8UC1);  //初始数据直接截掉图像的四个边界后的图像

		nSecondCut_TopEdge = 0;
		nSecondCut_Height = src_Mat.rows;
		nSecondCut_LeftEdge = src_Mat.cols/6;
		nSecondCut_Width = src_Mat.cols*2/3;

	//	nThirdCut_LeftEdge = src_Mat.cols/8;
	//	nThirdCut_Width = src_Mat.cols*3/4;

		nThirdCut_LeftEdge = 0;
		nThirdCut_Width = src_Mat.cols;



		inPut_PreProcessMat = cvCreateMat(src_Mat.rows,src_Mat.cols,CV_8UC1);  //预处理过程中用于拷贝src_Mat
		input_PreProcessROIMat = cvCreateMat(nSecondCut_Height,nSecondCut_Width,CV_8UC1); //预处理过程中截取的ROI
		input_PreProcessROIBinMat = cvCreateMat(input_PreProcessROIMat.rows,input_PreProcessROIMat.cols,CV_8UC1);

		input_PreProcessROICannyMat = cvCreateMat(input_PreProcessROIMat.rows,input_PreProcessROIMat.cols,CV_8UC1); //Canny边界图像

		inPut_RecognizeMat = cvCreateMat(src_Mat.rows,src_Mat.cols,CV_8UC1);  //弓网识别过程中用于拷贝src_Mat
		inPut_RecognizeROIMat = cvCreateMat(inPut_RecognizeMat.rows/3,nThirdCut_Width,CV_8UC1);
		inPut_RecognizeROIBinMat = cvCreateMat(inPut_RecognizeROIMat.rows,inPut_RecognizeROIMat.cols,CV_8UC1); //根据定位出的弓位置截取的ROI
		
		Is_LineSatisfied = FALSE; 
	
		cut_UpRows = 0; //根据弓的位置截掉的行数
		cut_LeftCols = 0; //截掉的列数
		vlK = 100000.00;

		cascadeName = "\\GuidePull\\cascade.xml"; //训练器的xml文档   //需要还原
		
		IsLoadXMLFile = false;

		pixelImage = (uchar*)malloc(sizeof(uchar)*cur_nWidth*cur_nHeight);
		memset(pixelImage, 0, sizeof(uchar)*cur_nWidth*cur_nHeight);
		pHist = (int*)malloc(sizeof(int)*256);
		memset(pHist, 0, sizeof(int) * 256);
	}
};

struct RecResult  //识别的结果参数
{
	bool IsCur_BowExist; //表示当前帧图像中是否有弓	
	bool IsPre_BowExist; //表示上一帧图像初始判断的是否有弓 （对于当前帧初始判断为无弓会根据上一帧的初始判断情况进行修正）

	Point midPoint; //弓的中点坐标(原始图像中的位置)
	vector<BOW_NET_ANGLE> cross_Angles; //记录初次计算出的所接触点及其相应的夹角
	
	
	Point pre_cross_Point; //上一帧受电弓与接触网的交点坐标
	Point cross_Point;  //当前帧受电弓与接触网的交点坐标

	int pre_pixelBowHeight; //前一帧的导高值(像素单位)
	int pixel_BowHeight; //受电弓的导高值(像素单位)
	int pixel_PullOut; //接触网的拉出值(像素单位)

	double real_BowHeight;  //实际的导高值
	double real_PullOut; //实际的拉出值	

	Point cross_Point2; //双支线时另一个交点
	double real_PullOut2; //双支线时另一个交点的拉出值

	RecResult()
	{
		IsCur_BowExist = FALSE;
		IsPre_BowExist = FALSE;

		midPoint.x = 160;
		midPoint.y = 120;

		cross_Point.x = 0;
		cross_Point.y = 0;

		cross_Point2.x = 0;
		cross_Point2.y = 0; 

		pre_cross_Point.x = 0; //初始化为0,表示不存在上一帧的接触点
		pre_cross_Point.y = 0;

		pre_pixelBowHeight = 0; //初始化为0，表示不存在上一帧的导高值

		pixel_BowHeight = 0;
		pixel_PullOut = 0;

		real_BowHeight = 0;
		real_PullOut = 0;
		real_PullOut2 = 0;
	}
};

/****定义导高拉出类****/
class Height_Stagger
{
public:
	Height_Stagger();
	~Height_Stagger();

private:
	ImageParameter image_Para;
	RecResult recResult; 
	RecognitionResult BowNet_Result;
	Rect bowRange;  //受电弓矩形框
	
	string trainName; //列车车号
	CvvImage m_showImg;  //将图像输出到窗口上

	int TopBowHeightScale;  //图像顶端对应的最大导高值
	int BottomBowHeightScale;  //图像上导高值的最小刻度(取整)
	int MinBHFromImgBottom ; //最小导高刻度值距离图像底端的像素距离
	int IntervalNumber;  //导高值刻度对应多少段

	int heightInterval; //导高值刻度对应的像素间隔
	int PullOutPixelSpan; //左或右拉出值的最大像素范围
	int pulloutInterval; //每100mm对应的列像素刻度
	int AlarmHeightPixel; //导高报警线对应的像素位置（从上往下）


	string d_g;  //导高输出
	char daogao[5];
	char c_dg[10];

	string s_g;   //拉出输出
	char lachu[5] ;
	char c_sg[10] ;

	string z_h;   //帧号输出
	char frame_number[5];
	char c_zh[10];
	

	HWND h_wnd; //窗口句柄
	HDC hdc;  //设备上下文
	RECT rect; //窗口显示区域

	//Mat ScaleResultMat;  //缩放的图像（与显示窗口尺寸相同）
	int ObliqueNumber; //记录第几条斜线为最后的接触网

	TCHAR* szPath;  //文件夹路径
	char* chRtn;  //文件夹路径

	double xRatio; //x方向的比率
	double yRatio; //y方向的比率
	int camera_Height; //相机离基面的高度

	int LeftMaxPullOut; //向左偏离中点的最大像素单位
	int RightMaxPullOut; //向右偏离中点的最大像素单位

	int nContinueNoBow; //连续无受电弓的帧数
	int nContinueBigInterval; //连续相邻两帧之间间隔较大的帧数

	bool IsRecByTrainer; //是否已经用训练器识别受电弓存在
	bool IsBowRectExist; //受电弓是否存在
	bool loadFlag;  //加载XML文档的标记
	int nFrame; //记录帧号

public:
	int InfraRedImage_ANALYZE(const unsigned char* pGBuffer, UINT iFrame,int*  pullOutValue,int* HeightValue,RECT* bowRegion);
	void AdjustParameters(int cut_Top,int cut_Height,int cut_Left,int cut_Width);
	bool GetShowWindowHwnd(UINT input_hwnd); //获取显示窗口的句柄
	bool SetImageSize(int nWidth,int nHeight); //重新设置图像尺寸
	bool SetTrainName(LPCTSTR t_name);  //设定车号
	void ReRreshImage();


private:
	int GrayBufferToMat(const unsigned char* pArr); //将buffer转换为Mat图像（直接剪掉了边界区域）
	int BowNet_Recognize(RecResult& ret);//弓网的识别函数，主要是寻找接触点
	bool DetectBowRectRegion(Mat& src_gray_Mat,vector<Rect>& rects); //利用训练器识别受电弓是否存在
	bool LoadCalibrationParameters(); //加载用于计算导高拉出的标定参数

	bool DrawRecognizeResult(Mat resultMat,int recFrame);  //绘制识别效果图

	int Pre_Process(int& bow_H) ;//图像的预处理函数，确定受电弓的位置	
	int ConfirmBowPosition(int& HL); //根据ROI图像，寻找弓的位置
	
	int EdgeToBlack(Mat& src,int rowNum,int colNum); //将图像的上下左右边界全部置为0
	int GetMaxNumberValue(Mat InputMat);   //获取最多个数的像素值
	int GetMaxPixelValue(Mat InputMat);   //获取图像的最大像素值
	int GetMinPixelValue(Mat InputMat);   //获取图像的最小像素值

	vector<BOW_NET_LINE> CombineOblique(); //合并检测出的倾斜线条
	vector<BOW_NET_ANGLE> Detect_Angles(int bow_row,vector<BOW_NET_ANGLE>& angles) ; //检测倾斜线条与弓的夹角
	cv::Point SelectCrossPointByDistance(RecResult& calPoints); //根据交点离受电弓中点的距离、斜率以及当前帧与上一帧的差值筛选接触点
	int CalculateSpaceCoordinateByScaleRatio(int bowHeight, int PullOut,double& RealHeight,double& RealStagger); //图像坐标还原为空间坐标(利用比率)	
};

