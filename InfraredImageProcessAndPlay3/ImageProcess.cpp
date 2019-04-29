#include "stdafx.h"
#include "ImageProcess.h"

Height_Stagger::Height_Stagger()
{
	nContinueNoBow = 0;
	nContinueBigInterval = 0;
	IsRecByTrainer = false;
	IsBowRectExist = false;
	loadFlag = false;
	ObliqueNumber = 0;

	//fontType = NULL;
	trainName = "";
	h_wnd = NULL;
	hdc = NULL;
	nFrame = 0;

	xRatio = 7.96875;   //初始化列方向的比率
	yRatio = 8.04167;    //初始化行方向的比率
	camera_Height = 5200;  //初始化相机的高度

	LeftMaxPullOut = 0;
	RightMaxPullOut = 0; 

	d_g = "导高：";
	s_g = "拉出：";
	z_h = "帧号：";

	TopBowHeightScale = 0;  //图像顶端对应的最大导高值
	BottomBowHeightScale = 0; //图像上导高值的最小刻度(取整)
	MinBHFromImgBottom = 0; //最小导高刻度值距离图像底端的像素距离
	IntervalNumber = 0;  //导高值刻度对应多少段
	heightInterval = 0; //导高值刻度对应的像素间隔
	PullOutPixelSpan = 0; //左或右拉出值的最大像素范围
	pulloutInterval = 0; //每100mm对应的列像素刻度
	AlarmHeightPixel = 0; //导高报警值对应的像素位置

}
Height_Stagger::~Height_Stagger()
{
	nContinueNoBow = 0;
	nContinueBigInterval = 0;
	IsRecByTrainer = false;
	IsBowRectExist = false;
	loadFlag = false;
	ObliqueNumber = 0;

	LeftMaxPullOut = 0;
	RightMaxPullOut = 0; 
	AlarmHeightPixel = 0;

	trainName = "";
	h_wnd = NULL;
	hdc = NULL;
	nFrame = 0;

	if(image_Para.pCountHArray)
	{
		free(image_Para.pCountHArray);
		image_Para.pCountHArray = NULL;
	}
	if(image_Para.pixelImage)
	{
		free(image_Para.pixelImage);
		image_Para.pixelImage = NULL;
	}
	if(image_Para.pHist)
	{
		free(image_Para.pHist);
		image_Para.pHist = NULL;
	}
}

bool Height_Stagger::LoadCalibrationParameters()   //加载标定参数
{
	TCHAR* executePath = NULL;
	char* charExcPath = NULL;

	executePath  = new TCHAR[1000*sizeof(TCHAR)];
	memset(executePath ,0,1000*sizeof(TCHAR));
	GetModuleFileName(NULL,executePath ,1000);
	int iLen = WideCharToMultiByte(CP_ACP, 0,executePath , -1, NULL, 0, NULL, NULL);
	charExcPath =new char[iLen*sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, executePath , -1, charExcPath, iLen, NULL, NULL);

	std::string ParameterPath(charExcPath);
	string postfix= "GuidePull\\InfraredCalibrationPara.txt";  //配置文件
	int station = ParameterPath.find_last_of("/\\");
	if(station>1)
	{
		ParameterPath = ParameterPath.substr(0,station+1);
		ParameterPath = ParameterPath + postfix;
	}

	ifstream in(ParameterPath);  
	if(in)   // 有该文件  
	{  
		string line_str = ""; 
		int position = 0;
		while (getline(in, line_str))    //line_str中不包括每行的换行符\n  
		{   
			position = line_str.find(":");
			string trainNO  = line_str.substr(0,position);
			int compare_Result = trainNO.compare(trainName);
			if(compare_Result == 0)
			{
				break;
			}
		}  
		vector<char*> str_para;
		string sub_str =  line_str.substr(position+1,line_str.length());

		char* para_Path = new char[sub_str.length()+1];
		strcpy(para_Path,sub_str .c_str());

		char* str_tt = strtok(para_Path," ");
		str_para.push_back(str_tt);

		while(str_tt !=NULL)    //将字符串中的变量值按空格分割后存储
		{ 
			str_tt = strtok(NULL," "); 
			if(str_tt== NULL)
			{
				break;
			}
			str_para.push_back(str_tt);
		} 
		
		recResult.midPoint.x = atoi(str_para.at(0));  //弓的中点在原始图像中的列坐标（相当于是指定弓中点的列坐标）
		camera_Height = atoi(str_para.at(1));
		xRatio = atof(str_para.at(2));
		yRatio = atof(str_para.at(3));

		TopBowHeightScale = (int)(camera_Height + yRatio*image_Para.cur_nHeight+0.5);  //图像顶端对应的最大导高值
		BottomBowHeightScale = (int)(camera_Height + 100 +0.5);  //图像上导高值的最小刻度(取整)
		MinBHFromImgBottom = 100/yRatio; //最小导高刻度值距离图像底端的像素距离
		IntervalNumber = (TopBowHeightScale - BottomBowHeightScale) /100;  //导高值刻度对应多少段

		heightInterval = (int)(100/yRatio+0.5); //导高值刻度对应的像素间隔
		PullOutPixelSpan = (int)(600/xRatio+0.5); //左或右拉出值的最大像素范围
		pulloutInterval = (int)(100/xRatio+0.5); //拉出值刻度对应的像素间隔

		RightMaxPullOut = recResult.midPoint.x + (int)(600/xRatio+0.5);  //向右最大拉出像素位置
		LeftMaxPullOut = recResult.midPoint.x - (int)(600/xRatio+0.5);   //向左最大拉出像素位置


		if (image_Para.cur_nHeight == 240)
		AlarmHeightPixel = (int)(240 - ((6600 - camera_Height)/yRatio) +0.5);  //根据比率关系算出导高报警时对应的像素值
		else
		AlarmHeightPixel = (int)(480 - ((6600 - camera_Height)/yRatio) + 0.5);  //根据比率关系算出导高报警时对应的像素值


		IsRecByTrainer = false;   //每次切换车号时，重新初始化为false 
		BowNet_Result.Bow_Exist = FALSE;
		BowNet_Result.crossPx = 0;
		BowNet_Result.crossPy = 0;
		BowNet_Result.pixel_BowHeight =0;
		BowNet_Result.pixel_PullOut = 0;
		BowNet_Result.real_BowHeight = 0;
		BowNet_Result.real_PullOut = 0;
		BowNet_Result.net_Lines.clear();


		recResult.cross_Point.x = 0;
		recResult.cross_Point.y = 0;
		recResult.pre_cross_Point.x = 0; 
		recResult.pre_cross_Point.y = 0;
		recResult.pre_pixelBowHeight = 0; 
		recResult.pixel_BowHeight = 0;
		recResult.pixel_PullOut = 0;
		recResult.real_BowHeight = 0;
		recResult.real_PullOut = 0;

		str_tt = NULL;
		delete[] para_Path;
		para_Path = NULL;
		return true;
	}

	return false;
}

bool Height_Stagger::GetShowWindowHwnd(UINT input_hwnd)
{
	h_wnd = (HWND)input_hwnd;
	hdc = ::GetDC(h_wnd);
	::GetClientRect(h_wnd,&rect);

	if(!hdc||(rect.bottom - rect.top)==0||(rect.right - rect.left)==0)
	{
		return false;
	}

	//::Rectangle(hdc,0,0,150,150);

	return true;
}

void Height_Stagger::ReRreshImage()
{
	if(!h_wnd)
	{
		return;
	}
	hdc = ::GetDC(h_wnd);
	::GetClientRect(h_wnd,&rect);
	DrawRecognizeResult(image_Para.src_MatRgb,nFrame);
}

bool Height_Stagger::SetImageSize(int nWidth, int nHeight)
{
    	if (nWidth <= 0 || nHeight <= 0)  //图像尺寸错误
	     {
		   return false;
	     }

		image_Para.pre_nHeight = image_Para.cur_nHeight = nHeight;
		image_Para.pre_nWidth = image_Para.cur_nWidth = nWidth;
		
		image_Para.src_MatRgb = cvCreateMat(image_Para.cur_nHeight, image_Para.cur_nWidth, CV_8UC3);

		if(image_Para.cur_nHeight == 240 && image_Para.cur_nWidth == 320)
		{
			image_Para.pre_nHeight = 240;
			image_Para.pre_nWidth = 320;

			image_Para.cur_nHeight = 240;
			image_Para.cur_nWidth = 320;

			if(image_Para.pixelImage)
			{
				free(image_Para.pixelImage);
				image_Para.pixelImage = (uchar*)malloc(sizeof(uchar)*image_Para.cur_nWidth*image_Para.cur_nHeight);
			}	
		}

		else if(image_Para.cur_nHeight == 480 && image_Para.cur_nWidth == 640)
		{
			image_Para.pre_nHeight = 480;
			image_Para.pre_nWidth = 640;

			image_Para.cur_nHeight = 480;
			image_Para.cur_nWidth = 640;

			if(image_Para.pixelImage)
			{
				free(image_Para.pixelImage);
				image_Para.pixelImage = (uchar*)malloc(sizeof(uchar)*image_Para.cur_nWidth*image_Para.cur_nHeight);
			}
		}

		else
		{
			return false; //目前仅支持320或640分辨率的图像，否则直接返回false，不再进行后续处理
		}
		return true;
}


bool Height_Stagger::SetTrainName(LPCTSTR t_name)
{
	wstring wstr = LPCTSTR(t_name);

	unsigned len = wstr.size() * 4;
	if(len == 0)
	{
		trainName = "";	
		return false;
	}
	else
	{
		char *p = new char[len];
		wcstombs(p,wstr.c_str(),len);
		std::string str(p);
		trainName = str;	
		delete[] p;
		LoadCalibrationParameters();
		return true;
	}
	return false;
}

