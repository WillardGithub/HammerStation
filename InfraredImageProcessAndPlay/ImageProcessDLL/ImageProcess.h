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


/********ʶ������п��ܵķ��ؽ��*******/
struct BOW_NET_LINE   //����ֱ�߽ṹ��
{
	cv::Point pt1;	//ֱ�ߵ���˵�
	cv::Point pt2;	//ֱ�ߵ��Ҷ˵�
	double k;  //б��
	double b;  //�ؾ�
	double maxD;  //ͬ��ֱ�������ļ������

	int number; //����ֱ���Ǻϲ����ֱ�ߣ����ʾ���飨����ֱ�߰�����ԭʼֱ������
	int groupNumber;  //��������ʾ��ǰֱ�߹�Ϊ��һ��

	BOW_NET_LINE()   //�ṹ��Ĺ��캯��
	{
		pt1 = cv::Point(0, 0);
		pt2 = cv::Point(0, 0);
		k = 0;
		b = 0;
		number = 1;  //Ĭ����Ŀ��1 ��ֱ�߱�������һ����
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

struct BOW_NET_ANGLE   //�����нǽṹ��
{
	BOW_NET_LINE line1, line2;	// �нǵ�����,line1��ʾˮƽ�ߣ�line2 ��ʾб��
	int angle;   // �Ƕ�ֵ���ǻ���ֵ��
	cv::Point cross; // ����

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
	int pre_nWidth;  //ǰһ֡ͼ��Ŀ��
	int pre_nHeight; //ǰһ֡ͼ��ĸ߶�

	int cur_nWidth;  //��ǰ֡ͼ��Ŀ��
	int cur_nHeight; //��ǰ֡ͼ��ĸ߶�

	/*****��һ�βü��ı߽磨�õ�ԭʼͼ����������ұ߽��������Ϣ������ͼ��Ϊ��Ҫ�����ԭͼ��*****/
	int nFirstCut_TopEdge;   //��ʼ��
	int nFirstCut_Height;  //�ü��ĸ߶ȣ�������
	int nFirstCut_LeftEdge;  //��ʼ��
	int nFirstCut_Width;   //�ü��Ŀ�ȣ�������

	/*****�ڶ��βü��ı߽磨�ڵ�һ�βü���Ļ����Ͻ�һ����С��������ȷ���ܵ繭λ�ã�*****/
	int nSecondCut_TopEdge;
	int nSecondCut_Height;
	int nSecondCut_LeftEdge; 
	int nSecondCut_Width;

	/*****�����βü��ı߽磨�����ܵ繭��λ�õĸߵ����ڵ�һ�βü���ԭͼ�����ϲü���ͬ��С��ROI���������ҵ��Ӵ��ߣ�*****/
	int nThirdCut_LeftEdge;
	int nThirdCut_Width;


	/****ѵ����ʶ���ܵ繭****/
	CascadeClassifier cascade; //������������������ 
	String cascadeName; //ѵ�������ɵ�xml�ļ�
	bool IsLoadXMLFile; //�Ƿ��Ѽ��ط�����XML�ĵ�
	vector<Rect> bow_rects; //ѵ����ʶ�𵽵��ܵ繭����ľ��ο���� 

	short* pCountHArray; //������
	
	Mat src_MatRgb; //ԭʼ��ͨ������
	Mat src_Mat; //ԭʼ����ֱ�ӽص�ͼ����ĸ��߽���ͼ��(�Ҷ�ͼ)

	Mat inPut_PreProcessMat;  //Ԥ������������ڿ���src_Mat
	Mat input_PreProcessROIMat;  //Ԥ��������н�ȡsrc_Mat���м䲿��
	Mat input_PreProcessROIBinMat;  //ROI����Ķ�ֵͼ��


	Mat input_PreProcessROICannyMat;  //ROI�����Cannyͼ��(���Ҳ�������ʱ��һ�����ſջ�ɽ��ʱĿ��䰵���Ҳ���������ҪCanny���ӵó��߽�ͼ��ȷ������λ��)

	Mat inPut_RecognizeMat;  //����ʶ����������ڿ���src_Mat
	Mat inPut_RecognizeROIMat;   //����Ԥ�����������ܵ繭λ�ý�ȡ���ܵ繭���Ϸ�����ͼ��
	Mat inPut_RecognizeROIBinMat; //��ȡ���Ϸ�����Ķ�ֵͼ��
	Mat gauss_Mat; //Canny��������ǰ�ĸ�˹ģ��

	int cut_UpRows; //���ݹ���λ�ýص�������
	int cut_LeftCols; //���ݹ���λ�ýص�������
	
	vector<cv::Vec4i> dectecedLines; //ROIͼ���⵽�����е�ֱ��
	vector<BOW_NET_LINE> detectedInclines; //ROIͼ��ɸѡ������б����
	vector<BOW_NET_LINE> ComInclines; //ROIͼ��ϲ����ֱ��
	BOW_NET_LINE temp_Incline; //����ѡ�е���бֱ��
	cv::Point ptI1,ptI2; //������бֱ�ߵ������˵�
	BOW_NET_LINE lineteamI; //��бֱ�߷���ʱ����ʱ�������

	BOOL Is_LineSatisfied; //�Ƿ��ҵ�����������б��
	double vlK;  //���崹ֱ�ߵ�б��

	int MaxValue_Pixel; //��¼�������ֵ
	int MaxCount_Pixel; //��¼�������ֵ

	Point cP; //���ڼ�¼�����Ӵ���(�ڹ��Ϸ���ROIͼ���µ�����)

	vector<int> bow_station; //��¼ÿһ֡����λ��
	vector<int> stateRecord; //��¼ÿ�����֡��λ�ñȽϵĽ��

	unsigned char* pixelImage; //ÿ֡ͼ��RGB����Ļ�����
	int* pHist; //ͳ�ƻҶ�ֵ����

	ImageParameter()
	{
		pre_nHeight = 240;
		pre_nWidth = 320;

		cur_nHeight = 240;
		cur_nWidth = 320;

		pCountHArray = (short*)malloc(sizeof(short)*cur_nHeight); //����������

		nFirstCut_TopEdge = 30;     
		nFirstCut_Height = 180;
		nFirstCut_LeftEdge = 30;
		nFirstCut_Width = 260;

		//nFirstCut_TopEdge = 0;
		//nFirstCut_Height = 240;
		//nFirstCut_LeftEdge = 0;
		//nFirstCut_Width = 320;


		src_Mat = cvCreateMat(nFirstCut_Height,nFirstCut_Width,CV_8UC1);  //��ʼ����ֱ�ӽص�ͼ����ĸ��߽���ͼ��

		nSecondCut_TopEdge = 0;
		nSecondCut_Height = src_Mat.rows;
		nSecondCut_LeftEdge = src_Mat.cols/6;
		nSecondCut_Width = src_Mat.cols*2/3;

	//	nThirdCut_LeftEdge = src_Mat.cols/8;
	//	nThirdCut_Width = src_Mat.cols*3/4;

		nThirdCut_LeftEdge = 0;
		nThirdCut_Width = src_Mat.cols;



		inPut_PreProcessMat = cvCreateMat(src_Mat.rows,src_Mat.cols,CV_8UC1);  //Ԥ������������ڿ���src_Mat
		input_PreProcessROIMat = cvCreateMat(nSecondCut_Height,nSecondCut_Width,CV_8UC1); //Ԥ��������н�ȡ��ROI
		input_PreProcessROIBinMat = cvCreateMat(input_PreProcessROIMat.rows,input_PreProcessROIMat.cols,CV_8UC1);

		input_PreProcessROICannyMat = cvCreateMat(input_PreProcessROIMat.rows,input_PreProcessROIMat.cols,CV_8UC1); //Canny�߽�ͼ��

		inPut_RecognizeMat = cvCreateMat(src_Mat.rows,src_Mat.cols,CV_8UC1);  //����ʶ����������ڿ���src_Mat
		inPut_RecognizeROIMat = cvCreateMat(inPut_RecognizeMat.rows/3,nThirdCut_Width,CV_8UC1);
		inPut_RecognizeROIBinMat = cvCreateMat(inPut_RecognizeROIMat.rows,inPut_RecognizeROIMat.cols,CV_8UC1); //���ݶ�λ���Ĺ�λ�ý�ȡ��ROI
		
		Is_LineSatisfied = FALSE; 
	
		cut_UpRows = 0; //���ݹ���λ�ýص�������
		cut_LeftCols = 0; //�ص�������
		vlK = 100000.00;

		cascadeName = "\\GuidePull\\cascade.xml"; //ѵ������xml�ĵ�   //��Ҫ��ԭ
		
		IsLoadXMLFile = false;

		pixelImage = (uchar*)malloc(sizeof(uchar)*cur_nWidth*cur_nHeight);
		memset(pixelImage, 0, sizeof(uchar)*cur_nWidth*cur_nHeight);
		pHist = (int*)malloc(sizeof(int)*256);
		memset(pHist, 0, sizeof(int) * 256);
	}
};

struct RecResult  //ʶ��Ľ������
{
	bool IsCur_BowExist; //��ʾ��ǰ֡ͼ�����Ƿ��й�	
	bool IsPre_BowExist; //��ʾ��һ֡ͼ���ʼ�жϵ��Ƿ��й� �����ڵ�ǰ֡��ʼ�ж�Ϊ�޹��������һ֡�ĳ�ʼ�ж��������������

	Point midPoint; //�����е�����(ԭʼͼ���е�λ��)
	vector<BOW_NET_ANGLE> cross_Angles; //��¼���μ���������Ӵ��㼰����Ӧ�ļн�
	
	
	Point pre_cross_Point; //��һ֡�ܵ繭��Ӵ����Ľ�������
	Point cross_Point;  //��ǰ֡�ܵ繭��Ӵ����Ľ�������

	int pre_pixelBowHeight; //ǰһ֡�ĵ���ֵ(���ص�λ)
	int pixel_BowHeight; //�ܵ繭�ĵ���ֵ(���ص�λ)
	int pixel_PullOut; //�Ӵ���������ֵ(���ص�λ)

	double real_BowHeight;  //ʵ�ʵĵ���ֵ
	double real_PullOut; //ʵ�ʵ�����ֵ	

	Point cross_Point2; //˫֧��ʱ��һ������
	double real_PullOut2; //˫֧��ʱ��һ�����������ֵ

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

		pre_cross_Point.x = 0; //��ʼ��Ϊ0,��ʾ��������һ֡�ĽӴ���
		pre_cross_Point.y = 0;

		pre_pixelBowHeight = 0; //��ʼ��Ϊ0����ʾ��������һ֡�ĵ���ֵ

		pixel_BowHeight = 0;
		pixel_PullOut = 0;

		real_BowHeight = 0;
		real_PullOut = 0;
		real_PullOut2 = 0;
	}
};

/****���嵼��������****/
class Height_Stagger
{
public:
	Height_Stagger();
	~Height_Stagger();

private:
	ImageParameter image_Para;
	RecResult recResult; 
	RecognitionResult BowNet_Result;
	Rect bowRange;  //�ܵ繭���ο�
	
