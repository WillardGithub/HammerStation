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

	xRatio = 7.96875;   //��ʼ���з���ı���
	yRatio = 8.04167;    //��ʼ���з���ı���
	camera_Height = 5200;  //��ʼ������ĸ߶�

	LeftMaxPullOut = 0;
	RightMaxPullOut = 0; 

	d_g = "���ߣ�";
	s_g = "������";
	z_h = "֡�ţ�";

	TopBowHeightScale = 0;  //ͼ�񶥶˶�Ӧ����󵼸�ֵ
	BottomBowHeightScale = 0; //ͼ���ϵ���ֵ����С�̶�(ȡ��)
	MinBHFromImgBottom = 0; //��С���߿̶�ֵ����ͼ��׶˵����ؾ���
	IntervalNumber = 0;  //����ֵ�̶ȶ�Ӧ���ٶ�
	heightInterval = 0; //����ֵ�̶ȶ�Ӧ�����ؼ��
	PullOutPixelSpan = 0; //���������ֵ��������ط�Χ
	pulloutInterval = 0; //ÿ100mm��Ӧ�������ؿ̶�
	AlarmHeightPixel = 0; //���߱���ֵ��Ӧ������λ��

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

bool Height_Stagger::LoadCalibrationParameters()   //���ر궨����
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
	string postfix= "GuidePull\\InfraredCalibrationPara.txt";  //�����ļ�
	int station = ParameterPath.find_last_of("/\\");
	if(station>1)
	{
		ParameterPath = ParameterPath.substr(0,station+1);
		ParameterPath = ParameterPath + postfix;
	}

	ifstream in(ParameterPath);  
	if(in)   // �и��ļ�  
	{  
		string line_str = ""; 
		int position = 0;
		while (getline(in, line_str))    //line_str�в�����ÿ�еĻ��з�\n  
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

		while(str_tt !=NULL)    //���ַ����еı���ֵ���ո�ָ��洢
		{ 
			str_tt = strtok(NULL," "); 
			if(str_tt== NULL)
			{
				break;
			}
			str_para.push_back(str_tt);
		} 
		
		recResult.midPoint.x = atoi(str_para.at(0));  //�����е���ԭʼͼ���е������꣨�൱����ָ�����е�������꣩
		camera_Height = atoi(str_para.at(1));
		xRatio = atof(str_para.at(2));
		yRatio = atof(str_para.at(3));

		TopBowHeightScale = (int)(camera_Height + yRatio*image_Para.cur_nHeight+0.5);  //ͼ�񶥶˶�Ӧ����󵼸�ֵ
		BottomBowHeightScale = (int)(camera_Height + 100 +0.5);  //ͼ���ϵ���ֵ����С�̶�(ȡ��)
		MinBHFromImgBottom = 100/yRatio; //��С���߿̶�ֵ����ͼ��׶˵����ؾ���
		IntervalNumber = (TopBowHeightScale - BottomBowHeightScale) /100;  //����ֵ�̶ȶ�Ӧ���ٶ�

		heightInterval = (int)(100/yRatio+0.5); //����ֵ�̶ȶ�Ӧ�����ؼ��
		PullOutPixelSpan = (int)(600/xRatio+0.5); //���������ֵ��������ط�Χ
		pulloutInterval = (int)(100/xRatio+0.5); //����ֵ�̶ȶ�Ӧ�����ؼ��

		RightMaxPullOut = recResult.midPoint.x + (int)(600/xRatio+0.5);  //���������������λ��
		LeftMaxPullOut = recResult.midPoint.x - (int)(600/xRatio+0.5);   //���������������λ��


		if (image_Para.cur_nHeight == 240)
		AlarmHeightPixel = (int)(240 - ((6600 - camera_Height)/yRatio) +0.5);  //���ݱ��ʹ�ϵ������߱���ʱ��Ӧ������ֵ
		else
		AlarmHeightPixel = (int)(480 - ((6600 - camera_Height)/yRatio) + 0.5);  //���ݱ��ʹ�ϵ������߱���ʱ��Ӧ������ֵ


		IsRecByTrainer = false;   //ÿ���л�����ʱ�����³�ʼ��Ϊfalse 
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
    	if (nWidth <= 0 || nHeight <= 0)  //ͼ��ߴ����
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
			return false; //Ŀǰ��֧��320��640�ֱ��ʵ�ͼ�񣬷���ֱ�ӷ���false�����ٽ��к�������
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

		image_Para.src_Mat = cvCreateMat(image_Para.nFirstCut_Height ,image_Para.nFirstCut_Width,CV_8UC1);  //��ʼ����ֱ�ӽص�ͼ����ĸ��߽���ͼ��

		image_Para.nSecondCut_TopEdge = 0;
		image_Para.nSecondCut_Height = image_Para.src_Mat.rows;
		image_Para.nSecondCut_LeftEdge = image_Para.src_Mat.cols/6;
		image_Para.nSecondCut_Width = image_Para.src_Mat.cols*2/3;

		image_Para.nThirdCut_LeftEdge = image_Para.src_Mat.cols/8;
		image_Para.nThirdCut_Width = image_Para.src_Mat.cols*3/4;

		image_Para.inPut_PreProcessMat = cvCreateMat(image_Para.src_Mat.rows,image_Para.src_Mat.cols,CV_8UC1);  //Ԥ������������ڿ���src_Mat
		image_Para.input_PreProcessROIMat = cvCreateMat(image_Para.nSecondCut_Height,image_Para.nSecondCut_Width,CV_8UC1); //Ԥ��������н�ȡ��ROI
		image_Para.input_PreProcessROIBinMat = cvCreateMat(image_Para.input_PreProcessROIMat.rows,image_Para.input_PreProcessROIMat.cols,CV_8UC1);

		image_Para.input_PreProcessROICannyMat = cvCreateMat(image_Para.input_PreProcessROIMat.rows,image_Para.input_PreProcessROIMat.cols,CV_8UC1); //Canny�߽�ͼ��

		image_Para.inPut_RecognizeMat = cvCreateMat(image_Para.src_Mat.rows,image_Para.src_Mat.cols,CV_8UC1);  //����ʶ����������ڿ���src_Mat
		image_Para.inPut_RecognizeROIMat = cvCreateMat(image_Para.inPut_RecognizeMat.rows/3,image_Para.nThirdCut_Width,CV_8UC1);
		image_Para.inPut_RecognizeROIBinMat = cvCreateMat(image_Para.inPut_RecognizeROIMat.rows,image_Para.inPut_RecognizeROIMat.cols,CV_8UC1); //���ݶ�λ���Ĺ�λ�ý�ȡ��ROI
	}
	catch(...)
	{
		return;
	}
}