void Height_Stagger::AdjustParameters(int cut_Top,int cut_Height,int cut_Left,int cut_Width)
{
	if(image_Para.cur_nHeight == 480)
	{
		if(cut_Top<0|| cut_Top+cut_Height> 480 || cut_Left<0 || cut_Left+cut_Width > 640)
		{
			return;
		}
	}

	else
	{
		if(cut_Top<0|| cut_Top+cut_Height> 240 || cut_Left<0 || cut_Left+cut_Width > 320)
		{
			return;
		}
	}

	try
	{
		image_Para.nFirstCut_TopEdge = cut_Top;     
		image_Para.nFirstCut_Height = cut_Height;
		image_Para.nFirstCut_LeftEdge = cut_Left;
		image_Para.nFirstCut_Width = cut_Width;

		image_Para.src_Mat = cvCreateMat(image_Para.nFirstCut_Height ,image_Para.nFirstCut_Width,CV_8UC1);  //初始数据直接截掉图像的四个边界后的图像

		image_Para.nSecondCut_TopEdge = 0;
		image_Para.nSecondCut_Height = image_Para.src_Mat.rows;
		image_Para.nSecondCut_LeftEdge = image_Para.src_Mat.cols/6;
		image_Para.nSecondCut_Width = image_Para.src_Mat.cols*2/3;

		image_Para.nThirdCut_LeftEdge = image_Para.src_Mat.cols/8;
		image_Para.nThirdCut_Width = image_Para.src_Mat.cols*3/4;

		image_Para.inPut_PreProcessMat = cvCreateMat(image_Para.src_Mat.rows,image_Para.src_Mat.cols,CV_8UC1);  //预处理过程中用于拷贝src_Mat
		image_Para.input_PreProcessROIMat = cvCreateMat(image_Para.nSecondCut_Height,image_Para.nSecondCut_Width,CV_8UC1); //预处理过程中截取的ROI
		image_Para.input_PreProcessROIBinMat = cvCreateMat(image_Para.input_PreProcessROIMat.rows,image_Para.input_PreProcessROIMat.cols,CV_8UC1);

		image_Para.input_PreProcessROICannyMat = cvCreateMat(image_Para.input_PreProcessROIMat.rows,image_Para.input_PreProcessROIMat.cols,CV_8UC1); //Canny边界图像

		image_Para.inPut_RecognizeMat = cvCreateMat(image_Para.src_Mat.rows,image_Para.src_Mat.cols,CV_8UC1);  //弓网识别过程中用于拷贝src_Mat
		image_Para.inPut_RecognizeROIMat = cvCreateMat(image_Para.inPut_RecognizeMat.rows/3,image_Para.nThirdCut_Width,CV_8UC1);
		image_Para.inPut_RecognizeROIBinMat = cvCreateMat(image_Para.inPut_RecognizeROIMat.rows,image_Para.inPut_RecognizeROIMat.cols,CV_8UC1); //根据定位出的弓位置截取的ROI
	}
	catch(...)
	{
		return;
	}
}

int Height_Stagger::InfraRedImage_ANALYZE(const unsigned char* pGBuffer, UINT iFrame, int*  pullOutValue, int* HeightValue, RECT* bowRegion)
{
	BowNet_Result.midPx =  recResult.midPoint.x;  //将弓的中点值传给内部参数

	recResult.cross_Angles.clear();
	image_Para.ComInclines.clear();
	image_Para.detectedInclines.clear();
	image_Para.dectecedLines.clear();
	BowNet_Result.net_Lines.clear();
	
	nFrame = iFrame;
	

	bowRegion->left = -1;
	bowRegion->right = 0;
	bowRegion->top = -1;
	bowRegion->bottom = 0;

	//recResult.midPoint.x = 188;  //弓的中点在原始图像中的列坐标（相当于是指定弓中点的列坐标）
	//camera_Height = 5300;
	//xRatio = 3.875;
	//yRatio = 4.16667;

	//TopBowHeightScale = camera_Height + yRatio*image_Para.cur_nHeight;  //图像顶端对应的最大导高值
	//BottomBowHeightScale = ((int)((camera_Height + 100) * 10 / 1000)) * 100;  //图像上导高值的最小刻度(取整)
	//MinBHFromImgBottom = (BottomBowHeightScale - camera_Height) / yRatio; //最小导高刻度值距离图像底端的像素距离
	//IntervalNumber = (TopBowHeightScale - BottomBowHeightScale) / 100;  //导高值刻度对应多少段

	//heightInterval = 100 / yRatio; //导高值刻度对应的像素间隔
	//PullOutPixelSpan = 600 / xRatio; //左或右拉出值的最大像素范围
	//pulloutInterval = 100 / xRatio; //拉出值刻度对应的像素间隔

	//RightMaxPullOut = recResult.midPoint.x + (int)(600 / xRatio);  //向右最大拉出像素位置
	//LeftMaxPullOut = recResult.midPoint.x - (int)(600 / xRatio);   //向左最大拉出像素位置
	//AlarmHeightPixel = (int)(480 - ((6600 - camera_Height) / yRatio));  //根据比率关系算出导高报警时对应的像素值


	int Multi_Factor = image_Para.cur_nHeight / 240;


	if(nFrame==1)
	{
		ReRreshImage();
	}
	try
	{
		if(!pGBuffer)
		{
			return NO_PICTURE;
		}
	}
	catch(...)
	{
		return NO_PICTURE;
	}

	for(int row = 0;row< image_Para.cur_nHeight;row++)   //获取单通道的图像数组
	{
		for(int col = 0;col<image_Para.cur_nWidth;col++)
		{
			image_Para.src_MatRgb.at<Vec3b>(row,col)[0] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col];
			image_Para.src_MatRgb.at<Vec3b>(row,col)[1] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col+1];
			image_Para.src_MatRgb.at<Vec3b>(row,col)[2] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col+2];
		}
	}

	for(int row = 0;row< image_Para.cur_nHeight;row++)   //获取单通道的图像数组
	{
		for(int col = 0;col<image_Para.cur_nWidth;col++)
		{
			image_Para.pixelImage[row*image_Para.cur_nWidth+col] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col];
		}
	}

	GrayBufferToMat(image_Para.pixelImage);   //数组转换为Mat图像

	if(!image_Para.src_Mat.data)
	{
		BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = FALSE;
		BowNet_Result.real_BowHeight = BowNet_Result.pixel_BowHeight = 0;
		BowNet_Result.real_PullOut = BowNet_Result.pixel_PullOut = 0;
		return NO_PICTURE;
	}

	else
	{
		try
		{
			if(IsRecByTrainer == false) 
			{

				IsBowRectExist = DetectBowRectRegion(image_Para.src_Mat,image_Para.bow_rects);
				if(loadFlag == false)
				{
					BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = FALSE;
					BowNet_Result.real_BowHeight = BowNet_Result.pixel_BowHeight = 0;
					BowNet_Result.real_PullOut = BowNet_Result.pixel_PullOut = 0;
					pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
					HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
					recResult.pre_pixelBowHeight = 0;
					DrawRecognizeResult(image_Para.src_MatRgb,nFrame);

					return NOT_FOUND_BOW;
				}
				if(IsBowRectExist == false)
				{
					BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = FALSE;
					BowNet_Result.real_BowHeight = BowNet_Result.pixel_BowHeight = 0;
					BowNet_Result.real_PullOut = BowNet_Result.pixel_PullOut = 0;
					pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
					HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
					recResult.pre_pixelBowHeight = 0;
					DrawRecognizeResult(image_Para.src_MatRgb,nFrame);

					return NOT_FOUND_BOW;
				}
			}

			IsRecByTrainer = true; //表示当前帧已用训练器判断已经有弓
			int Pre_Result = Pre_Process(recResult.pixel_BowHeight); //预处理，根据温度亮点先确定弓的位置
			if(Pre_Result == NO_PICTURE)
			{
				BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = FALSE;
				BowNet_Result.real_BowHeight = BowNet_Result.pixel_BowHeight = 0;
				BowNet_Result.real_PullOut = BowNet_Result.pixel_PullOut = 0;
				pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
				HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
				DrawRecognizeResult(image_Para.src_MatRgb,nFrame);
				return NO_PICTURE;

			}
			else if(Pre_Result == NOT_FOUND_BOW)
			{
				BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = FALSE;
				BowNet_Result.real_BowHeight = BowNet_Result.pixel_BowHeight = 0;
				BowNet_Result.real_PullOut = BowNet_Result.pixel_PullOut = 0;
				pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
				HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
				DrawRecognizeResult(image_Para.src_MatRgb,nFrame);

				return NOT_FOUND_BOW;
			}
			else if(Pre_Result == BOW_HEIGHT_ERROR)
			{
				BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = FALSE;
				BowNet_Result.real_BowHeight = BowNet_Result.pixel_BowHeight = 0;
				BowNet_Result.real_PullOut = BowNet_Result.pixel_PullOut = 0;
				pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
				HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
				DrawRecognizeResult(image_Para.src_MatRgb, nFrame);

				return BOW_HEIGHT_ERROR;
			}
			else
			{
				BowNet_Result.Bow_Exist = recResult.IsCur_BowExist = TRUE;
				BowNet_Result.midPy = recResult.pixel_BowHeight + image_Para.nFirstCut_TopEdge;  //都是基于320 * 240的分辨率来计算的
				BowNet_Result.pixel_BowHeight = Multi_Factor*(240 - BowNet_Result.midPy);  //导高(从下往上计算)  都是基于320*240的分辨率来计算的


				//记录弓区域位置，用于返回判断高温点是否在弓范围内
				bowRegion->left = Multi_Factor*(recResult.midPoint.x - 110) < 0 ? 0 : Multi_Factor*(recResult.midPoint.x - 110);
				bowRegion->right = Multi_Factor*(recResult.midPoint.x + 110) > Multi_Factor * 319 ? Multi_Factor * 319 : Multi_Factor*(recResult.midPoint.x + 110);
				bowRegion->top = Multi_Factor*(BowNet_Result.midPy - 10) < 0 ? 0 : Multi_Factor*(BowNet_Result.midPy - 10);
				bowRegion->bottom = bowRegion->top + Multi_Factor*60;


				int Rec_Result = BowNet_Recognize(recResult);
				if(Rec_Result == NOT_FOUND_LINE)
				{
					CalculateSpaceCoordinateByScaleRatio(BowNet_Result.pixel_BowHeight,0,BowNet_Result.real_BowHeight,BowNet_Result.real_PullOut);
					recResult.cross_Point.x = recResult.cross_Point.y = 0;
					pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
					HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
					DrawRecognizeResult(image_Para.src_MatRgb,nFrame);
					return NOT_FOUND_LINE;
				}
				else if(Rec_Result == NOT_FOUND_OBLIQUE)
				{
					CalculateSpaceCoordinateByScaleRatio(BowNet_Result.pixel_BowHeight,0,BowNet_Result.real_BowHeight,BowNet_Result.real_PullOut);
					recResult.cross_Point.x = recResult.cross_Point.y = 0;
					pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
					HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
					DrawRecognizeResult(image_Para.src_MatRgb,nFrame);

					return NOT_FOUND_OBLIQUE;
				}
				else if(Rec_Result == NOT_FOUND_CROSSPOINT)
				{
					CalculateSpaceCoordinateByScaleRatio(BowNet_Result.pixel_BowHeight*2,0,BowNet_Result.real_BowHeight,BowNet_Result.real_PullOut);
					recResult.cross_Point.x = recResult.cross_Point.y = 0;
					pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
					HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
					DrawRecognizeResult(image_Para.src_MatRgb,nFrame);

					return NOT_FOUND_CROSSPOINT;
				}
				else
				{
					recResult.cross_Point = SelectCrossPointByDistance(recResult);

					BowNet_Result.net_Lines.clear();
					for(size_t t= 0; t<recResult.cross_Angles.size();t++)    //保存合并后所有的接触网线条
					{	
						NET_LINE temp_Line;
						temp_Line.x1 = (recResult.cross_Angles.at(t).line2.pt1.x + image_Para.cut_LeftCols + image_Para.nFirstCut_LeftEdge);
						temp_Line.y1 = (recResult.cross_Angles.at(t).line2.pt1.y + image_Para.cut_UpRows + image_Para.nFirstCut_TopEdge);

						temp_Line.x2 = (recResult.cross_Angles.at(t).line2.pt2.x + image_Para.cut_LeftCols + image_Para.nFirstCut_LeftEdge);
						temp_Line.y2 = (recResult.cross_Angles.at(t).line2.pt2.y + image_Para.cut_UpRows + image_Para.nFirstCut_TopEdge);

						temp_Line.k = recResult.cross_Angles.at(t).line2.k;

						if(temp_Line.k == image_Para.vlK)
						{
							temp_Line.b = 100000;
						}

						else
						{
							temp_Line.b = temp_Line.y1 - temp_Line.k*temp_Line.x1;
						}
						
						temp_Line.number = recResult.cross_Angles.at(t).line2.number;
						temp_Line.groupNumber = recResult.cross_Angles.at(t).line2.groupNumber;

						BowNet_Result.net_Lines.push_back(temp_Line);
					}

					BowNet_Result.crossPx = recResult.cross_Point.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge;
					BowNet_Result.crossPy = BowNet_Result.midPy ; 
					recResult.pixel_PullOut = BowNet_Result.crossPx - BowNet_Result.midPx;   //320分辨率图像中像素拉出值

					BowNet_Result.pixel_PullOut = Multi_Factor*recResult.pixel_PullOut;

					CalculateSpaceCoordinateByScaleRatio(BowNet_Result.pixel_BowHeight,BowNet_Result.pixel_PullOut,BowNet_Result.real_BowHeight,BowNet_Result.real_PullOut);
					pullOutValue[iFrame-1] = (int)BowNet_Result.real_PullOut;
					HeightValue[iFrame - 1] = (int)BowNet_Result.real_BowHeight;
					DrawRecognizeResult(image_Para.src_MatRgb,nFrame);

					

					return 1;
				}
			}
		}
		catch(...)
		{
			return 0;
		}
	}
	return 0;
}