	string trainName; //�г�����
	CvvImage m_showImg;  //��ͼ�������������

	int TopBowHeightScale;  //ͼ�񶥶˶�Ӧ����󵼸�ֵ
	int BottomBowHeightScale;  //ͼ���ϵ���ֵ����С�̶�(ȡ��)
	int MinBHFromImgBottom ; //��С���߿̶�ֵ����ͼ��׶˵����ؾ���
	int IntervalNumber;  //����ֵ�̶ȶ�Ӧ���ٶ�

	int heightInterval; //����ֵ�̶ȶ�Ӧ�����ؼ��
	int PullOutPixelSpan; //���������ֵ��������ط�Χ
	int pulloutInterval; //ÿ100mm��Ӧ�������ؿ̶�
	int AlarmHeightPixel; //���߱����߶�Ӧ������λ�ã��������£�


	string d_g;  //�������
	char daogao[5];
	char c_dg[10];

	string s_g;   //�������
	char lachu[5] ;
	char c_sg[10] ;

	string z_h;   //֡�����
	char frame_number[5];
	char c_zh[10];
	

	HWND h_wnd; //���ھ��
	HDC hdc;  //�豸������
	RECT rect; //������ʾ����

	//Mat ScaleResultMat;  //���ŵ�ͼ������ʾ���ڳߴ���ͬ��
	int ObliqueNumber; //��¼�ڼ���б��Ϊ���ĽӴ���

	TCHAR* szPath;  //�ļ���·��
	char* chRtn;  //�ļ���·��

	double xRatio; //x����ı���
	double yRatio; //y����ı���
	int camera_Height; //��������ĸ߶�

	int LeftMaxPullOut; //����ƫ���е��������ص�λ
	int RightMaxPullOut; //����ƫ���е��������ص�λ

	int nContinueNoBow; //�������ܵ繭��֡��
	int nContinueBigInterval; //����������֮֡�����ϴ��֡��

	bool IsRecByTrainer; //�Ƿ��Ѿ���ѵ����ʶ���ܵ繭����
	bool IsBowRectExist; //�ܵ繭�Ƿ����
	bool loadFlag;  //����XML�ĵ��ı��
	int nFrame; //��¼֡��

public:
	int InfraRedImage_ANALYZE(const unsigned char* pGBuffer, UINT iFrame,int*  pullOutValue,int* HeightValue,RECT* bowRegion);
	void AdjustParameters(int cut_Top,int cut_Height,int cut_Left,int cut_Width);
	bool GetShowWindowHwnd(UINT input_hwnd); //��ȡ��ʾ���ڵľ��
	bool SetImageSize(int nWidth,int nHeight); //��������ͼ��ߴ�
	bool SetTrainName(LPCTSTR t_name);  //�趨����
	void ReRreshImage();


private:
	int GrayBufferToMat(const unsigned char* pArr); //��bufferת��ΪMatͼ��ֱ�Ӽ����˱߽�����
	int BowNet_Recognize(RecResult& ret);//������ʶ��������Ҫ��Ѱ�ҽӴ���
	bool DetectBowRectRegion(Mat& src_gray_Mat,vector<Rect>& rects); //����ѵ����ʶ���ܵ繭�Ƿ����
	bool LoadCalibrationParameters(); //�������ڼ��㵼�������ı궨����

	bool DrawRecognizeResult(Mat resultMat,int recFrame);  //����ʶ��Ч��ͼ

	int Pre_Process(int& bow_H) ;//ͼ���Ԥ��������ȷ���ܵ繭��λ��	
	int ConfirmBowPosition(int& HL); //����ROIͼ��Ѱ�ҹ���λ��
	
	int EdgeToBlack(Mat& src,int rowNum,int colNum); //��ͼ����������ұ߽�ȫ����Ϊ0
	int GetMaxNumberValue(Mat InputMat);   //��ȡ������������ֵ
	int GetMaxPixelValue(Mat InputMat);   //��ȡͼ����������ֵ
	int GetMinPixelValue(Mat InputMat);   //��ȡͼ�����С����ֵ

	vector<BOW_NET_LINE> CombineOblique(); //�ϲ���������б����
	vector<BOW_NET_ANGLE> Detect_Angles(int bow_row,vector<BOW_NET_ANGLE>& angles) ; //�����б�����빭�ļн�
	cv::Point SelectCrossPointByDistance(RecResult& calPoints); //���ݽ������ܵ繭�е�ľ��롢б���Լ���ǰ֡����һ֡�Ĳ�ֵɸѡ�Ӵ���
	int CalculateSpaceCoordinateByScaleRatio(int bowHeight, int PullOut,double& RealHeight,double& RealStagger); //ͼ�����껹ԭΪ�ռ�����(���ñ���)	
};