int Height_Stagger::InfraRedImage_ANALYZE(const unsigned char* pGBuffer, UINT iFrame, int*  pullOutValue, int* HeightValue, RECT* bowRegion)
{
	BowNet_Result.midPx =  recResult.midPoint.x;  //�������е�ֵ�����ڲ�����

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

	//recResult.midPoint.x = 188;  //�����е���ԭʼͼ���е������꣨�൱����ָ�����е�������꣩
	//camera_Height = 5300;
	//xRatio = 3.875;
	//yRatio = 4.16667;

	//TopBowHeightScale = camera_Height + yRatio*image_Para.cur_nHeight;  //ͼ�񶥶˶�Ӧ����󵼸�ֵ
	//BottomBowHeightScale = ((int)((camera_Height + 100) * 10 / 1000)) * 100;  //ͼ���ϵ���ֵ����С�̶�(ȡ��)
	//MinBHFromImgBottom = (BottomBowHeightScale - camera_Height) / yRatio; //��С���߿̶�ֵ����ͼ��׶˵����ؾ���
	//IntervalNumber = (TopBowHeightScale - BottomBowHeightScale) / 100;  //����ֵ�̶ȶ�Ӧ���ٶ�

	//heightInterval = 100 / yRatio; //����ֵ�̶ȶ�Ӧ�����ؼ��
	//PullOutPixelSpan = 600 / xRatio; //���������ֵ��������ط�Χ
	//pulloutInterval = 100 / xRatio; //����ֵ�̶ȶ�Ӧ�����ؼ��

	//RightMaxPullOut = recResult.midPoint.x + (int)(600 / xRatio);  //���������������λ��
	//LeftMaxPullOut = recResult.midPoint.x - (int)(600 / xRatio);   //���������������λ��
	//AlarmHeightPixel = (int)(480 - ((6600 - camera_Height) / yRatio));  //���ݱ��ʹ�ϵ������߱���ʱ��Ӧ������ֵ


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

	for(int row = 0;row< image_Para.cur_nHeight;row++)   //��ȡ��ͨ����ͼ������
	{
		for(int col = 0;col<image_Para.cur_nWidth;col++)
		{
			image_Para.src_MatRgb.at<Vec3b>(row,col)[0] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col];
			image_Para.src_MatRgb.at<Vec3b>(row,col)[1] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col+1];
			image_Para.src_MatRgb.at<Vec3b>(row,col)[2] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col+2];
		}
	}

	for(int row = 0;row< image_Para.cur_nHeight;row++)   //��ȡ��ͨ����ͼ������
	{
		for(int col = 0;col<image_Para.cur_nWidth;col++)
		{
			image_Para.pixelImage[row*image_Para.cur_nWidth+col] = pGBuffer[(image_Para.cur_nHeight-1-row)*image_Para.cur_nWidth*3+3*col];
		}
	}

	GrayBufferToMat(image_Para.pixelImage);   //����ת��ΪMatͼ��

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

			IsRecByTrainer = true; //��ʾ��ǰ֡����ѵ�����ж��Ѿ��й�
			int Pre_Result = Pre_Process(recResult.pixel_BowHeight); //Ԥ���������¶�������ȷ������λ��
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
				BowNet_Result.midPy = recResult.pixel_BowHeight + image_Para.nFirstCut_TopEdge;  //���ǻ���320 * 240�ķֱ����������
				BowNet_Result.pixel_BowHeight = Multi_Factor*(240 - BowNet_Result.midPy);  //����(�������ϼ���)  ���ǻ���320*240�ķֱ����������


				//��¼������λ�ã����ڷ����жϸ��µ��Ƿ��ڹ���Χ��
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
					for(size_t t= 0; t<recResult.cross_Angles.size();t++)    //����ϲ������еĽӴ�������
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
					recResult.pixel_PullOut = BowNet_Result.crossPx - BowNet_Result.midPx;   //320�ֱ���ͼ������������ֵ

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