int Height_Stagger::GrayBufferToMat(const unsigned char* pArr)   //将buffer数组转为Mat矩阵图像
{
	if(!pArr)
	{
		return NO_PICTURE;
	}
	Mat src(image_Para.cur_nHeight, image_Para.cur_nWidth, CV_8UC1);
	Mat srcNorm(240, 320, CV_8UC1);  //所有的图像同一标准化为320再进行处理

	try
	{
		for (int row = 0; row< image_Para.cur_nHeight; row++)
		{
			for (int col = 0; col<image_Para.cur_nWidth; col++)
			{
				src.at<uchar>(row, col) = *(pArr + row* image_Para.cur_nWidth + col);
			}
		}
		resize(src, srcNorm, Size(srcNorm.cols, srcNorm.rows)); //将图像缩放为320


		for (int row = 0; row< image_Para.src_Mat.rows; row++)
		{
			for (int col = 0; col<image_Para.src_Mat.cols; col++)
			{
				image_Para.src_Mat.at<uchar>(row, col) = srcNorm.at<uchar>(row + image_Para.nFirstCut_TopEdge, col + image_Para.nFirstCut_LeftEdge);
			}
		}

		src.release();
		srcNorm.release();
	}
	catch (...)
	{
		src.release();
		srcNorm.release();
		return NO_PICTURE;
	}
	return 1;
}

int Height_Stagger::Pre_Process(int& bow_H)   //图像的预处理函数，确定受电弓的位置
{
	bow_H = 0;   //每帧处理前导高值初始化为0;
	try
	{
		if(!image_Para.src_Mat.data)
		{
			return NO_PICTURE;
		}
		image_Para.src_Mat.copyTo(image_Para.inPut_PreProcessMat);
		if(!image_Para.inPut_PreProcessMat.data)
		{
			return NO_PICTURE;
		}
		image_Para.input_PreProcessROIMat = image_Para.inPut_PreProcessMat(Range(image_Para.nSecondCut_TopEdge,image_Para.nSecondCut_TopEdge+image_Para.nSecondCut_Height),Range(image_Para.nSecondCut_LeftEdge,image_Para.nSecondCut_LeftEdge+image_Para.nSecondCut_Width));  //截取图像的中间列部分区域，用于找到弓的位置
		
		if(!image_Para.input_PreProcessROIMat.data)
		{
			return NO_PICTURE;
		}

		int BowHeight = ConfirmBowPosition(bow_H);   //确定弓的位置
		
		if(BowHeight == NOT_FOUND_BOW) 
		{	
			if(recResult.pre_pixelBowHeight == 0)
			{
				recResult.IsPre_BowExist = FALSE;
				recResult.pre_pixelBowHeight = bow_H = 0;
				nContinueNoBow++;
				IsRecByTrainer = false;
				return NOT_FOUND_BOW;
			}
			else
			{
				recResult.IsPre_BowExist = FALSE;
				nContinueNoBow++;

				if(nContinueNoBow>=3)
				{
					recResult.pre_pixelBowHeight = bow_H = 0;
					IsRecByTrainer = false;
					return NOT_FOUND_BOW;
				}
				else
				{
					if(recResult.pre_pixelBowHeight != 0)
					{
						bow_H = recResult.pre_pixelBowHeight;	 //未找到受电弓时且不是连续3帧无弓利用前一帧的导高值进行矫正
					}
					else
					{
						recResult.pre_pixelBowHeight = bow_H = 0;
						IsRecByTrainer = false;
						return NOT_FOUND_BOW;
					}			
				}
				return 1;
			}
			return NOT_FOUND_BOW;
		}

		else if(BowHeight == BOW_HEIGHT_ERROR)  //表示找到的弓位置过高或过低，位置错误，也即未找到弓，相当于是存在干扰
		{
			if(recResult.pre_pixelBowHeight == 0)
			{
				recResult.IsPre_BowExist = FALSE;
				recResult.pre_pixelBowHeight = bow_H = 0;
				nContinueNoBow++;
				IsRecByTrainer = false;
				return BOW_HEIGHT_ERROR;
			}
			else
			{
				recResult.IsPre_BowExist = FALSE;
				nContinueNoBow++;
				if(nContinueNoBow>=3)   //连续有3帧无弓
				{
					recResult.pre_pixelBowHeight = bow_H = 0;
					IsRecByTrainer = false;
					return BOW_HEIGHT_ERROR;
				}
				else
				{
					if(recResult.pre_pixelBowHeight != 0)
					{
						bow_H = recResult.pre_pixelBowHeight;  //未找到受电弓且连续3帧无弓时利用前一帧的导高值进行修正
					}				
					else
					{
						recResult.pre_pixelBowHeight = bow_H = 0;
						IsRecByTrainer = false;
						return BOW_HEIGHT_ERROR;
					}
				}
				return 1;
			}
			return BOW_HEIGHT_ERROR;
		}

		else
		{
			recResult.IsPre_BowExist = TRUE;
			nContinueNoBow = 0;

			if(recResult.pre_pixelBowHeight == 0)
			{
				recResult.pre_pixelBowHeight = bow_H;
			}

			else
			{
				if(abs(bow_H - recResult.pre_pixelBowHeight)>=10)  //如果当前帧的导高与上一帧的差距大于10，则修正该导高值,相当于默认前后两帧导高值在10个像素以内属于正常范围
				{
					nContinueBigInterval++;
					if(nContinueBigInterval>=5)
					{
						recResult.pre_pixelBowHeight = bow_H = 0;
						IsRecByTrainer = false;		
					}
					else
					{
						bow_H = recResult.pre_pixelBowHeight;   //若不是连续5对相邻帧之间的间隔较大，则用上一帧的导高值进行矫正
					}			
				}
				else
				{
					nContinueBigInterval = 0;  //若当前帧与上一帧间隔未超过10个像素，则将其归0 
					recResult.pre_pixelBowHeight = bow_H;   
				}
		   }
		}
	}
	catch (...)
	{
		return NOT_FOUND_BOW;
	}

	return 1;
}

int Height_Stagger::ConfirmBowPosition(int& HL)   //定位出弓的位置
{
	int maxNum = 0;  //最多白点行的255的个数	

	try
	{
		//先统计灰度值大于205的点数
		int countPixel = 0;
		for (int row = 0; row < image_Para.input_PreProcessROIMat.rows; row+=2)    //相当于缩小为原有的1/4
		{
			for (int col = 0; col < image_Para.input_PreProcessROIMat.cols; col+=2)
			{
				if (image_Para.input_PreProcessROIMat.at<uchar>(row, col) >= 205)
				{
					countPixel++;
				}

			}
		}	
		int totalNumber = (int)((image_Para.input_PreProcessROIMat.rows * image_Para.input_PreProcessROIMat.cols) / 4);

		if (countPixel >= totalNumber / 10)  //如果大于205的点数占比超过1/10
		{
			/****************将小于205的像素点全置为0**************/
			for (int row = 0; row< image_Para.input_PreProcessROIMat.rows; row++)
			{
				for (int col = 0; col<image_Para.input_PreProcessROIMat.cols; col++)
				{
					if (image_Para.input_PreProcessROIMat.at<uchar>(row, col)<205)
					{
						image_Para.input_PreProcessROIBinMat.at<uchar>(row, col) = 0;
					}
					else
					{
						image_Para.input_PreProcessROIBinMat.at<uchar>(row, col) = 255;
					}
				}
			}
		}

		else
		{
			threshold(image_Para.input_PreProcessROIMat, image_Para.input_PreProcessROIBinMat, 0, 255, CV_THRESH_OTSU);   //added by wf at 2019-04-25
		}
	
		for(int k =0;k<image_Para.input_PreProcessROIBinMat.rows;k++)
		{
			image_Para.pCountHArray[k] = 0;
		}

		for(int i = 0;i<image_Para.input_PreProcessROIBinMat.rows;i++)   //统计每一行的255点数
		{
			for(int j = 0;j<image_Para.input_PreProcessROIBinMat.cols;j++)
			{
				if(image_Para.input_PreProcessROIBinMat.at<uchar>(i,j)==255)
				{
					image_Para.pCountHArray[i]++;
				}
			}
		}

		for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)   //左边界10列的255点数
		{
			int L_edge_num = 0;	
			for(int col = 0;col< 20;col++)
			{
				if(row == 0)
				{
					if(image_Para.input_PreProcessROIBinMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row+1,col) ==255)
					{
						L_edge_num++;
					}

				}

				else if(row == image_Para.input_PreProcessROIBinMat.rows -1)
				{
					if(image_Para.input_PreProcessROIBinMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row-1,col) ==255)
					{
						L_edge_num++;
					}
				}
				else
				{
					if(image_Para.input_PreProcessROIBinMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row-1,col) ==255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row+1,col) ==255)
					{
						L_edge_num++;
					}
				}
			}

			if(L_edge_num>10)    //将边界有较多白点的行数组值置为0，相当于去除横杆的影响
			{
				image_Para.pCountHArray[row] = 0;
			}
		}

		for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)   //右边界20列的255点数
		{
			int R_edge_num = 0;
			for(int col = image_Para.input_PreProcessROIBinMat.cols-20;col<image_Para.input_PreProcessROIBinMat.cols;col++)
			{
				if(row == 0)
				{
					if(image_Para.input_PreProcessROIBinMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row+1,col) ==255)
					{
						R_edge_num++;
					}
				}

				else if(row == image_Para.input_PreProcessROIBinMat.rows -1)
				{
					if(image_Para.input_PreProcessROIBinMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row-1,col) ==255)
					{
						R_edge_num++;
					}

				}
				else
				{
					if(image_Para.input_PreProcessROIBinMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row-1,col) ==255 || image_Para.input_PreProcessROIBinMat.at<uchar>(row+1,col) ==255)
					{
						R_edge_num++;
					}
				}
			}
			if(R_edge_num>10)       
			{
				image_Para.pCountHArray[row] = 0;
			}
		}

		for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)   //找出255点数最多的行
		{		
			if(image_Para.pCountHArray[row]>maxNum)
			{
				maxNum = image_Para.pCountHArray[row];
			}
		}

		if(maxNum<image_Para.input_PreProcessROIBinMat.cols/6)   //若最大行的255点数小于列像素的1/6,则表示当前帧图像无弓
		{
			HL = -1;
		}

		else
		{
			for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)    /****************根据白点个数计算弓的位置***************/
			{		
				if(image_Para.pCountHArray[row]>maxNum/2)   //从上往下找到白点较多的行,大于MaxNum的1/2
				{
					HL = row;
					int nWhiteCount = 0;       //统计每行中间区域的白点个数
					for(int i_col = image_Para.input_PreProcessROIBinMat.cols/3;i_col< 2*image_Para.input_PreProcessROIBinMat.cols/3; i_col++)
					{
						if(image_Para.input_PreProcessROIBinMat.at<uchar>(HL,i_col)==255)
						{
							nWhiteCount++;
						}
					}
					if(nWhiteCount<=maxNum/5)  //判断该行的中间区域高温点(即255)的个数（若小于maxNum的五分之一，则剔除）
					{
						HL = -1;   //若当前行不满足条件，则设定HL为-1
						continue;
					}
					else
					{
						break;
					}
				}
			}
		}
		
		if(HL==-1 || HL<image_Para.input_PreProcessROIBinMat.rows/10 || HL>image_Para.input_PreProcessROIBinMat.rows*9/10)
		{
			Canny(image_Para.input_PreProcessROIMat,image_Para.input_PreProcessROICannyMat,130,150,3);   //Canny算子突显轮廓(相当于进行二值化)

			for(int k =0;k<image_Para.input_PreProcessROICannyMat.rows;k++)
			{
				image_Para.pCountHArray[k] = 0;
			}

			for(int i = 0;i<image_Para.input_PreProcessROICannyMat.rows;i++)    //统计每一行的值为255的点数
			{
				for(int j = 0;j<image_Para.input_PreProcessROICannyMat.cols;j++)
				{
					if(image_Para.input_PreProcessROICannyMat.at<uchar>(i,j)==255)
					{
						image_Para.pCountHArray[i]++;
					}
				}
			}


			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   //左边界20列的255点数
			{
				int L_edge_num = 0;	
				for(int col = 0;col< 20;col++)
				{
					if(row == 0)
					{
						if(image_Para.input_PreProcessROICannyMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row+1,col) ==255)
						{
							L_edge_num++;
						}

					}

					else if(row == image_Para.input_PreProcessROICannyMat.rows -1)
					{
						if(image_Para.input_PreProcessROICannyMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row-1,col) ==255)
						{
							L_edge_num++;
						}

					}
					else
					{
						if(image_Para.input_PreProcessROICannyMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row-1,col) ==255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row+1,col) ==255)
						{
							L_edge_num++;
						}
					}
				}
				if(L_edge_num>15)    //将边界有较多白点的行数组值置为0，相当于去除横杆的影响
				{
					image_Para.pCountHArray[row] = 0;
				}
			}

			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   //右边界20列的255点数
			{
				int R_edge_num = 0;
				for(int col = image_Para.input_PreProcessROICannyMat.cols-20;col<image_Para.input_PreProcessROICannyMat.cols;col++)
				{
					if(row == 0)
					{
						if(image_Para.input_PreProcessROICannyMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row+1,col) ==255)
						{
							R_edge_num++;
						}
					}

					else if(row == image_Para.input_PreProcessROICannyMat.rows -1)
					{
						if(image_Para.input_PreProcessROICannyMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row-1,col) ==255)
						{
							R_edge_num++;
						}
					}
					else
					{
						if(image_Para.input_PreProcessROICannyMat.at<uchar>(row,col) == 255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row-1,col) ==255 || image_Para.input_PreProcessROICannyMat.at<uchar>(row+1,col) ==255)
						{
							R_edge_num++;
						}
					}
				}
				if(R_edge_num>15)       
				{
					image_Para.pCountHArray[row] = 0;
				}
			}

			maxNum = 0;
			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   //找出255点数最多的行
			{		
				if(image_Para.pCountHArray[row]>maxNum)
				{
					maxNum = image_Para.pCountHArray[row];
				}
			}

			if(maxNum<image_Para.input_PreProcessROICannyMat.cols/6)    
			{
				return NOT_FOUND_BOW;
			}

			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   
			{		
				if(image_Para.pCountHArray[row]>maxNum/2)   //从上往下找到白点较多的行,大于MaxNum的1/2,即为受电弓的位置
				{
					HL = row;
					break;
				}
			}

			if(HL == -1)
			{
				return NOT_FOUND_BOW;
			}
			else if(HL<image_Para.input_PreProcessROIBinMat.rows/10 || HL>image_Para.input_PreProcessROIBinMat.rows*9/10)
			{
				return BOW_HEIGHT_ERROR;
			}
			else
			{
				return 1;
			}
		}
		else 
		{
			return 1;
		}
	}
	catch(...)
	{
		return  NOT_FOUND_BOW;
	}

	return 1;
}