int Height_Stagger::GrayBufferToMat(const unsigned char* pArr)   //��buffer����תΪMat����ͼ��
{
	if(!pArr)
	{
		return NO_PICTURE;
	}
	Mat src(image_Para.cur_nHeight, image_Para.cur_nWidth, CV_8UC1);
	Mat srcNorm(240, 320, CV_8UC1);  //���е�ͼ��ͬһ��׼��Ϊ320�ٽ��д���

	try
	{
		for (int row = 0; row< image_Para.cur_nHeight; row++)
		{
			for (int col = 0; col<image_Para.cur_nWidth; col++)
			{
				src.at<uchar>(row, col) = *(pArr + row* image_Para.cur_nWidth + col);
			}
		}
		resize(src, srcNorm, Size(srcNorm.cols, srcNorm.rows)); //��ͼ������Ϊ320


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

int Height_Stagger::Pre_Process(int& bow_H)   //ͼ���Ԥ��������ȷ���ܵ繭��λ��
{
	bow_H = 0;   //ÿ֡����ǰ����ֵ��ʼ��Ϊ0;
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
		image_Para.input_PreProcessROIMat = image_Para.inPut_PreProcessMat(Range(image_Para.nSecondCut_TopEdge,image_Para.nSecondCut_TopEdge+image_Para.nSecondCut_Height),Range(image_Para.nSecondCut_LeftEdge,image_Para.nSecondCut_LeftEdge+image_Para.nSecondCut_Width));  //��ȡͼ����м��в������������ҵ�����λ��
		
		if(!image_Para.input_PreProcessROIMat.data)
		{
			return NO_PICTURE;
		}

		int BowHeight = ConfirmBowPosition(bow_H);   //ȷ������λ��
		
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
						bow_H = recResult.pre_pixelBowHeight;	 //δ�ҵ��ܵ繭ʱ�Ҳ�������3֡�޹�����ǰһ֡�ĵ���ֵ���н���
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

		else if(BowHeight == BOW_HEIGHT_ERROR)  //��ʾ�ҵ��Ĺ�λ�ù��߻���ͣ�λ�ô���Ҳ��δ�ҵ������൱���Ǵ��ڸ���
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
				if(nContinueNoBow>=3)   //������3֡�޹�
				{
					recResult.pre_pixelBowHeight = bow_H = 0;
					IsRecByTrainer = false;
					return BOW_HEIGHT_ERROR;
				}
				else
				{
					if(recResult.pre_pixelBowHeight != 0)
					{
						bow_H = recResult.pre_pixelBowHeight;  //δ�ҵ��ܵ繭������3֡�޹�ʱ����ǰһ֡�ĵ���ֵ��������
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
				if(abs(bow_H - recResult.pre_pixelBowHeight)>=10)  //�����ǰ֡�ĵ�������һ֡�Ĳ�����10���������õ���ֵ,�൱��Ĭ��ǰ����֡����ֵ��10��������������������Χ
				{
					nContinueBigInterval++;
					if(nContinueBigInterval>=5)
					{
						recResult.pre_pixelBowHeight = bow_H = 0;
						IsRecByTrainer = false;		
					}
					else
					{
						bow_H = recResult.pre_pixelBowHeight;   //����������5������֮֡��ļ���ϴ�������һ֡�ĵ���ֵ���н���
					}			
				}
				else
				{
					nContinueBigInterval = 0;  //����ǰ֡����һ֡���δ����10�����أ������0 
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

int Height_Stagger::ConfirmBowPosition(int& HL)   //��λ������λ��
{
	int maxNum = 0;  //���׵��е�255�ĸ���	

	try
	{
		//��ͳ�ƻҶ�ֵ����205�ĵ���
		int countPixel = 0;
		for (int row = 0; row < image_Para.input_PreProcessROIMat.rows; row+=2)    //�൱����СΪԭ�е�1/4
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

		if (countPixel >= totalNumber / 10)  //�������205�ĵ���ռ�ȳ���1/10
		{
			/****************��С��205�����ص�ȫ��Ϊ0**************/
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

		for(int i = 0;i<image_Para.input_PreProcessROIBinMat.rows;i++)   //ͳ��ÿһ�е�255����
		{
			for(int j = 0;j<image_Para.input_PreProcessROIBinMat.cols;j++)
			{
				if(image_Para.input_PreProcessROIBinMat.at<uchar>(i,j)==255)
				{
					image_Para.pCountHArray[i]++;
				}
			}
		}

		for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)   //��߽�10�е�255����
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

			if(L_edge_num>10)    //���߽��н϶�׵��������ֵ��Ϊ0���൱��ȥ����˵�Ӱ��
			{
				image_Para.pCountHArray[row] = 0;
			}
		}

		for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)   //�ұ߽�20�е�255����
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

		for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)   //�ҳ�255����������
		{		
			if(image_Para.pCountHArray[row]>maxNum)
			{
				maxNum = image_Para.pCountHArray[row];
			}
		}

		if(maxNum<image_Para.input_PreProcessROIBinMat.cols/6)   //������е�255����С�������ص�1/6,���ʾ��ǰ֡ͼ���޹�
		{
			HL = -1;
		}

		else
		{
			for(int row = 0;row<image_Para.input_PreProcessROIBinMat.rows;row++)    /****************���ݰ׵�������㹭��λ��***************/
			{		
				if(image_Para.pCountHArray[row]>maxNum/2)   //���������ҵ��׵�϶����,����MaxNum��1/2
				{
					HL = row;
					int nWhiteCount = 0;       //ͳ��ÿ���м�����İ׵����
					for(int i_col = image_Para.input_PreProcessROIBinMat.cols/3;i_col< 2*image_Para.input_PreProcessROIBinMat.cols/3; i_col++)
					{
						if(image_Para.input_PreProcessROIBinMat.at<uchar>(HL,i_col)==255)
						{
							nWhiteCount++;
						}
					}
					if(nWhiteCount<=maxNum/5)  //�жϸ��е��м�������µ�(��255)�ĸ�������С��maxNum�����֮һ�����޳���
					{
						HL = -1;   //����ǰ�в��������������趨HLΪ-1
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
			Canny(image_Para.input_PreProcessROIMat,image_Para.input_PreProcessROICannyMat,130,150,3);   //Canny����ͻ������(�൱�ڽ��ж�ֵ��)

			for(int k =0;k<image_Para.input_PreProcessROICannyMat.rows;k++)
			{
				image_Para.pCountHArray[k] = 0;
			}

			for(int i = 0;i<image_Para.input_PreProcessROICannyMat.rows;i++)    //ͳ��ÿһ�е�ֵΪ255�ĵ���
			{
				for(int j = 0;j<image_Para.input_PreProcessROICannyMat.cols;j++)
				{
					if(image_Para.input_PreProcessROICannyMat.at<uchar>(i,j)==255)
					{
						image_Para.pCountHArray[i]++;
					}
				}
			}


			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   //��߽�20�е�255����
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
				if(L_edge_num>15)    //���߽��н϶�׵��������ֵ��Ϊ0���൱��ȥ����˵�Ӱ��
				{
					image_Para.pCountHArray[row] = 0;
				}
			}

			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   //�ұ߽�20�е�255����
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
			for(int row = 0;row<image_Para.input_PreProcessROICannyMat.rows;row++)   //�ҳ�255����������
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
				if(image_Para.pCountHArray[row]>maxNum/2)   //���������ҵ��׵�϶����,����MaxNum��1/2,��Ϊ�ܵ繭��λ��
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

int Height_Stagger::GetMaxNumberValue(Mat InputMat)   //�������ֵ�����������ص�
{
	if(!InputMat.data)
	{
		return 0;
	}
	try
	{
		memset(image_Para.pHist,0,sizeof(int)*256); //������Ԫ�ض���Ϊ0
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

int Height_Stagger::GetMaxPixelValue(Mat InputMat)  //���ͼ����������ֵ
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

int Height_Stagger::GetMinPixelValue(Mat InputMat)  //���ͼ�����С����ֵ
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

int Height_Stagger::BowNet_Recognize(RecResult& ret)   //�Ӵ����ʶ��
{
	BOOL Is_Canny = FALSE; //�ж��Ƿ�������Canny���ӽ��ж�ֵ��

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
		/*******************��ȡ�ܵ繭�Ϸ���Χ��ROI����*******************/

		image_Para.inPut_RecognizeROIMat = image_Para.inPut_RecognizeMat(Range(0,ret.pixel_BowHeight),Range(image_Para.nThirdCut_LeftEdge,image_Para.nThirdCut_LeftEdge+image_Para.nThirdCut_Width)); //��ȡimage_Para.inPut_RecognizeMat.rows/3�У�nThirdCut_Width�е�����
		image_Para.cut_UpRows = 0;
		image_Para.cut_LeftCols = image_Para.nThirdCut_LeftEdge;
	
		if(!image_Para.inPut_RecognizeROIMat.data)   //�ж��Ƿ�ɹ���ȡROI
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
	//imshow("���Ϸ�����",image_Para.inPut_RecognizeROIMat);
	//waitKey(0);

	try
	{
		image_Para.MaxCount_Pixel = GetMaxNumberValue(image_Para.inPut_RecognizeROIMat);  //���������������ֵ
		image_Para.MaxValue_Pixel = GetMaxPixelValue(image_Para.inPut_RecognizeROIMat);  //�����������ֵ

		if((image_Para.MaxCount_Pixel<30 && image_Para.MaxValue_Pixel<100) || (image_Para.MaxCount_Pixel>=175 && image_Para.MaxValue_Pixel>=200))   //ͼ��������������,ָ����ֵ���ж�ֵ������
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
		else if(((image_Para.MaxValue_Pixel-image_Para.MaxCount_Pixel)<50))  //���������ֵ���������ֵ���С��50�������ô�򷨽��ж�ֵ��
		{
			GaussianBlur(image_Para.inPut_RecognizeROIMat,image_Para.gauss_Mat,Size(3,3),2,2);
			threshold(image_Para.gauss_Mat,image_Para.inPut_RecognizeROIBinMat, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);  //���ô��(�����䷽��)��ֵ��
		}
		else
		{
			GaussianBlur(image_Para.inPut_RecognizeROIMat,image_Para.gauss_Mat,Size(3,3),2,2);
			Canny(image_Para.gauss_Mat,image_Para.inPut_RecognizeROIBinMat,150,180,3); //Canny����ͻ������(Ҳ�����ж�ֵ��)
			Is_Canny = TRUE;  //��һ��������Canny��������¼ΪTRUE
		}

		if(Is_Canny == FALSE)
		{
			Canny(image_Para.inPut_RecognizeROIBinMat,image_Para.inPut_RecognizeROIBinMat,150,180,3); //Canny����ͻ������
		}

		if(!EdgeToBlack(image_Para.inPut_RecognizeROIBinMat,2,2))  //���߽綼�ú�
		{
		  return NOT_FOUND_CROSSPOINT;
		}

		//imshow("���Ϸ�����binary",image_Para.inPut_RecognizeROIBinMat);
		//waitKey(30);

		HoughLinesP(image_Para.inPut_RecognizeROIBinMat, image_Para.dectecedLines,1,CV_PI/180,image_Para.inPut_RecognizeROIBinMat.rows*2/5,image_Para.inPut_RecognizeROIBinMat.rows*2/5,image_Para.inPut_RecognizeROIBinMat.rows/3);  //houghֱ�߼��
		if(image_Para.dectecedLines.size()==0)
		{
			return NOT_FOUND_LINE;
		}
		else 
		{
			CombineOblique();  //ֱ�ߺϲ�
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
			Detect_Angles(ret.pixel_BowHeight,ret.cross_Angles);  //�н���ȡ
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
	int groupNumber = 0;  //ֱ���������
	image_Para.lineteamI.number = 1; //Ĭ��ÿ��ֱ�߶���Ϊһ�飬ÿ�����һ��ֱ��
	image_Para.lineteamI.maxD = 0.0; //����ֱ�߳�ʼ���������Ϊ0

	image_Para.detectedInclines.clear();
	image_Para.ComInclines.clear();

	try
	{
		for(size_t i = 0; i < image_Para.dectecedLines.size(); i++)
		{
			//ѡȡ��ֱ����
			if(image_Para.dectecedLines[i][2] - image_Para.dectecedLines[i][0] == 0)  //ֱ��i�������˵�ĺ����꣨�����ڵ��У�������ͬ��������ֱ����һ������
			{
				image_Para.ptI1.x = image_Para.dectecedLines[i][0];
				image_Para.ptI1.y = image_Para.dectecedLines[i][1];
				image_Para.ptI2.x = image_Para.dectecedLines[i][2];
				image_Para.ptI2.y = image_Para.dectecedLines[i][3];

				image_Para.temp_Incline.pt1 = image_Para.ptI1;
				image_Para.temp_Incline.pt2 = image_Para.ptI2;
				image_Para.temp_Incline.k = image_Para.vlK; 
				image_Para.temp_Incline.b = image_Para.ptI1.x ; //����ֱ�ߵĽؾඨ��Ϊ�������
				image_Para.detectedInclines.push_back(image_Para.temp_Incline);
				bFlagI = true;
				continue;
			}

			//����б��
			double k = (double)(image_Para.dectecedLines[i][3] - image_Para.dectecedLines[i][1]) / (image_Para.dectecedLines[i][2] - image_Para.dectecedLines[i][0]); //������֮����Ϻ�����֮���Ϊֱ�ߵ�б��
			//ѡȡ��б����
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


		/*****************��ÿ��ֱ�ߵ������˵�������궼������ͬһ���߶�***************/
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

		////�ϲ���б��ֱ�ߣ�ÿ���Ӵ����п�ȣ�����һ��������������߽磬��Ҫ��Ϊ�˽������߽�ϲ���
		/**********
		Method1:����ֱ�ߵ�б�ʺͽؾ���кϲ�
		**********************/
		//for(size_t i = 0; i < image_Para.detectedInclines.size(); i++)
		//{
		//	int iFlag = 0;
		//	double deltaB = 0.0,deltaK = 0.0;
		//	for (size_t j = 0; j < image_Para.ComInclines.size(); j++)
		//	{
		//		deltaK = abs(image_Para.detectedInclines.at(i).k - image_Para.ComInclines.at(j).k);
		//		deltaB = abs(image_Para.detectedInclines.at(i).b - image_Para.ComInclines.at(j).b); 
		//		if (deltaK <= 0.35 && deltaB <= 80)  //�ϲ�б�ʲ����0.35��Χ�ڣ��ؾ���80���������ڵ�б������
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
		//			image_Para.detectedInclines.at(i).groupNumber = (int)j;  //��ʾ��i��ֱ�߹�Ϊ��j��
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
		Method2:����ֱ�����˵�ĺ�����֮��ľ������ֱ�ߺϲ�
		****************/
		for(size_t i = 0; i < image_Para.detectedInclines.size(); i++)
		{
			int iFlag = 0;
		    int delta_LX = 0,delta_RX = 0;    //��������б�ߵ�����˵�ĺ�����֮��ľ���ֵ
			for (size_t j = 0; j < image_Para.ComInclines.size(); j++)
			{
				delta_LX = abs(image_Para.detectedInclines.at(i).pt1.x - image_Para.ComInclines.at(j).pt1.x);
				delta_RX = abs(image_Para.detectedInclines.at(i).pt2.x - image_Para.ComInclines.at(j).pt2.x);

				if (delta_LX <= 3 && delta_LX <= 3 && abs(image_Para.detectedInclines.at(i).k - image_Para.detectedInclines.at(j).k)<2)   //�����˵�ĺ�����֮��С��5
				{
					double mid_LY = (double) (image_Para.detectedInclines.at(i).pt1.y +  image_Para.ComInclines.at(j).pt1.y) / 2;  //������˵���е�������
					double mid_LX = (double) (image_Para.detectedInclines.at(i).pt1.x +  image_Para.ComInclines.at(j).pt1.x) / 2;  //������˵���е������
					double mid_RY=  (double) (image_Para.detectedInclines.at(i).pt2.y +  image_Para.ComInclines.at(j).pt2.y) / 2;  //�����Ҷ˵���е�������
					double mid_RX = (double) (image_Para.detectedInclines.at(i).pt2.x +  image_Para.ComInclines.at(j).pt2.x) / 2;  //������˵���е������

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
					image_Para.detectedInclines.at(i).groupNumber = (int)j;  //��ʾ��i��ֱ�߹�Ϊ��j��
					iFlag = 1;
					break;
				}
			}
			if (iFlag == 1)
			{
				continue;
			}
			else   //û�����ֱ��ʱ,�͵�����Ϊһ��ֱ��
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


vector<BOW_NET_ANGLE> Height_Stagger::Detect_Angles(int bow_row,vector<BOW_NET_ANGLE>& angles)  //�����б�����빭�ļн�
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
			double ik = image_Para.ComInclines.at(j).k;  //��¼��бֱ�ߵ�б��
			double ib = image_Para.ComInclines.at(j).b;  //��¼��бֱ�ߵĽؾ�

			if(ik!=image_Para.vlK)
			{
				int xDeltaI = image_Para.ComInclines.at(j).pt1.x - image_Para.ComInclines.at(j).pt2.x;
				int yDeltaI = image_Para.ComInclines.at(j).pt1.y - image_Para.ComInclines.at(j).pt2.y;
				double a = abs(xDeltaH * xDeltaI + yDeltaH * yDeltaI);  //��ʾ�����߶ε��ڻ�
				double b = sqrt(double(xDeltaH * xDeltaH + yDeltaH * yDeltaH)) * sqrt(double(xDeltaI * xDeltaI + yDeltaI * yDeltaI));//������ģ�ĳ˻�

				temp_Angle.line2 = image_Para.ComInclines.at(j);
				temp_Angle.angle = static_cast<int>(acos(a / b)/CV_PI * 180);//������ʽ�ͷ����Һ����������߶εļн�
				temp_Angle.cross.y = bow_row-image_Para.cut_UpRows;  //ȷ������λ�ú󣬽���������겻��
				temp_Angle.cross.x = (int)((temp_Angle.cross.y -image_Para.ComInclines.at(j).b)/image_Para.ComInclines.at(j).k);  //���ݹ�ʽ�������ĺ�����		
			}
			else 
			{
				temp_Angle.line2 = image_Para.ComInclines.at(j);
				temp_Angle.angle = 90;
				temp_Angle.cross.y = bow_row-image_Para.cut_UpRows;  
				temp_Angle.cross.x = image_Para.ComInclines.at(j).pt1.x; //���ݹ�ʽ�������ĺ�����		
			}
			angles.push_back(temp_Angle);  //push_back ����ĩβ��������
		}	
	}
	catch(...)
	{
		return angles;
	}
	
	return angles;
}


int Height_Stagger::EdgeToBlack(Mat& src,int rowNum,int colNum)  //����ͨ��ͼ��ָ�����������ұ߽�����ֵ��Ϊ0
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

	bool IsFindCross = false;  //��¼�Ƿ��ҵ��˽Ӵ���
	try
	{
		if(calPoints.pre_cross_Point.x == 0)  //��һ֡ͼ���ɸѡ
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
				int minDist_FromMid = abs(calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x);  //��������ܵ繭�е�����ľ���
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
			if(calPoints.cross_Angles.size()==1)  //����ͼ���ɸѡ
			{
				image_Para.cP = calPoints.cross_Angles.at(0).cross;
				calPoints.pre_cross_Point = image_Para.cP;
				IsFindCross = true;
				ObliqueNumber = 0;
				return image_Para.cP;
			}

			else
			{
				if(calPoints.cross_Angles.size()==2)  //���ֻ������ֱ�ߣ�������ֱ����ͬһ�࣬��ѡ�����ܵ繭�е���ĽӴ���
				{
					if((calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x)
						*(calPoints.cross_Angles.at(1).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x)>=0)  //��������ͬ���е��һ��
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
				for(size_t i = 1;i<calPoints.cross_Angles.size(); i++)  //����������һ֡�Ӵ��������̵Ľ���
				{
					if(abs(calPoints.cross_Angles.at(i).cross.x - calPoints.pre_cross_Point.x) < minDist_FromPreCross)
					{
						minDist_FromPreCross = abs(calPoints.cross_Angles.at(i).cross.x - calPoints.pre_cross_Point.x);
						minDist_AngleNumber = (int)i;
					}
				}	

				if(minDist_FromPreCross<=10)   //������֡�Ӵ���ľ���������Ӧ��10����������
				{
					ObliqueNumber = minDist_AngleNumber;
					image_Para.cP = calPoints.cross_Angles.at(minDist_AngleNumber).cross;
					calPoints.pre_cross_Point = image_Para.cP;  //�����ɸѡ���ĽӴ��㱣��������������һ֡��ɸѡ

					if(image_Para.cP.x + image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge > RightMaxPullOut-5 || image_Para.cP.x + image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge < LeftMaxPullOut+5) //�ж��Ƿ�Խ��
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
					int minDist_FromMidPoint = abs(calPoints.cross_Angles.at(0).cross.x+image_Para.cut_LeftCols+image_Para.nFirstCut_LeftEdge - calPoints.midPoint.x);  //��������ܵ繭�е�����ľ���
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
					calPoints.pre_cross_Point = image_Para.cP;  //�����ɸѡ���ĽӴ��㸳����һ֡�Ľ��㣬������һ֡��ɸѡ
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
	int scale = image_Para.cur_nHeight / 240; //��������ͼ����240ͼ��ĸ߶�֮��
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
	if(image_Para.IsLoadXMLFile == false)  //��û����xml�ļ����ȼ���xml�ĵ�
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

		if(!image_Para.cascade.load(str))   //���ؼ��������� xml�ļ�
		{  
			loadFlag = false;
			return false;  
		}
		else
		{
			loadFlag = true;
			image_Para.IsLoadXMLFile = true;  //��ʾ�ѳɹ�����xml�ļ�
		}
	}

	if(!src_gray_Mat.data)
	{
		return false;
	}

	static vector<Rect> rectBlock;  //��⵽�ܵ繭����ľ��ο�Ĵ洢��
	rectBlock.clear();  //���

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

	int ScaleFactor = image_Para.cur_nHeight / 240;   //�ò�����ʾ����ͼ�������240���������ߴ��ͼ��ķŴ��� 


	//��������ֵ������
	//if(AlarmHeightPixel>0 &&AlarmHeightPixel< image_Para.cur_nHeight)
	//{
	//	line(resultMat, Point(0, (int)AlarmHeightPixel), Point((int)319 *  ScaleFactor*  x_Scale, (int)AlarmHeightPixel), Scalar(255, 0, 255), 1, 8);  //��ɫ������ǳ�����λ��
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
		//���弸�ֲ�ͬ��ɫ
		const static Scalar colors[] =
		{
			CV_RGB(0, 0, 255),   //��
			CV_RGB(0, 255, 0),   //��
			CV_RGB(255, 255, 0), //��
			CV_RGB(255, 0, 255)  //��
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
				if (((recResult.cross_Angles.at(t).cross.x + image_Para.cut_LeftCols + image_Para.nFirstCut_LeftEdge)*  ScaleFactor <= RightMaxPullOut - 5) && ((recResult.cross_Angles.at(t).cross.x + image_Para.cut_LeftCols + image_Para.nFirstCut_LeftEdge) * ScaleFactor >= LeftMaxPullOut + 5)) //�ж�б���Ƿ��ڷ�Χ��
				{
					line(resultMat, Point(BowNet_Result.net_Lines.at(t).x1* ScaleFactor, BowNet_Result.net_Lines.at(t).y1* ScaleFactor), Point(BowNet_Result.net_Lines.at(t).x2* ScaleFactor, BowNet_Result.net_Lines.at(t).y2* ScaleFactor), color, 1, 8);
				}
			}
		}

		char str[5] = "";
		if (BowNet_Result.pixel_BowHeight != 0)
		{
			line(resultMat, Point(0, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor)*  ScaleFactor), Point((int)319 *  ScaleFactor, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor)*  ScaleFactor), CV_RGB(255, 0, 255), 1, 8);  //��ǳ�����λ��
			line(resultMat, Point((int)(recResult.midPoint.x*  ScaleFactor - PullOutPixelSpan), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor),
				Point((int)(recResult.midPoint.x *  ScaleFactor + PullOutPixelSpan), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);   //������ֵ��ˮƽ��̶���

			for (int t = BowNet_Result.midPx*ScaleFactor - pulloutInterval, i = -10; t >= BowNet_Result.midPx* ScaleFactor - PullOutPixelSpan; t -= pulloutInterval, i -= 10)   //���������ֵ�̶�
			{
				itoa(i, str, 10);
				putText(resultMat, str, Point((int)(t - 10 * ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 25)*  ScaleFactor), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 0, 255), 1, 8);
				line(resultMat, Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor), Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 15)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);  //��������ֵΪ���Ŀ̶���
			}

			itoa(0, str, 10);
			putText(resultMat, str, Point((int)(BowNet_Result.midPx*  ScaleFactor - 3), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 33)*  ScaleFactor), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 0, 255), 1, 8);	 //���ܵ繭���е��б�ǳ�0ֵ
			line(resultMat, Point((int)(BowNet_Result.midPx*  ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor),
				Point((int)(BowNet_Result.midPx*  ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 25)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);  //��������ֵΪ0 �Ĵ�ֱ�̶���

			for (int t = BowNet_Result.midPx* ScaleFactor + pulloutInterval, i = 10; t <= BowNet_Result.midPx* ScaleFactor + PullOutPixelSpan; t += pulloutInterval, i += 10)  //���Ҳ�����ֵ�̶�
			{
				itoa(i, str, 10);
				putText(resultMat, str, Point((int)(t - 7 * ScaleFactor), (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 25)*  ScaleFactor), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 0, 255), 1, 8);
				line(resultMat, Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 5)*  ScaleFactor), Point((int)t, (int)(240 - BowNet_Result.pixel_BowHeight / ScaleFactor + 15)*  ScaleFactor), Scalar(255, 255, 0), 1, 8);  //��������ֵΪ���Ŀ̶���
			}

			if ((recResult.cross_Point.x != 0) && (recResult.cross_Point.y != 0))
			{
				circle(resultMat, Point(BowNet_Result.crossPx*  ScaleFactor, BowNet_Result.crossPy*  ScaleFactor), 4 * ScaleFactor, CV_RGB(255, 255, 0), 2, 8, 0); //��ǳ�����
			}

		}

		for (int i = BottomBowHeightScale,k =0; i < TopBowHeightScale;i += 100,k++)  //������ֵ�̶���
		{
			int t = (int)(resultMat.rows - 1 - MinBHFromImgBottom - k*(100 / yRatio) + 0.5);

			itoa(i, str, 10);
			putText(resultMat, str, Point(1, (t + 3)), FONT_HERSHEY_PLAIN, 0.5*ScaleFactor, Scalar(255, 255, 0), 1, 8);
			line(resultMat, Point(20 * ScaleFactor, t), Point(30 * ScaleFactor, t), Scalar(0, 255, 0), 1, 8);
		}

		line(resultMat, Point(30 * ScaleFactor, 0), Point(30 * ScaleFactor, resultMat.rows - 1), Scalar(0, 255, 0), 1, 8);  //������ֱ�ĵ��߱�߿̶���


		string str2 = "MM";
		putText(resultMat, str2, Point(37 * ScaleFactor, 8 * ScaleFactor), FONT_HERSHEY_PLAIN, 0.45*ScaleFactor, Scalar(255, 255, 0), 1, 8);   //���ӵ���ֵ�ñ�ߵ�λmm



		if (recResult.pixel_BowHeight != 0)
		{
			//bowRange.x = (int)(BowNet_Result.midPx - 120)*  ScaleFactor;
			//bowRange.y = (int)(240 - BowNet_Result.pixel_BowHeight - 5)*  ScaleFactor;
			//bowRange.height = (int)(53 * y_Scale)*  ScaleFactor;
			//bowRange.width = (int)(230 * x_Scale)*  ScaleFactor;
			//rectangle(resultMat,bowRange,cvScalar(255,0,0),1,8,0);  //�������Σ���ɫ����

			/******��ͼ������������******/
			d_g = "���ߣ�";
			itoa((int)BowNet_Result.real_BowHeight, daogao, 10);
			d_g.append(daogao);
			strcpy(c_dg, d_g.c_str());

			s_g = "������";
			itoa((int)BowNet_Result.real_PullOut, lachu, 10);
			s_g.append(lachu);
			strcpy(c_sg, s_g.c_str());

			z_h = "֡�ţ�";
			itoa(recFrame, frame_number, 10);
			z_h.append(frame_number);
			strcpy(c_zh, z_h.c_str());

			putTextZH(resultMat, c_dg, Point(resultMat.cols * 17 / 20, resultMat.rows / 18), CV_RGB(255, 0, 255), 8 * ScaleFactor, "����");
			putTextZH(resultMat, c_sg, Point(resultMat.cols * 17 / 20, resultMat.rows * 2 / 18), CV_RGB(255, 0, 255), 8 * ScaleFactor, "����");
			putTextZH(resultMat, c_zh, Point(resultMat.cols * 17 / 20, resultMat.rows * 17 / 18), CV_RGB(255, 0, 255), 8 * ScaleFactor, "����");
		}
	}

	resize(resultMat, ScaleResultMat, Size(rect.right - rect.left, rect.bottom - rect.top));  //����ͼ�������ο��С

	//imshow("dst", resultMat);
	//waitKey(5);
	
	IplImage* srcImg = (&IplImage(ScaleResultMat));
	m_showImg.CopyOf(srcImg, 1);
	m_showImg.DrawToHDC(hdc, &rect);
	ScaleResultMat.release();

	return true;
}