int Height_Stagger::GetMaxNumberValue(Mat InputMat)   //获得像素值个数最多的像素点
{
	if(!InputMat.data)
	{
		return 0;
	}
	try
	{
		memset(image_Para.pHist,0,sizeof(int)*256); //将数组元素都置为0
		for(int i = 0;i<InputMat.rows;i++)
		{
			for(int j = 0;j<InputMat.cols; j++)
			{
				image_Para.pHist[InputMat.at<uchar>(i,j)]++;
			}
		}
		int nMax_Number = image_Para.pHist[0];
		int nMaxNumber_Pixel = 0;
		for(int j = 0;j<256;j++)
		{
			if(image_Para.pHist[j] > nMax_Number)
			{
				nMax_Number = image_Para.pHist[j];
				nMaxNumber_Pixel = j;
			}
		}	
		return nMaxNumber_Pixel ;
	}
	catch(...)
	{
		return 0;
	}
}

int Height_Stagger::GetMaxPixelValue(Mat InputMat)  //获得图像的最大像素值
{
	if(!InputMat.data)
	{
		return 0;
	}
	try
	{
		int MaxValue_Pixel = InputMat.at<uchar>(0,0);
		for(int i = 0;i<InputMat.rows;i++)
		{
			for(int j = 0;j<InputMat.cols; j++)
			{
				if(InputMat.at<uchar>(i,j) > MaxValue_Pixel)
				{
					MaxValue_Pixel = InputMat.at<uchar>(i,j);
				}
			}
		}
		return MaxValue_Pixel;
	}
	catch(...)
	{
		return 0;
	}

}

int Height_Stagger::GetMinPixelValue(Mat InputMat)  //获得图像的最小像素值
{
	if(!InputMat.data)
	{
		return 0;
	}
	try
	{
		int MinValue_Pixel = InputMat.at<uchar>(0,0);
		for(int i = 0;i<InputMat.rows;i++)
		{
			for(int j = 0;j<InputMat.cols; j++)
			{
				if(InputMat.at<uchar>(i,j) < MinValue_Pixel)
				{
					MinValue_Pixel = InputMat.at<uchar>(i,j);
				}
			}
		}
		return MinValue_Pixel;
	}
	catch(...)
	{
		return 0;
	}

}

int Height_Stagger::BowNet_Recognize(RecResult& ret)   //接触点的识别
{
	BOOL Is_Canny = FALSE; //判断是否利用了Canny算子进行二值化

	image_Para.Is_LineSatisfied  = FALSE;  
	image_Para.dectecedLines.clear(); 
	image_Para.ComInclines.clear();

	try
	{
		if(!image_Para.src_Mat.data)
		{
			return NOT_FOUND_LINE;
		}
		image_Para.src_Mat.copyTo(image_Para.inPut_RecognizeMat);
		if(!image_Para.inPut_RecognizeMat.data)
		{
			return NOT_FOUND_LINE;
		}
		/*******************截取受电弓上方周围的ROI区域*******************/

		image_Para.inPut_RecognizeROIMat = image_Para.inPut_RecognizeMat(Range(0,ret.pixel_BowHeight),Range(image_Para.nThirdCut_LeftEdge,image_Para.nThirdCut_LeftEdge+image_Para.nThirdCut_Width)); //截取image_Para.inPut_RecognizeMat.rows/3行，nThirdCut_Width列的区域
		image_Para.cut_UpRows = 0;
		image_Para.cut_LeftCols = image_Para.nThirdCut_LeftEdge;
	
		if(!image_Para.inPut_RecognizeROIMat.data)   //判断是否成功截取ROI
		{
			return NOT_FOUND_LINE;
		}
		image_Para.inPut_RecognizeROIMat.copyTo(image_Para.inPut_RecognizeROIBinMat); 
		if(!image_Para.inPut_RecognizeROIBinMat.data)
		{
			return NOT_FOUND_LINE;
		}
	}
	catch (...)
	{
		return NOT_FOUND_LINE;

	}
	//imshow("弓上方区域",image_Para.inPut_RecognizeROIMat);
	//waitKey(0);

	try
	{
		image_Para.MaxCount_Pixel = GetMaxNumberValue(image_Para.inPut_RecognizeROIMat);  //计算个数最多的像素值
		image_Para.MaxValue_Pixel = GetMaxPixelValue(image_Para.inPut_RecognizeROIMat);  //计算最大像素值

		if((image_Para.MaxCount_Pixel<30 && image_Para.MaxValue_Pixel<100) || (image_Para.MaxCount_Pixel>=175 && image_Para.MaxValue_Pixel>=200))   //图像整体过暗或过亮,指定阈值进行二值化处理
		{
			for(int row = 0;row< image_Para.inPut_RecognizeROIBinMat.rows;row++)
			{
				for(int col = 0;col<image_Para.inPut_RecognizeROIBinMat.cols;col++)
				{
					if(image_Para.inPut_RecognizeROIMat.at<uchar>(row,col)<image_Para.MaxCount_Pixel+5)
					{
						image_Para.inPut_RecognizeROIBinMat.at<uchar>(row,col) = 0;
					}
					else
					{
						image_Para.inPut_RecognizeROIBinMat.at<uchar>(row,col) = 255;
					}
				}
			}
		}
		else if(((image_Para.MaxValue_Pixel-image_Para.MaxCount_Pixel)<50))  //若最大像素值与最多像素值相差小于50，则利用大津法进行二值化
		{
			GaussianBlur(image_Para.inPut_RecognizeROIMat,image_Para.gauss_Mat,Size(3,3),2,2);
			threshold(image_Para.gauss_Mat,image_Para.inPut_RecognizeROIBinMat, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);  //利用大津法(最大类间方差)二值化
		}
		else
		{
			GaussianBlur(image_Para.inPut_RecognizeROIMat,image_Para.gauss_Mat,Size(3,3),2,2);
			Canny(image_Para.gauss_Mat,image_Para.inPut_RecognizeROIBinMat,150,180,3); //Canny算子突显轮廓(也即进行二值化)
			Is_Canny = TRUE;  //上一步进行了Canny操作，记录为TRUE
		}

		if(Is_Canny == FALSE)
		{
			Canny(image_Para.inPut_RecognizeROIBinMat,image_Para.inPut_RecognizeROIBinMat,150,180,3); //Canny算子突显轮廓
		}

		if(!EdgeToBlack(image_Para.inPut_RecognizeROIBinMat,2,2))  //将边界都置黑
		{
		  return NOT_FOUND_CROSSPOINT;
		}

		//imshow("弓上方区域binary",image_Para.inPut_RecognizeROIBinMat);
		//waitKey(30);

		HoughLinesP(image_Para.inPut_RecognizeROIBinMat, image_Para.dectecedLines,1,CV_PI/180,image_Para.inPut_RecognizeROIBinMat.rows*2/5,image_Para.inPut_RecognizeROIBinMat.rows*2/5,image_Para.inPut_RecognizeROIBinMat.rows/3);  //hough直线检测
		if(image_Para.dectecedLines.size()==0)
		{
			return NOT_FOUND_LINE;
		}
		else 
		{
			CombineOblique();  //直线合并
			if(image_Para.ComInclines.size()>0)
			{
				image_Para.Is_LineSatisfied = TRUE;
			}
			else
			{
				return NOT_FOUND_OBLIQUE;
			}
		}
		if(image_Para.Is_LineSatisfied == TRUE)
		{
			ret.cross_Angles.clear();
			Detect_Angles(ret.pixel_BowHeight,ret.cross_Angles);  //夹角提取
			if(ret.cross_Angles.size()==0)
			{
				return NOT_FOUND_CROSSPOINT;
			}
		}
		else
		{
			return NOT_FOUND_CROSSPOINT;
		}

	}
	catch(...)
	{
		return NOT_FOUND_CROSSPOINT;
	}

	return 1;
}

vector<BOW_NET_LINE> Height_Stagger::CombineOblique()
{
	bool bFlagI = false;
	int groupNumber = 0;  //直线所属组号
	image_Para.lineteamI.number = 1; //默认每条直线都作为一组，每组仅有一条直线
	image_Para.lineteamI.maxD = 0.0; //组内直线初始化的最大间距为0

	image_Para.detectedInclines.clear();
	image_Para.ComInclines.clear();

	try
	{
		for(size_t i = 0; i < image_Para.dectecedLines.size(); i++)
		{
			//选取垂直线条
			if(image_Para.dectecedLines[i][2] - image_Para.dectecedLines[i][0] == 0)  //直线i的两个端点的横坐标（即所在的列），若相同，表明该直线是一条垂线
			{
				image_Para.ptI1.x = image_Para.dectecedLines[i][0];
				image_Para.ptI1.y = image_Para.dectecedLines[i][1];
				image_Para.ptI2.x = image_Para.dectecedLines[i][2];
				image_Para.ptI2.y = image_Para.dectecedLines[i][3];

				image_Para.temp_Incline.pt1 = image_Para.ptI1;
				image_Para.temp_Incline.pt2 = image_Para.ptI2;
				image_Para.temp_Incline.k = image_Para.vlK; 
				image_Para.temp_Incline.b = image_Para.ptI1.x ; //将垂直线的截距定义为其横坐标
				image_Para.detectedInclines.push_back(image_Para.temp_Incline);
				bFlagI = true;
				continue;
			}

			//计算斜率
			double k = (double)(image_Para.dectecedLines[i][3] - image_Para.dectecedLines[i][1]) / (image_Para.dectecedLines[i][2] - image_Para.dectecedLines[i][0]); //纵坐标之差比上横坐标之差，即为直线的斜率
			//选取倾斜线条
			if((abs(k)>0.5)&&(image_Para.dectecedLines[i][2]>20) && (image_Para.dectecedLines[i][0]>20))
			{		
				image_Para.ptI1.x = image_Para.dectecedLines[i][0];
				image_Para.ptI1.y = image_Para.dectecedLines[i][1];
				image_Para.ptI2.x = image_Para.dectecedLines[i][2];
				image_Para.ptI2.y = image_Para.dectecedLines[i][3];

				image_Para.temp_Incline.pt1 = image_Para.ptI1;
				image_Para.temp_Incline.pt2 = image_Para.ptI2;
				image_Para.temp_Incline.k = k;
				image_Para.temp_Incline.b = image_Para.ptI2.y - k * image_Para.ptI2.x;
				image_Para.detectedInclines.push_back(image_Para.temp_Incline);
				bFlagI = true;
			}
		}


		/*****************将每条直线的两个端点的纵坐标都放置在同一个高度***************/
		for(size_t t = 0; t< image_Para.detectedInclines.size();t++)   
		{
			image_Para.detectedInclines.at(t).pt1.y = recResult.pixel_BowHeight-image_Para.cut_UpRows;
			image_Para.detectedInclines.at(t).pt2.y = (recResult.pixel_BowHeight-image_Para.cut_UpRows)/3;

			if(image_Para.detectedInclines.at(t).k == image_Para.vlK)
			{
				continue;
			}

			image_Para.detectedInclines.at(t).pt1.x = (image_Para.detectedInclines.at(t).pt1.y - image_Para.detectedInclines.at(t).b)/image_Para.detectedInclines.at(t).k;
			image_Para.detectedInclines.at(t).pt2.x = (image_Para.detectedInclines.at(t).pt2.y - image_Para.detectedInclines.at(t).b)/image_Para.detectedInclines.at(t).k;
		}

		////合并倾斜的直线（每根接触网有宽度，所以一般会有左右两个边界，主要是为了将两个边界合并）
		/**********
		Method1:根据直线的斜率和截距进行合并
		**********************/
		//for(size_t i = 0; i < image_Para.detectedInclines.size(); i++)
		//{
		//	int iFlag = 0;
		//	double deltaB = 0.0,deltaK = 0.0;
		//	for (size_t j = 0; j < image_Para.ComInclines.size(); j++)
		//	{
		//		deltaK = abs(image_Para.detectedInclines.at(i).k - image_Para.ComInclines.at(j).k);
		//		deltaB = abs(image_Para.detectedInclines.at(i).b - image_Para.ComInclines.at(j).b); 
		//		if (deltaK <= 0.35 && deltaB <= 80)  //合并斜率差距在0.35范围内，截距在80个像素以内的斜向线条
		//		{
		//			image_Para.ComInclines.at(j).k = (image_Para.ComInclines.at(j).number * image_Para.ComInclines.at(j).k + image_Para.detectedInclines.at(i).k) / (image_Para.ComInclines.at(j).number + 1);
		//			image_Para.ComInclines.at(j).b = (image_Para.ComInclines.at(j).number * image_Para.ComInclines.at(j).b + image_Para.detectedInclines.at(i).b) / (image_Para.ComInclines.at(j).number + 1);
		//			image_Para.ComInclines.at(j).number++;

		//			if(image_Para.ComInclines.at(j).k == image_Para.vlK)
		//			{
		//				image_Para.ComInclines.at(j).pt1.x = image_Para.ComInclines.at(j).pt2.x = (int)image_Para.ComInclines.at(j).b;
		//			}
		//			else
		//			{
		//				if (image_Para.ComInclines.at(j).pt2.x < image_Para.detectedInclines.at(i).pt2.x)
		//				{
		//					image_Para.ComInclines.at(j).pt2.x = image_Para.detectedInclines.at(i).pt2.x;
		//				}
		//				if (image_Para.ComInclines.at(j).pt1.x > image_Para.detectedInclines.at(i).pt1.x)
		//				{
		//					image_Para.ComInclines.at(j).pt1.x = image_Para.detectedInclines.at(i).pt1.x;
		//				}
		//			}
		//			image_Para.detectedInclines.at(i).groupNumber = (int)j;  //表示第i条直线归为第j组
		//			iFlag = 1;
		//			break;
		//		}
		//	}
		//	if (iFlag == 1)
		//	{
		//		continue;
		//	}
		//	else
		//	{
		//		image_Para.lineteamI.k = image_Para.detectedInclines.at(i).k;
		//		image_Para.lineteamI.b = image_Para.detectedInclines.at(i).b;
		//		image_Para.lineteamI.pt1.x = image_Para.detectedInclines.at(i).pt1.x;
		//		image_Para.lineteamI.pt2.x = image_Para.detectedInclines.at(i).pt2.x;
		//		image_Para.lineteamI.groupNumber = image_Para.detectedInclines.at(i).groupNumber = groupNumber++;
		//		image_Para.ComInclines.push_back(image_Para.lineteamI);
		//	}
		//}

		//for(size_t i = 0; i < image_Para.ComInclines.size(); i++)
		//{
		//	if(image_Para.ComInclines.at(i).k != image_Para.vlK)
		//	{
		//		image_Para.ComInclines.at(i).pt1.y = static_cast<int>(image_Para.ComInclines.at(i).k * image_Para.ComInclines.at(i).pt1.x + image_Para.ComInclines.at(i).b);
		//		image_Para.ComInclines.at(i).pt2.y = static_cast<int>(image_Para.ComInclines.at(i).k * image_Para.ComInclines.at(i).pt2.x + image_Para.ComInclines.at(i).b);
		//	}
		//	else
		//	{
		//		image_Para.ComInclines.at(i).pt1.y = image_Para.inPut_RecognizeROIBinMat.rows/2;
		//		image_Para.ComInclines.at(i).pt2.y = image_Para.inPut_RecognizeROIBinMat.rows/2;
		//	}
		//}


		/**
		Method2:根据直线两端点的横坐标之差的距离进行直线合并
		****************/
		for(size_t i = 0; i < image_Para.detectedInclines.size(); i++)
		{
			int iFlag = 0;
		    int delta_LX = 0,delta_RX = 0;    //定义两条斜线的两组端点的横坐标之差的绝对值
			for (size_t j = 0; j < image_Para.ComInclines.size(); j++)
			{
				delta_LX = abs(image_Para.detectedInclines.at(i).pt1.x - image_Para.ComInclines.at(j).pt1.x);
				delta_RX = abs(image_Para.detectedInclines.at(i).pt2.x - image_Para.ComInclines.at(j).pt2.x);

				if (delta_LX <= 3 && delta_LX <= 3 && abs(image_Para.detectedInclines.at(i).k - image_Para.detectedInclines.at(j).k)<2)   //两个端点的横坐标之差小于5
				{
					double mid_LY = (double) (image_Para.detectedInclines.at(i).pt1.y +  image_Para.ComInclines.at(j).pt1.y) / 2;  //两个左端点的中点纵坐标
					double mid_LX = (double) (image_Para.detectedInclines.at(i).pt1.x +  image_Para.ComInclines.at(j).pt1.x) / 2;  //两个左端点的中点横坐标
					double mid_RY=  (double) (image_Para.detectedInclines.at(i).pt2.y +  image_Para.ComInclines.at(j).pt2.y) / 2;  //两个右端点的中点纵坐标
					double mid_RX = (double) (image_Para.detectedInclines.at(i).pt2.x +  image_Para.ComInclines.at(j).pt2.x) / 2;  //两个左端点的中点横坐标

					if(mid_RX == mid_LX)
					{
						image_Para.ComInclines.at(j).k = image_Para.vlK;
						image_Para.ComInclines.at(j).b = 100000;		
					}
					else
					{
						image_Para.ComInclines.at(j).k = (mid_RY - mid_LY) /(mid_RX - mid_LX);
						image_Para.ComInclines.at(j).b = mid_RY - image_Para.ComInclines.at(j).k*mid_RX;
					}
					
					image_Para.ComInclines.at(j).pt1.x = mid_LX;
					image_Para.ComInclines.at(j).pt1.y = mid_LY;

					image_Para.ComInclines.at(j).pt2.x = mid_RX;
					image_Para.ComInclines.at(j).pt2.y = mid_RY;

					image_Para.ComInclines.at(j).number++;
					image_Para.detectedInclines.at(i).groupNumber = (int)j;  //表示第i条直线归为第j组
					iFlag = 1;
					break;
				}
			}
			if (iFlag == 1)
			{
				continue;
			}
			else   //没有相近直线时,就单独作为一组直线
			{
				image_Para.lineteamI.k = image_Para.detectedInclines.at(i).k;
				image_Para.lineteamI.b = image_Para.detectedInclines.at(i).b;
				image_Para.lineteamI.pt1.x = image_Para.detectedInclines.at(i).pt1.x;
				image_Para.lineteamI.pt1.y = image_Para.detectedInclines.at(i).pt1.y;

				image_Para.lineteamI.pt2.x = image_Para.detectedInclines.at(i).pt2.x;
				image_Para.lineteamI.pt2.y = image_Para.detectedInclines.at(i).pt2.y;

				image_Para.lineteamI.groupNumber = image_Para.detectedInclines.at(i).groupNumber = groupNumber++;
				image_Para.ComInclines.push_back(image_Para.lineteamI);
			}
		}

	}
	catch(...)
	{
		return image_Para.ComInclines;
	}
	
	return image_Para.ComInclines;
}


vector<BOW_NET_ANGLE> Height_Stagger::Detect_Angles(int bow_row,vector<BOW_NET_ANGLE>& angles)  //检测倾斜线条与弓的夹角
{
	double xDeltaH = -100;
	double yDeltaH = 0;
	static BOW_NET_ANGLE temp_Angle;

	angles.clear();
	temp_Angle.line1.k = 0;
	temp_Angle.line1.b = bow_row-image_Para.cut_UpRows;
	temp_Angle.line1.pt1 = Point(0,bow_row-image_Para.cut_UpRows);
	temp_Angle.line1.pt2 = Point(100,bow_row-image_Para.cut_UpRows);

	try
	{
		for (size_t j = 0; j < image_Para.ComInclines.size(); j++)
		{
			double ik = image_Para.ComInclines.at(j).k;  //记录倾斜直线的斜率
			double ib = image_Para.ComInclines.at(j).b;  //记录倾斜直线的截距

			if(ik!=image_Para.vlK)
			{
				int xDeltaI = image_Para.ComInclines.at(j).pt1.x - image_Para.ComInclines.at(j).pt2.x;
				int yDeltaI = image_Para.ComInclines.at(j).pt1.y - image_Para.ComInclines.at(j).pt2.y;
				double a = abs(xDeltaH * xDeltaI + yDeltaH * yDeltaI);  //表示两条线段的内积
				double b = sqrt(double(xDeltaH * xDeltaH + yDeltaH * yDeltaH)) * sqrt(double(xDeltaI * xDeltaI + yDeltaI * yDeltaI));//两向量模的乘积

				temp_Angle.line2 = image_Para.ComInclines.at(j);
				temp_Angle.angle = static_cast<int>(acos(a / b)/CV_PI * 180);//向量公式和反余弦函数求两条线段的夹角
				temp_Angle.cross.y = bow_row-image_Para.cut_UpRows;  //确定弓的位置后，交点的纵坐标不变
				temp_Angle.cross.x = (int)((temp_Angle.cross.y -image_Para.ComInclines.at(j).b)/image_Para.ComInclines.at(j).k);  //根据公式求出交点的横坐标		
			}
			else 
			{
				temp_Angle.line2 = image_Para.ComInclines.at(j);
				temp_Angle.angle = 90;
				temp_Angle.cross.y = bow_row-image_Para.cut_UpRows;  
				temp_Angle.cross.x = image_Para.ComInclines.at(j).pt1.x; //根据公式求出交点的横坐标		
			}
			angles.push_back(temp_Angle);  //push_back 是在末尾插入数据
		}	
	}
	catch(...)
	{
		return angles;
	}
	
	return angles;
}


int Height_Stagger::EdgeToBlack(Mat& src,int rowNum,int colNum)  //将单通道图像指定的上下左右边界像素值置为0
{
	if(!src.data)
	{
		return NO_PICTURE;
	}

	try
	{
		for(int row = 0;row < rowNum;row++)
		{
			for(int col = 0;col<src.cols;col++)
			{
				(*(src.data + src.step[0]*row + src.step[1]*col)) = 0;	
			}
		}

		for(int row = src.rows-1;row >src.rows-rowNum-1;row--)
		{
			for(int col = 0;col<src.cols;col++)
			{
				(*(src.data + src.step[0]*row + src.step[1]*col)) = 0;	
			}
		}

		for(int col = 0;col < colNum;col++)
		{
			for(int row = 0;row<src.rows;row++)
			{
				(*(src.data + src.step[0]*row + src.step[1]*col)) = 0;	
			}	
		}

		for(int col = src.cols -1;col> src.cols -colNum-1;col--)
		{
			for(int row = 0;row<src.rows;row++)
			{
				(*(src.data + src.step[0]*row + src.step[1]*col)) = 0;	
			}		
		}
	}
	catch (...)
	{
		return 0;	
	}
	
	return 1;
}

cv::Point Height_Stagger::SelectCrossPointByDistance(RecResult& calPoints)
{

	bool IsFindCross = false;  //记录是否找到了接触点
	try
	{
		if(calPoints.pre_cross_Point.x == 0)  //第一帧图像的筛选
		{
			if(calPoints.cross_Angles.size()==1)
			{
				image_Para.cP = calPoints.cross_Angles.at(0).cross;
				calPoints.pre_cross_Point = image_Para.cP;
				IsFindCross = true;
				ObliqueNumber = 0;
				return image_Para.cP;
			}
			else
			{
				int minDist_FromMid = abs(calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x);  //计算距离受电弓中点最近的距离
				int minDist_Num = 0;
				for(size_t i =1;i<calPoints.cross_Angles.size();i++)
				{
					if(abs(calPoints.cross_Angles.at(i).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x) < minDist_FromMid)
					{
						minDist_FromMid = abs(calPoints.cross_Angles.at(i).cross.x - calPoints.midPoint.x);
						minDist_Num = (int)i;
						
					}
				}
				ObliqueNumber = minDist_Num;
				image_Para.cP = calPoints.cross_Angles.at(minDist_Num).cross;
				calPoints.pre_cross_Point = image_Para.cP;
				IsFindCross = true;
				return image_Para.cP;	
			}
		}

		else
		{
			if(calPoints.cross_Angles.size()==1)  //后续图像的筛选
			{
				image_Para.cP = calPoints.cross_Angles.at(0).cross;
				calPoints.pre_cross_Point = image_Para.cP;
				IsFindCross = true;
				ObliqueNumber = 0;
				return image_Para.cP;
			}

			else
			{
				if(calPoints.cross_Angles.size()==2)  //如果只有两条直线，且两条直线在同一侧，则选择离受电弓中点近的接触点
				{
					if((calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x)
						*(calPoints.cross_Angles.at(1).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x)>=0)  //两个交点同在中点的一侧
					{
						if(abs(calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x)
							<= abs(calPoints.cross_Angles.at(1).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x))
						{
							image_Para.cP = calPoints.cross_Angles.at(0).cross;
							ObliqueNumber = 0;
						}
						else
						{
							image_Para.cP = calPoints.cross_Angles.at(1).cross;
							ObliqueNumber = 1;
						}
						calPoints.pre_cross_Point = image_Para.cP;
						IsFindCross = true;
						return image_Para.cP;
					}
				}

				int minDist_FromPreCross = abs(calPoints.cross_Angles.at(0).cross.x - calPoints.pre_cross_Point.x);
				int minDist_AngleNumber = 0;
				for(size_t i = 1;i<calPoints.cross_Angles.size(); i++)  //搜索出与上一帧接触点距离最短的交点
				{
					if(abs(calPoints.cross_Angles.at(i).cross.x - calPoints.pre_cross_Point.x) < minDist_FromPreCross)
					{
						minDist_FromPreCross = abs(calPoints.cross_Angles.at(i).cross.x - calPoints.pre_cross_Point.x);
						minDist_AngleNumber = (int)i;
					}
				}	

				if(minDist_FromPreCross<=10)   //相邻两帧接触点的距离理论上应在10个像素以内
				{
					ObliqueNumber = minDist_AngleNumber;
					image_Para.cP = calPoints.cross_Angles.at(minDist_AngleNumber).cross;
					calPoints.pre_cross_Point = image_Para.cP;  //将最后筛选出的接触点保存起来，用于下一帧的筛选

					if(image_Para.cP.x + image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge > RightMaxPullOut-5 || image_Para.cP.x + image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge < LeftMaxPullOut+5) //判断是否越界
					{
						IsFindCross = false;
					}
					else
					{
						IsFindCross = true;
					}
				}

				if(IsFindCross == false)
				{
					int minDist_FromMidPoint = abs(calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x);  //计算距离受电弓中点最近的距离
					int minDist_Number = 0;
		
					for(size_t i =1;i<calPoints.cross_Angles.size();i++)
					{
						if(abs(calPoints.cross_Angles.at(i).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x) < minDist_FromMidPoint)
						{
							minDist_FromMidPoint = abs(calPoints.cross_Angles.at(i).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x);
							minDist_Number = (int)i;
						}
					}

					ObliqueNumber = minDist_Number;
					image_Para.cP = calPoints.cross_Angles.at(minDist_Number).cross;
					calPoints.pre_cross_Point = image_Para.cP;  //将最后筛选出的接触点赋给上一帧的交点，用于下一帧的筛选
				}

				return image_Para.cP;	
			}
		}	
	}
	catch(...)
	{
		return calPoints.pre_cross_Point;
	}
}

int Height_Stagger:: CalculateSpaceCoordinateByScaleRatio(int bowHeight, int PullOut,double& RealHeight,double& RealStagger)
{
	int scale = image_Para.cur_nHeight / 240; //计算输入图像是240图像的高度之比
	if(xRatio<= 1.0||yRatio <= 1.0)
	{
		RealHeight = 0;
		RealStagger = 0;
		return 0;
	}
	else
	{
		RealHeight = bowHeight*yRatio + camera_Height;
		RealStagger = PullOut*xRatio;
	}
	return 1;
}

bool Height_Stagger::DetectBowRectRegion(Mat& src_gray_Mat,vector<Rect>& rects)
{
	if(image_Para.IsLoadXMLFile == false)  //若没加载xml文件则先加载xml文档
	{
		TCHAR szPath[ MAX_PATH ] = {0};
		GetModuleFileName( NULL, szPath,MAX_PATH );
		int iLen = WideCharToMultiByte(CP_ACP, 0,szPath, -1, NULL, 0, NULL, NULL);
		char* chRtn =new char[iLen*sizeof(char)];
		WideCharToMultiByte(CP_ACP, 0, szPath, -1, chRtn, iLen, NULL, NULL);
		std::string str(chRtn);

		int a = str.find_last_of("/\\");

		if(a>1)
		{
			str = str.substr(0,a+1);
			str = str + image_Para.cascadeName;
		}
		else
		{
			return false;
		}

		if(!image_Para.cascade.load(str))   //加载级联分类器 xml文件
		{  
			loadFlag = false;
			return false;  
		}
		else
		{
			loadFlag = true;
			image_Para.IsLoadXMLFile = true;  //表示已成功加载xml文件
		}
	}

	if(!src_gray_Mat.data)
	{
		return false;
	}

	static vector<Rect> rectBlock;  //检测到受电弓区域的矩形块的存储区
	rectBlock.clear();  //清空

	image_Para.cascade.detectMultiScale(src_gray_Mat,rectBlock,1.05, 6, 0,Size(180,40));
	if(rectBlock.size()>0)
	{
		return true;
	}
	else
	{
		image_Para.cascade.detectMultiScale(src_gray_Mat, rectBlock, 1.05, 5, 0, Size(180, 40));
		if(rectBlock.size()>0)
		{
			return true;
		}
		return false;
	}	

	return true;
}


bool Height_Stagger::DrawRecognizeResult(Mat resultMat,int recFrame)
{
	if(h_wnd==NULL)
	{
		return false;
	}
	if(hdc == NULL)
	{
		hdc = ::GetDC(h_wnd);
		::GetClientRect(h_wnd,&rect);
	}

	int ScaleFactor = image_Para.cur_nHeight / 240;   //该参数表示输入图像相对于240（行数）尺寸的图像的放大倍数 


	//画出导高值报警线
	//if(AlarmHeightPixel>0 &&AlarmHeightPixel< image_Para.cur_nHeight)
	//{
	//	line(resultMat, Point(0, (int)AlarmHeightPixel), Point((int)319 *  ScaleFactor*  x_Scale, (int)AlarmHeightPixel), Scalar(255, 0, 255), 1, 8);  //紫色线条标记出导高位置
	//
	//}


	Mat ScaleResultMat(rect.bottom - rect.top, rect.right - rect.left, CV_8UC3);

	if (TopBowHeightScale == 0 || BottomBowHeightScale == 0 || MinBHFromImgBottom == 0 || IntervalNumber == 0 || heightInterval == 0 || PullOutPixelSpan == 0 || pulloutInterval == 0)
	{
		resize(resultMat, ScaleResultMat, Size(rect.right - rect.left, rect.bottom - rect.top));
		IplImage* srcImg = (&IplImage(ScaleResultMat));
		m_showImg.CopyOf(srcImg, 1);
		m_showImg.DrawToHDC(hdc, &rect);
		ScaleResultMat.release();
		return false;
	}
	else
	{
		//定义几种不同颜色
		const static Scalar colors[] =
		{
			CV_RGB(0, 0, 255),   //蓝
			CV_RGB(0, 255, 0),   //绿
			CV_RGB(255, 255, 0), //紫
			CV_RGB(255, 0, 255)  //黄
		};

		for (int t = 0; t<BowNet_Result.net_Lines.size(); t++)
		{
			if (BowNet_Result.net_Lines.at(t).k == image_Para.vlK)
			{
				BowNet_Result.net_Lines.at(t).y2 = 0;
			}
			else
			{
				BowNet_Result.net_Lines.at(t).y2 = 0;
				BowNet_Result.net_Lines.at(t).x2 = (BowNet_Result.net_Lines.at(t).y2 - BowNet_Result.net_Lines.at(t).b) / BowNet_Result.net_Lines.at(t).k;
			}

			if (t == ObliqueNumber)
			{
				line(resultMat, Point(BowNet_Result.net_Lines.at(t).x1 * ScaleFactor, BowNet_Result.net_Lines.at(t).y1 * ScaleFactor), Point(BowNet_Result.net_Lines.at(t).x2 * ScaleFactor, BowNet_Result.net_Lines.at(t).y2 * ScaleFactor), CV_RGB(255, 0, 0), 1, 8);
			}

			else
			{
				int color_num = t;
				if (color_num >= 4)
				{
					color_num %= 4;
				}
				Scalar color = colors[color_num];
				if (((recResult.cross_Angles.at(t).cross.x + image_Para.cut_LeftCols + image_Para.nFirstCut_LeftEdge)*  ScaleFactor <= RightMaxPullOut - 5) && ((recResult.cross_Angles.at(t).cross.x + image_Para.cut_LeftCols + image_Para.nFirstCut_LeftEdge) * ScaleFactor >= LeftMaxPullOut + 5)) //判断斜线是否在范围内
				{
					line(resultMat, Point(BowNet_Result.net_Lines.at(t).x1* ScaleFactor, BowNet_Result.net_Lines.at(t).y1* ScaleFactor), Point(BowNet_Result.net_Lines.at(t).x2* ScaleFactor, BowNet_Result.net_Lines.at(t).y2* ScaleFactor), color, 1, 8);
				}
			}
		}

		char str[5] = "";
		if (BowNet_Result.pixel_BowHeight != 0)
		{
			line(resultMat, Point(0, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor)*  ScaleFactor), Point((int)319 *  ScaleFactor, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor)*  ScaleFactor), CV_RGB(255, 0, 255), 1, 8);  //标记出导高位置
			line(resultMat, Point((int)(recResult.midPoint.x*  ScaleFactor - PullOutPixelSpan), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor),
				Point((int)(recResult.midPoint.x *  ScaleFactor + PullOutPixelSpan), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);   //画拉出值的水平轴刻度线

			for (int t = BowNet_Result.midPx*ScaleFactor - pulloutInterval, i = -10; t >= BowNet_Result.midPx* ScaleFactor - PullOutPixelSpan; t -= pulloutInterval, i -= 10)   //画左侧拉出值刻度
			{
				itoa(i, str, 10);
				putText(resultMat, str, Point((int)(t - 10 * ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 25)*  ScaleFactor), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 0, 255), 1, 8);
				line(resultMat, Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor), Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 15)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);  //画出拉出值为负的刻度线
			}

			itoa(0, str, 10);
			putText(resultMat, str, Point((int)(BowNet_Result.midPx*  ScaleFactor - 3), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 33)*  ScaleFactor), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 0, 255), 1, 8);	 //在受电弓的中点列标记出0值
			line(resultMat, Point((int)(BowNet_Result.midPx*  ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor),
				Point((int)(BowNet_Result.midPx*  ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 25)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);  //画出拉出值为0 的垂直刻度线

			for (int t = BowNet_Result.midPx* ScaleFactor + pulloutInterval, i = 10; t <= BowNet_Result.midPx* ScaleFactor + PullOutPixelSpan; t += pulloutInterval, i += 10)  //画右侧拉出值刻度
			{
				itoa(i, str, 10);
				putText(resultMat, str, Point((int)(t - 7 * ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 25)*  ScaleFactor), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 0, 255), 1, 8);
				line(resultMat, Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor), Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 15)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);  //画出拉出值为正的刻度线
			}

			if ((recResult.cross_Point.x != 0) && (recResult.cross_Point.y != 0))
			{
				circle(resultMat, Point(BowNet_Result.crossPx*  ScaleFactor, BowNet_Result.crossPy*  ScaleFactor), 4 * ScaleFactor, CV_RGB(255, 255, 0), 2, 8, 0); //标记出交点
			}

		}

		for (int i = BottomBowHeightScale,k =0; i < TopBowHeightScale;i += 100,k++)  //画导高值刻度线
		{
			int t = (int)(resultMat.rows - 1 - MinBHFromImgBottom - k*(100 / yRatio) + 0.5);

			itoa(i, str, 10);
			putText(resultMat, str, Point(1, (t + 3)), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 255, 0), 1, 8);
			line(resultMat, Point(20 * ScaleFactor, t), Point(30 * ScaleFactor, t), Scalar(0, 255, 0), 1, 8);
		}

		line(resultMat, Point(30 * ScaleFactor, 0), Point(30 * ScaleFactor, resultMat.rows - 1), Scalar(0, 255, 0), 1, 8);  //画出垂直的导高标尺刻度线


		string str2 = "MM";
		putText(resultMat, str2, Point(37 * ScaleFactor, 8 * ScaleFactor), FONT_HERSHEY_PLAIN, 0.45*ScaleFactor, Scalar(255, 255, 0), 1, 8);   //增加导高值得标尺单位mm



		if (recResult.pixel_BowHeight != 0)
		{
			//bowRange.x = (int)(BowNet_Result.midPx - 120)*  ScaleFactor;
			//bowRange.y = (int)(240 - BowNet_Result.pixel_BowHeight - 5)*  ScaleFactor;
			//bowRange.height = (int)(53 * y_Scale)*  ScaleFactor;
			//bowRange.width = (int)(230 * x_Scale)*  ScaleFactor;
			//rectangle(resultMat,bowRange,cvScalar(255,0,0),1,8,0);  //画出矩形，蓝色线条

			/******在图像上增添文字******/
			d_g = "导高：";
			itoa((int)BowNet_Result.real_BowHeight, daogao, 10);
			d_g.append(daogao);
			strcpy(c_dg, d_g.c_str());

			s_g = "拉出：";
			itoa((int)BowNet_Result.real_PullOut, lachu, 10);
			s_g.append(lachu);
			strcpy(c_sg, s_g.c_str());

			z_h = "帧号：";
			itoa(recFrame, frame_number, 10);
			z_h.append(frame_number);
			strcpy(c_zh, z_h.c_str());

			putTextZH(resultMat, c_dg, Point(resultMat.cols * 17 / 20, resultMat.rows / 18), CV_RGB(255, 0, 255), 8 * ScaleFactor, "楷体");
			putTextZH(resultMat, c_sg, Point(resultMat.cols * 17 / 20, resultMat.rows * 2 / 18), CV_RGB(255, 0, 255), 8 * ScaleFactor, "楷体");
			putTextZH(resultMat, c_zh, Point(resultMat.cols * 17 / 20, resultMat.rows * 17 / 18), CV_RGB(255, 0, 255), 8 * ScaleFactor, "楷体");
		}
	}

	resize(resultMat, ScaleResultMat, Size(rect.right - rect.left, rect.bottom - rect.top));  //缩放图像至矩形框大小

	//imshow("dst", resultMat);
	//waitKey(5);
	
	IplImage* srcImg = (&IplImage(ScaleResultMat));
	m_showImg.CopyOf(srcImg, 1);
	m_showImg.DrawToHDC(hdc, &rect);
	ScaleResultMat.release();

	return true;
}