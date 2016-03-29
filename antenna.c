/***********************************************************************
* 
* 时    间：2008-10-9
*
* 文 件 名：antenna.c
*
* 版    本：AKD10P07(发布版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：天线六个状态函数          
*
***********************************************************************/
#include "c8051f120.h"
#include "timer.h"
#include "uart.h"
#include "rweeprom.h" 
#include "adc.h"
#include "station.h"
#include "sharp.h"
#include "public.h"
#include "antenna.h"
#include "protcol.h"
#include "idvbsxrx.h"
#include "polar.h"
#include "math.h"
#include "XZ_8802.h"//Rill add 101202

/*静态函数*/
static void TestSelf(void);
static void GetPara(void);
static void ParaProtect(void);
static void TrackingSucceed(void);
static void TackingUpDownRightLeft(float DeltaAz,float DeltaEl, float St);
static void AntTestFun(void);
static void SetBianBan(void);


/*2009/6/20*/
static void GetBaseStarPara(void);
static void TestBaseStarPara(void);
static void GetParaEn(void);
static void GoToObjStar(void);
static void StorSrcPara(void);
static void	GetParaStor(void);
static UINT8 GetOptPara(void);


/***********************************************************************
*
* 函数原型：static void	TestBaseStarPara(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：检查各数据是否正确
*
***********************************************************************/
static void TestBaseStarPara(void)
{
	if((BaseStarLONG > 180.0) || (BaseStarLONG < 0.0))				//经度0.0到180.0的范围
	{
		BaseStarLONG = 134.0;										//亚三
	}
	if(BaseStarPol > V)
	{
		BaseStarPol = H;	
	}
	if(BaseStarKbS < 0.0 || BaseStarKbS > 50000.0)
	{
		BaseStarKbS = 27500.0;
	}
	if(BaseStarRType > SHARPREC)
	{
		BaseStarRType = SHARPREC;		
	}
	if((BaseStarFreq < 899.0) || (BaseStarFreq > 2160.1))		
	{
		BaseStarFreq = 1135.0;
	}
	if(BaseStarLONGEF > EAST)
	{
		BaseStarLONGEF = EAST;	
	}		
}


/***********************************************************************
*
* 函数原型：static void	SetBianBan(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：等待装边瓣函数(2009-3-11)
*
***********************************************************************/
static void SetBianBan(void)
{
	OpenTimer0Interrupt();								//打开计数器,计120秒
	  while(ChangeKit != 0 && 	\
			 !BianBanFlag &&   	\
			 OverflowT0 < 1200)							//如果正常开机，并且边瓣没有安装
	{
		Delay(10000);											
		SendKJ();										//发送装边瓣命令
		Delay(10000);											
		watch();		
	}
	CloseTimer0Interrupt();	
}


/***********************************************************************
*
* 函数原型：static void	TestSelf(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：GPS，DVB接收机,信标接收机的测试
*
***********************************************************************/
static void TestSelf(void)
{
	ReadGradient();

	StationPol = GetComPolA();
//	if(StationPol < 185.0 && StationPol > -5.0)
//	{
//		PloarNormal = 1;					 //极化不正常
//	}
//	else
//	{
//		PloarNormal = 0;					 //极化正常
//	}

	XinBiaoFlagNum = FALSE;
	GetXinBiaoVER();	 //2009/9/11加入新的对信标接收机版本号的
	AGC = GetAGC();
	SharpRecNormalFlag = SetSharpFreq1(SharpRecFreq, SSrcStarKbS);
	SetXinBiaoFreqKC(XinBiaoRecFreq);
	ReadGPS();
}


/***********************************************************************
*
* 函数原型：void GetBaseStarPara(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：读取基准卫星参数
*
***********************************************************************/
static void GetBaseStarPara(void)
{
	BaseStarLONG		= ReadEEPROM(BaseStarLONGAddr) + 			\
					  	  ReadEEPROM(BaseStarLONGAddr + 1) * 0.01;							
	BaseStarPol			= ReadEEPROM(BaseStarPolAddr);						

	BaseStarKbS			= ReadEEPROM(BaseStarKbSAddr) * 10000.0 + 	\
						  ReadEEPROM(BaseStarKbSAddr + 1) * 100.0 + \
						  ReadEEPROM(BaseStarKbSAddr + 2);						  				

	BaseStarRType		= ReadEEPROM(BaseStarRTypeAddr);					

	BaseStarLONGEF  	= ReadEEPROM(BaseStarLONGEFAddr);

	if(BaseStarRType == XINBIAOREC)
	{
		BaseStarFreq 	= ReadEEPROM(BaseStarFreqAddr) * 100.0 + 	\
						  ReadEEPROM(BaseStarFreqAddr + 1) + 		\
						  ReadEEPROM(BaseStarFreqAddr + 2) * 0.01 + \
						  ReadEEPROM(BaseStarFreqAddr + 3) * 0.001; 
	}
	else
	{
		BaseStarFreq 	= ReadEEPROM(BaseStarFreqAddr) * 100.0 + 	\
					  	  ReadEEPROM(BaseStarFreqAddr + 1) + 		\
					      ReadEEPROM(BaseStarFreqAddr + 2) * 0.01 + \
						  ReadEEPROM(BaseStarFreqAddr + 3) * 0.001; 	
	}			
}


/***********************************************************************
*
* 函数原型：void GetPara(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：读取对星所需的必要参数
*
***********************************************************************/
static void GetPara(void)
{
	StationLong		= ReadEEPROM(StationLongAddress) + \
					  ReadEEPROM(StationLongAddress + 1) * 0.01;

	StationEastFlag	= ReadEEPROM(StationEastFlagAddress);

	StationLat		= ReadEEPROM(StationLatAddress) + \
					  ReadEEPROM(StationLatAddress + 1) * 0.01;

	StationNorthFlag= ReadEEPROM(StationNorthFlagAddress);
	
	SatLong 		= ReadEEPROM(SatLongAddress) + \
					  ReadEEPROM(SatLongAddress + 1) * 0.01;			//读取默认卫星经度

	StationPloMode	= ReadEEPROM(StationPloModeAddress);				//读取默认地球站极化方式
	
	ReceiverKindFlag= ReadEEPROM(ReceiverKindAddress);					//上次选择的接收机类别
	
	XinBiaoRecFreq 	= ReadEEPROM(SatXinBiaoFreqAddress) * 100.0 + \
					  ReadEEPROM(SatXinBiaoFreqAddress + 1) + \
					  ReadEEPROM(SatXinBiaoFreqAddress + 2) * 0.01 + \
					  ReadEEPROM(SatXinBiaoFreqAddress + 3) * 0.001;

//	XinBiaoThreshold= ReadEEPROM(XBThresholdAddress) * 0.1;	
	XinBiaoThreshold= 0.1;
	SharpRecFreq    = ReadEEPROM(SatSharpFreqAddress) * 100.0 + \
					  ReadEEPROM(SatSharpFreqAddress + 1) + \
					  ReadEEPROM(SatSharpFreqAddress + 2) * 0.01 + \
					  ReadEEPROM(SatSharpFreqAddress + 3) * 0.001;

	SharpThreshold	= ReadEEPROM(SharpAgcThresholdAddress) * 0.1;		//门限存储的是10倍值

	SSrcStarKbS		= ReadEEPROM(SSrcStarKbSAddr) * 10000.0 + \
					  ReadEEPROM(SSrcStarKbSAddr + 1) * 100.0 +	\
					  ReadEEPROM(SSrcStarKbSAddr + 2);

	SatLongEastFlag = ReadEEPROM(SatLongEastFlagAddr);


  	if(!((XinBiaoRecFreq > 899.0) && (XinBiaoRecFreq < 1760.1)))		//KLR3000接收机的锁频范围(950.0~1750.0 MHZ)
	{
		XinBiaoRecFreq = 1449.015;										//水平极化频率
	}

	if(!((SharpRecFreq > 899.0) && (SharpRecFreq < 2160.1)))			//SHARP接收机的锁频范围(950.0~2150.0 MHZ)
	{
		SharpRecFreq = 1062.0;											//水平极化频率
	}
}


/***********************************************************************
*
* 函数原型：void ParaProtect(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：1，对地球站经纬度，目标卫星经度，接收机接收频率，进行检测；
*
*		    2，数据不合法则保护成北京位置对亚洲三号105.5的情况；
*
*		    3，根据当前接收机的类别判断选择何种接收机
*
***********************************************************************/
static void ParaProtect(void)
{
	if(!(StationLong < 180.0 && StationLong > 0.0) || \
		!(StationLat > 0.0 && StationLat < 90.0))				//经度0.0到180.0的范围；纬度0.0到90.0的范围
	{
		StationLong = 116.46;									//北京经度116.46
		StationLat  = 39.92;									//北京纬度39.92
	}	
	
	if(!(SatLong < 180.0 && SatLong > 0.0))						//经度0.0到180.0的范围
	{
		SatLong = 105.5;										//亚三
	}

	if(!(XinBiaoThreshold < 9.9 && XinBiaoThreshold > 0.0))
	{
		XinBiaoThreshold = 0.2;									//信标接收机门限保护设置为0.3
	}

	if(!(SharpThreshold < 9.9 && SharpThreshold > 0.0))
	{
		SharpThreshold = 0.2;									//夏普接收机门限保护设置为0.3
	}

  	if(!((XinBiaoRecFreq > 899.0) && (XinBiaoRecFreq < 1760.1)))//KLR3000接收机的锁频范围(950.0~1750.0 MHZ)
	{
		XinBiaoRecFreq = 1449.015;								//水平极化频率
		StationPloMode = H;
	}

	if(StationPloMode > V)										//极化方式2009-3-12修改
	{
		StationPloMode = H;	
	}

	if(!((SharpRecFreq > 899.0) && (SharpRecFreq < 2160.1)))	//SHARP接收机的锁频范围(950.0~2150.0 MHZ)
	{
		SharpRecFreq = 1062.0;									//水平极化频率
		StationPloMode = H;
	}

	if(ReceiverKindFlag == XINBIAOREC)							//若当前选择为信标接收机
	{
		if(XinBiaoRecNormalFlag == 1)							//信标接收机正常
		{
			Threshold = XinBiaoThreshold;						//选择信标门限
		}
		else
		{
			ReceiverKindFlag = SHARPREC;
			Threshold = SharpThreshold;							//选择夏普门限
		}
	}
	else
	{
		if(XinBiaoRecNormalFlag == 1 && SharpRecNormalFlag == 0)
		{
			ReceiverKindFlag = XINBIAOREC;
			Threshold = XinBiaoThreshold;						//选择信标门限
		}
		else
		{
			ReceiverKindFlag = SHARPREC;
			Threshold = SharpThreshold;							//选择夏普门限
		}
	}
	if(StationEastFlag > EAST)									//2009-2-17修改
	{
		 StationEastFlag = EAST;
	}
	if(StationNorthFlag > NORTH)								//2009-2-17修改
	{
		 StationNorthFlag = NORTH;
	}
	if(SSrcStarKbS < 0.0 || SSrcStarKbS > 99999.0)
	{
		SSrcStarKbS = 27500.0;
	}

	if(SatLongEastFlag > EAST)
	{
		SatLongEastFlag = EAST;	
	}
}
/***********************************************************************
*
* 函数原型：void PolarAngleInit(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：极化角初始化，电位器与极化器的转圈比为3：1，因此将圆极化器的角度分为3段，此函数将极化器转到限位，并将段数初始化为0
*
***********************************************************************/
void PolarAngleInit(void)
{
	POLAR_LEFT;
	while(ELMidLimit)
	{
		;
	}
	POLAR_STOP;
	PloarAngleStage = 0;
	return;
}

/***********************************************************************
*
* 函数原型：void AntennaInit(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线初始化
*
* 修    改：35度以下都得有装边瓣（2009-3-11，杨淳雯）
*
* 修    改：加入天线可以到俯仰91的相关代码（2009-4-13，杨淳雯）
*
***********************************************************************/
void AntennaInit(void)
{
	if(ChangeKit == 0)
	{
		Delay(20);
		if(ChangeKit == 0)	
		{
			AntTestFun();
		}	
	}

	/*2009/11/4加入对加密芯片的处理*/
	EA 	  = 0;								//2010/4/19加入，芯片优化	//Rill move here 101202
/*	if(!(Verify8802()))
	{
		while(1)
		{
			LEDRED;
			TimeTest(1);
			LEDCLOSE;
			TimeTest(1);
		}
	}		  */
	EA 	  = 1;

	LEDRED;
///*---------------------------------------------------------------------
//*/开机按键2009-1-20	
	while(1) 												//开机按键2009-1-20
	{
		OverflowT0 	= 0;
	  	if(ChangeKit == 0)
		{
			Delay(20);
			OpenTimer0Interrupt();
			while(OverflowT0 < 1200 && ChangeKit == 0)		//aproximately 276s/4.6minite
			{
				if((OverflowT0 & 7) == 0)
				{
					LEDCLOSE;
				}
				if((OverflowT0 & 7) == 4)
				{
					LEDRED;									//闪红灯				
				}
			}
		}
		CloseTimer0Interrupt();
		LEDRED;								  		
		if(OverflowT0 > 1)									//2009-3-9改为0.1秒开机时间	
		{
			break;
		}
	}//end while(1) 开机按键2009-1-20
/*---------------------------------------------------------------------*/
	LEDRED;

	SatLongEastFlag = EAST;
	watchwire();											//检查有无线切换
	StationEl = 10.0;
	NoMonitorF = 1;											//默认无监控
	AntReadySearchF = 1;									//初始化准备搜索标致位(2009/10/12)

	PolarAngleInit();
	StationPol = GetComPolA();
	GotoPolarAngle(45);										//

	Abnormal = ReadEEPROM(AbnormalFlagAddress);				//读取是否正常开机标志，Abnormal=0为正常开机，Abnormal=1为不正常开机
	
	if(ReadEEPROM(PolCompensateAddress) == '+')				//极化的校正
	{
		AngleCom = ReadEEPROM(PolCompensateAddress + 1);
	}
	if(ReadEEPROM(PolCompensateAddress) == '-')
	{
		AngleCom = 0.0 - ReadEEPROM(PolCompensateAddress + 1);
	}
	if(ReadEEPROM(ELCompensateAddress) == '+')	   			//俯仰的校正
	{
		GradientRightR = ReadEEPROM(ELCompensateAddress + 1) / 10.0;
	}
	if(ReadEEPROM(ELCompensateAddress) == '-')
	{
		GradientRightR = 0.0 - ReadEEPROM(ELCompensateAddress + 1) / 10.0;
	}

	AzMidLimitF 	= 0xff;		   	//2010-7-7
	AZLeftLimitF 	= 0xff;			//
	AZRightLimitF 	= 0xff;			//这三个量都是电机限位标志，目前看来只有值为1的时候表示相应的限位故障，其他值都是正常状态

	Delay(1000);											// 优化时间，2010/4/19
	WriteEEPROM(1, AbnormalFlagAddress);					//保存不正常关机标志


	SSrcStarKbS = 27500.0;
	SharpRecFreq = 1135.0;
	SetSharpFreq(SharpRecFreq, SSrcStarKbS);

	GetPara();
	GetBaseStarPara();
	Delay(1000);											//获取对星参数
																
	TestSelf();												//自检																
	StationAz = AZ180;										//屏蔽罗盘
	Delay(1000);
	ReadGradient();
//	StationEl = GradientY;
//	MotorCtrl(UPDOWN, 35.0, GetElS1());
	/*2010-7-16加入对俯仰角度的判断，主要是防止人为手动将天线摇起*/
	if(GradientNormal == 1 && Abnormal == 0 && GradientY < 25.0)	//如果倾斜仪正常,并且正常开机
	{
		StationEl = GradientY;
		if(StationEl < 0)
	   	{
			WriteEEPROM('-', StoreELSignAddress);			//存储初始的斜坡角度		
			WriteEEPROM(0.0 - StationEl, StoreElDownAddress);//存储初始的斜坡角度
		}
		else
		{
	   		WriteEEPROM('+', StoreELSignAddress);			//存储初始的斜坡角度		
	   		WriteEEPROM(StationEl, StoreElDownAddress);		//存储初始的斜坡角度
		}
		ELUPlimit = StationEl - 14.0 + 80.0;				//再次得到俯仰极限
		ELDOWNlimit = StationEl - 14.0 + ELDOWNlimitDef;								
	}

	//2010-7-29加入
	Delay(3000);
	if((ReadEEPROM(StoreELSignAddress) != '-') && (ReadEEPROM(StoreELSignAddress) != '+'))
	{
	   	WriteEEPROM('+', StoreELSignAddress);			//存储初始的斜坡角度		
	   	WriteEEPROM(0, StoreElDownAddress);				//存储初始的斜坡角度		
	}



	if(GradientNormal == 1)								//如果倾斜仪正常
	{

		StationEl = GradientY;
/* 2016/3/18 插入，测试天线俯仰下限是否会随着天线摆放角度变化而变化 */
		if(StationEl < ELDOWNlimit)
		{
			Delay(10);
			MotorFun(ELUP, MOTORSTOP, GetElS1());
			while(ELDOWNlimit > StationEl)
			{
				ReadGradient();
				StationEl = GradientY;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
		}
		TimeTest(2);

		if(StationEl < 40.0)
		{
			Delay(10);
			MotorFun(ELUP, MOTORSTOP, GetElS1());
			while(40.0 > StationEl)
			{
				ReadGradient();
				StationEl = GradientY;
				watch();
			}
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			TimeTest(2);
			AzLimitTest(); 									//2010-7-8加入对限位的测试；
			GotoAzMid();
			BianBanFlag = 0;
			SetBianBan();									//等待装边瓣		
		}
		else
		{
			AzLimitTest();
			GotoAzMid();
		}			
	}//end (GradientNormal == 1)
	else
	{
		Delay(10);
		StationEl = -73.0;
		MotorCtrl(UPDOWN, 40.0, GetElS1());
	}//end (GradientNormal != 1)

   	BaseStarFlagThree = 0;									//使用参考卫星标志位
	EnStorSrcPara = 1;
	status = SEARCHREADY;
	return;
}


/***********************************************************************
*
* 函数原型：void GetParaEn(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线准备搜索前,确定对目标卫星还是参考卫星
*
***********************************************************************/
static void GetParaEn(void)
{
	/*保存目标卫星参数*/
	ParaProtect();
	if(EnStorSrcPara == 1)							//保存目标卫星参数
	{
		StorSrcPara();
		EnStorSrcPara = 0;
	}

	/*
	 * 如果监控选用了参考卫星，则优先使用监控给的参考卫星
	 */
	if(BaseStarFlagThree == 2)
	{
		EnBaseStar = 2;	   //从参考卫星二开始，监控软件给的参考卫星
		BaseStarFlagThree = 0;
	}
	else
	{
		EnBaseStar = 1;	
	}

	/*
	 * 是否使用监控软件的参考卫星
	 */
	if(EnBaseStar == 2)
	{
		TestBaseStarPara();
		SatLongS 			= BaseStarLONG;
		StationPloModeS 	= BaseStarPol;
		ReceiverKindFlagS 	= BaseStarRType;
		SrcStarKbS 			= BaseStarKbS;
		XinBiaoRecFreqS 	= BaseStarFreq;
		SharpRecFreqS		= BaseStarFreq;
		ThresholdS			= Threshold;
		SatLongEastFlagS 	= BaseStarLONGEF;
	}
	else
	{
		SatLongS 			= SatLong;
		StationPloModeS 	= StationPloMode;
		ReceiverKindFlagS 	= ReceiverKindFlag;
		SrcStarKbS 			= SSrcStarKbS;
		XinBiaoRecFreqS 	= XinBiaoRecFreq;
		SharpRecFreqS		= SharpRecFreq;
		ThresholdS			= Threshold;
		SatLongEastFlagS 	= SatLongEastFlag;		
	}			
}


/***********************************************************************
*
* 函数原型：void AntennaReadySearch(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线准备搜索，到理论角附近
*
***********************************************************************/
void AntennaReadySearch(void)
{
	int a;
	float j;
	float MotorLook;

	TestSelf();
	if(GPSNormal == 1 && NoMonitorF == 1)
	{
		StationLong  	 	= GPSLong;
		StationLat  	 	= GPSLat;
		StationEastFlag  	= GPSEastFlag;
		StationNorthFlag 	= GPSNorthFlag;
		EnStorSrcPara 		= 1; //2009/10/14
	}

	GetParaEn(); 		//2009/6/12得到要对卫星参数并置相关标志位

	FindAnt(StationLong, StationLat, SatLongS, StationPloModeS);	//对星计算，得到理论值	
//	FindAnt(StationLong, StationLat, 113, H);	//对星计算，得到理论值															   
//	PolarAngleInit();
	GotoPolarAngle(StationPolCal);									//到目标极化

	
	if(ReceiverKindFlagS == XINBIAOREC)
	{
//		SetXinBiaoFreqKC(XinBiaoRecFreqS);							//设置信标频率
		SetXinBiaoFreqKC(1300.0);							//设置信标频率	
	}	
	else
	{
		SharpRecNormalFlag = SetSharpFreq1(SharpRecFreqS, SrcStarKbS);	
	} 

	/****************************************************		
	*走到理论俯仰
	****************************************************/
	TimeTest(1);
	ReadGradient();
	if(GradientNormal == 1)							//如果倾斜仪正常
	{
		if(GradientY - StationEl < 8.0 && GradientY - StationEl > -8.0)
		{
			StationEl = GradientY;
		}
	}
	StationElTemp = StationElCal;
	if(StationElCal > ELUPlimit)  					//保护理论俯仰值
	{
		StationElTemp = ELUPlimit;
	}
	if(StationElCal < ELDOWNlimit)
	{
		StationElTemp = ELDOWNlimit;
	}

	if(StationEl < StationElTemp)					//到理论俯仰位置
	{
		MotorFun(ELUP, MOTORSTOP, GetElS1());
		while(StationEl < StationElTemp)
		{
			watch();
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12再次复位用
			{
				AntReadySearchF = 1;
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
				return;	
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}
	else
	{
		MotorFun(ELDOWN, MOTORSTOP, GetElS1());
		while(StationEl > StationElTemp)
		{
			watch();
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12再次复位用
			{
				AntReadySearchF = 1;
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
				return;	
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}//if(StationEl < StationElTemp)

	/****************************************************		
	*得到噪声
	*走到理论方位加上右侧30度时
	****************************************************/
	TimeTest(2);
	a = 1;
	AGCNoise = GetAGC();
	j = StationAz;
	MotorLook = StationAzCal + AZSearchRange;		//找到右边界
	if(MotorLook > AZ180 + AZlimit)
	{											    //方位保护
		MotorLook = AZ180 + AZlimit;
	}
	 
	if(StationAz < MotorLook)						//方位走到目标
	{
		MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
		while(StationAz < MotorLook)
		{
			watch();
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12再次复位用
			{
				AntReadySearchF = 1;
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
				return;	
			}
			if(StationAz - j > 3.0)
			{ 
				AGCNoise += GetAGC();
				j = StationAz;
				a++;
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}
	else
	{
		MotorFun(MOTORSTOP, AZLEFT, GetAzS2());
		while(StationAz > MotorLook)
		{
			watch();
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12再次复位用
			{
				AntReadySearchF = 1;
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
				return;	
			}
			if(j - StationAz > 3.0)
			{
				AGCNoise += GetAGC();
				j = StationAz;
				a++;
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}
	AGCNoise /= a;
	AGC = GetAGC();
	if(AGCNoise < AGCNoiseNor)
	{
		AGCNoise = AGC;
	}

	TimeTest(2);
	status = SEARCHING;

	if(ResetFlag == 1)								//如果复位标志为高
	{
		ReturnSet();								//返回天线设置	
		ReturnResetSuccess();						//返回复位成功
		ResetFlag = 0;								//清除复位标志
	}		
	return;
}


/***********************************************************************
*
* 函数原型：void AntennaTimeSearch(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线搜索
*
***********************************************************************/
void AntennaTimeSearch(void)
{
	float tempUP, tempDOWN;
	float LeftEdgeAz, RightEdgeAz, UpEdgeEl, DownEdgeEl;	//上下左右边界
	UINT8 DirFlag;							  				//判断方向  ，0左1上2右3下
	UINT8 r;
	UINT16 uiLockStatus;
	UINT16 FlagSpeed = 0;
	UINT8 CountHoop = 0;									//确定上下五度
	float tempCH = 0.0;										//2009/5/21
	float T1, T2, T3;


	/*确定左边界*/
	LeftEdgeAz = StationAzCal - AZSearchRange;
	if(LeftEdgeAz < AZ180 - AZlimit)
	{
		LeftEdgeAz = AZ180 - AZlimit;
	}
	
	/*确定右边界*/
	RightEdgeAz = StationAzCal + AZSearchRange;
	if(RightEdgeAz > AZ180 + AZlimit)
	{
		RightEdgeAz	= AZ180 + AZlimit;
	}
	
	/*确定上边界*/
	UpEdgeEl = StationElCal + ELSearchRange;
	if((UpEdgeEl > ELUPlimit) || (UpEdgeEl < ELDOWNlimit))
	{
		UpEdgeEl = ELUPlimit;
	}
	
	/*确定下边界*/ 
	DownEdgeEl = StationElCal - ELSearchRange;
	if(DownEdgeEl < ELDOWNlimit || (DownEdgeEl > ELUPlimit))
	{
		DownEdgeEl = ELDOWNlimit;
	}
			
	DirFlag  = 0;
	tempDOWN = StationElTemp;
	tempUP	 = StationElTemp;
	T1 = StationEl;	   //2009/9/1
	T2 = StationEl;
	T3 = StationAz;   	//用于判断方位是否大于某一度数后再进行俯仰补尝

	while(CountHoop < 6)
	{
		if(fabs(StationAz - T3) > 4.5)				//2009/10/12差五度才进行补尝
		{							
			if(GradientNormal == 1)							//如果倾斜仪正常  	//2009/5/21加入
			{
				ReadGradient();
				if(GradientY - StationEl < 12.0 && GradientY - StationEl > -12.0)
				{
					T1 = GradientY;
					ReadGradient();
					T2 = GradientY;
					T3 = StationAz;
					StationEl = (T1 + T2) / 2.0;
				}
			}
		}
		
		if(DirFlag == 0)
		{
			if(fabs(tempDOWN - StationEl) > 1.2)
			{
				MotorCtrl(UPDOWN, tempDOWN, SpeedSearchEl);	
			}
		}

		if(DirFlag == 2)
		{
			if(fabs(StationEl - tempUP) > 1.2)
			{
				MotorCtrl(UPDOWN, tempUP, SpeedSearchEl);
			}
		}

		if(DirFlag == 0) 												//天线向左搜索
		{
			if(StationAz > LeftEdgeAz)
			{
				if(StationAz > RightEdgeAz - 2.0)
				{
					MotorFun(MOTORSTOP, AZLEFT, SpeedSearchAzS);
				}
				else if(StationAz < LeftEdgeAz + 2.0)
				{
					MotorFun(MOTORSTOP, AZLEFT, SpeedSearchAzS);
				}
				else
				{
					MotorFun(MOTORSTOP, AZLEFT, GetAzS1());
				}
				if(FlagSpeed == 1)	
				{
					MotorFun(MOTORSTOP, AZLEFT, 0xD000);
				}
			}
			else
			{
				CountHoop++;    										//用来计圈数
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
				DirFlag = 1;
				tempCH += 1.0;
				tempUP = StationElTemp + tempCH;					//2009/5/21
				if(tempUP > UpEdgeEl)									//保护，天不会出上限
				{
					tempUP = UpEdgeEl;
				}	
			}	
		}//end if(DirFlag == 0) 
		
		if(DirFlag == 1) 												//天线向上搜索
		{
			if(StationEl < tempUP)
			{
				MotorFun(ELUP, MOTORSTOP, SpeedSearchEl);
			}
			else
			{
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchAzS);
				DirFlag = 2;
			}
		}//end if(DirFlag == 1)
		
		if(DirFlag == 2) 												//天线向右搜索
		{
			if(StationAz < RightEdgeAz)
			{
				if(StationAz > RightEdgeAz - 2.0)
				{
					MotorFun(MOTORSTOP, AZRIGHT, SpeedSearchAzS);
				}
				else if(StationAz < LeftEdgeAz + 2.0)
				{
					MotorFun(MOTORSTOP, AZRIGHT, SpeedSearchAzS);
				}
				else
				{
					MotorFun(MOTORSTOP, AZRIGHT, GetAzS1());
				}
				if(FlagSpeed == 1)	
				{
					MotorFun(MOTORSTOP, AZRIGHT, 0xD000);
				}
			}
			else
			{
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
				DirFlag = 3;
				tempDOWN = StationElTemp - tempCH;
				if(tempDOWN < DownEdgeEl)								//保护，天线不会出下限
				{
					tempDOWN = DownEdgeEl;
				}
			}	
		}//end if(DirFlag == 2)
		
		if(DirFlag == 3) 												//天线向下搜索
		{
			if(StationEl > tempDOWN)
			{
				MotorFun(ELDOWN, MOTORSTOP, SpeedSearchEl);
			}
			else
			{
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchAzS);
				DirFlag = 0;
			}
		}//end if(DirFlag == 2)

		AGC = GetAGC();
		if((AGC > AGCNoise + ThresholdS * 0.9) && (AGCNoise > AGCNoiseNor))
		{	
			if((ReceiverKindFlagS == SHARPREC) && (SrcStarKbS - 10.0 > 0.0))
			{
				FlagSpeed = 1; 	
				Delay(100);

				r = A2108_IDVBSxRx_GetLockStatus(&uiLockStatus, pA2108Chip);
				if ((0 == r)&&(1 == uiLockStatus))
				{
					MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);	
					status = TRACKING;
					AntennaTracking();
				}

			}
			else
			{	
				if((XinBiaoFlagNum == TRUE) && (ReceiverKindFlagS != SHARPREC))
				{
					GetXinBiaoVorX();
					if(XinBiaoVorX == 'V')
					{
						MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
						status = TRACKING;
						AntennaTracking();
					}
				}
				else
				{
					MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
					status = TRACKING;
					AntennaTracking();
				}
			}						
		}
		else
		{
			FlagSpeed = 0;	
		}

		/*LNB刚上电用!!!不同接收机可能AGCNoiseNor不同, 可以在初始化时：AGCNoiseNor = -1.0*/
/*		if((AGCNoise < AGCNoiseNor) && (AGC > AGCNoiseNor))
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
			GotoAzMid();
			status = SEARCHREADY;
			TimeTest(1);
			return;
		}
 */
		watch();
		if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
			return;
		}	
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
	status = SEARCHREADY;
	TimeTest(1);
	return;
}


/***********************************************************************
*
* 函数原型：void GoToObjStar(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线从参考卫星到目标卫星的过程
*
***********************************************************************/
static void GoToObjStar(void)
{
	float AzTemp;
	float ElTemp;
	UINT8 TempF;

	TempF = SatLongEastFlagS;
	SatLongEastFlagS = SatLongEastFlag;
	FindAnt(StationLong, StationLat, SatLong, StationPloMode);
	AzTemp = StationAzCal;
	ElTemp = StationElCal;	

	GotoPolarAngle(StationPolCal);

	if(ReceiverKindFlag == XINBIAOREC)
	{
		ReceiverKindFlagS = XINBIAOREC;
		SetXinBiaoFreqKC(XinBiaoRecFreq);				//设置信标频率
	}	
	else
	{
		ReceiverKindFlagS = SHARPREC;
		SharpRecNormalFlag = SetSharpFreq1(SharpRecFreq, SSrcStarKbS);	
	}
	SatLongEastFlagS = TempF;
	FindAnt(StationLong, StationLat, SatLongS, StationPloModeS);

	AzTemp -= StationAzCal;
	ElTemp -= StationElCal;
	ElTemp += StationEl;

	AGC = GetAGC();	
   	AGCNoise = AGC;

	MotorCtrl(RIGHTLEFT, StationAz + AzTemp, SpeedSearchAzF);

	if(StationEl < ElTemp)						//到理论俯仰位置
	{
		MotorFun(ELUP, MOTORSTOP, GetElS1());
		while(StationEl < ElTemp)
		{
			ReadGradient();
			if(GradientNormal == 1 && 			\
			   GradientY - StationEl < 8.0 && 	\
			   GradientY - StationEl > -8.0)		//如果倾斜仪正常
			{
				StationEl = GradientY;
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}
	else
	{
		MotorFun(ELDOWN, MOTORSTOP, GetElS1());
		while(StationEl > ElTemp)
		{
			ReadGradient();
			if(GradientNormal == 1 && 			\
			   GradientY - StationEl < 8.0 && 	\
			   GradientY - StationEl > -8.0)		//如果倾斜仪正常
			{
				StationEl = GradientY;
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}//if(StationEl < MotorLook)	
	watch();
	if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
	{
		return;
	}

	TackingUpDownRightLeft(3.0, 3.0, 0.1);
	watch();
	if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
	{
		return;
	}
	AGC = GetAGC();	
   	AGCNoise = AGC - ThresholdS * 1.1;
	return;
}


/***********************************************************************
*
* 函数原型：void AntennaTracking(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线准备跟踪
*
***********************************************************************/
void AntennaTracking(void)
{
	UINT8 DIR_Flag;					//方向标志,取值0,1,2,3,分别对应左,上,右,下
	UINT8 count;					//用来记一圈AGC的个数
	UINT8 Trackcount;				//跟踪圈数
	float AGCPrev = 0.0;			//上一次采样值
	float TrackingStep = 0.1;		//方位跟踪步长
	float AverageAGC = 0.0;			//跟踪状态的每圈平均AGC值

	DIR_Flag 	= 0;
	AGCPrev  	= GetAGC();
	AverageAGC 	= AGCPrev;
	count	 	= 1;
	Trackcount  = 0;
	while(1)
	{
		if(DIR_Flag == 0)			//左跟踪
		{	
			MotorCtrl(RIGHTLEFT, StationAz - TrackingStep, SpeedTRACKING);
			AGC = GetAGC();
			AverageAGC += AGC;
			count++;
			if(AGC < AGCPrev)
			{
				DIR_Flag = 1;
			}
			AGCPrev = AGC;
		}
		
		if(DIR_Flag == 1)
		{
			MotorCtrl(UPDOWN, StationEl + TrackingStep, SpeedTRACKING - SElspeed);
			AGC = GetAGC();
			AverageAGC += AGC;
			count++;
			if(AGC < AGCPrev)
			{
				DIR_Flag = 2;
			}
			AGCPrev = AGC;
		}
		
		if(DIR_Flag == 2)			//右跟踪
		{
			MotorCtrl(RIGHTLEFT, StationAz + TrackingStep, SpeedTRACKING);
			AGC = GetAGC();
			AverageAGC += AGC;
			count++;
			if(AGC < AGCPrev)
			{
				DIR_Flag = 3;
			}
			AGCPrev = AGC;
		}
		
		if(DIR_Flag == 3)
		{
			MotorCtrl(UPDOWN, StationEl - TrackingStep, SpeedTRACKING - SElspeed);
			AGC = GetAGC();
			AverageAGC += AGC;
			count++;
			AverageAGC /= count;
			count = 1;
			if(AverageAGC < AGCNoise + ThresholdS * 0.9)
			{
				status = SEARCHING;
				return;
			}
			if(Trackcount > 1)
			{
				TackingUpDownRightLeft(2.0, 2.0, 0.1);
				if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS) //Rill add 101202
				{
					return;
				}

				if(EnBaseStar != 1)
				{
					GoToObjStar();
					EnBaseStar = 1;
				}
				TrackingSucceed();
				if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
				{
					return;
				}
			}
			if(AGC < AGCPrev)
			{
				DIR_Flag = 0;
				Trackcount++;
			}
			AGCPrev = AGC;
		}
		watch();
		if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
		{
			return;
		}
	}
}


/***********************************************************************
*
* 函数原型：void AntennaManual(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线手动
*
***********************************************************************/
void AntennaManual(void)
{
	LEDLOCKLOST;	//2009/12/8
	LEDLOCK;

	while(1)
	{
		Delay(60000);
		status = MANUALMODE;
		watch();
		if(status == STORESTATUS || status == SEARCHREADY || status == SEARCHING)
		{
			LEDLOCKLOST;	//2009/12/8
			break;
		}		
	}
	if(status == SEARCHING)
	{
		AGC = GetAGC();
		if((AGC > AGCNoise + ThresholdS * 0.9) && (AGCNoise > AGCNoiseNor))
		{
			LEDLOCKLOST;	//2009/12/8
			status = TRACKING;					
		}
	}
	return;
}

/***********************************************************************
*
* 函数原型：void AntennaStore(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线收藏
*
***********************************************************************/
void AntennaStore(void)
{
	float temp;

//	GotoPolarAngle(90.0);									//极化回零
	MotorCtrl(UPDOWN, 35.0, GetElS1());						//俯仰到35度
//	MotorCtrl(UPDOWN, 35.0, 0xF000);	

	if(AzMidLimit != MotorLimitFlag)						//到中间限位
	{
		TimeTest(2);
		GoAM();
		StoreStatus1 = 2;
	}
	else if(StoreStatus1 == 0)  //2008-07-31 收藏不用到中间限位以外
	{
		StoreStatus1 = 2;
		MotorCtrl(RIGHTLEFT, AZ180 - 15.0, GetAzS2());
		TimeTest(2);
		GoAM();
	}
	StoreStatus1 = 0;
	UninstallBianbanFlag = 0;
	OpenTimer0Interrupt();
	while(!UninstallBianbanFlag && 	\
		   OverflowT0 < 1200 && 	\
		   ChangeKit != 0)										//查询边瓣是否卸载
	{
		Delay(3000);
		SendSJ();
		Delay(3000);	
		watch();
	}
	CloseTimer0Interrupt();
	UninstallBianbanFlag = 0;

	if(AZDir == AZLEFT)											//如果从右边过来，再继续转,因为中间限位较偏
	{
		temp = (float)ReadEEPROM(rightONE) / 10.0;
		temp = 4;
		if(temp < 10.0)
		{
			MotorCtrl(RIGHTLEFT, AZ180 - temp, GetAzS2());
		}
		else
		{
	 		MotorCtrl(RIGHTLEFT, AZ180 - 2.0, GetAzS2());
		}
	}
	else
	{
		temp = (float)ReadEEPROM(rightTWO) / 10.0;
		temp = 5;
		if(temp < 10.0)
		{
			MotorCtrl(RIGHTLEFT, AZ180 + temp, SpeedSearchAzF);
		}
		else
		{
			MotorCtrl(RIGHTLEFT, AZ180 + 2.0, SpeedSearchAzF);
		}
	}

	ReadGradient();
	if(GradientNormal == 1)
	{
		if(GradientY - StationEl < 8.0 && GradientY - StationEl > -8.0)
		{
			StationEl = GradientY;
		}
	}

	if(ReadEEPROM(StoreELSignAddress) == '-')
	{
		temp = 0.0 - ReadEEPROM(StoreElDownAddress);
	}
	else if(ReadEEPROM(StoreELSignAddress) == '+')
	{
		temp = ReadEEPROM(StoreElDownAddress);	
	}
	else
	{
		temp = 0.0;
	}

	ELDOWNlimit	= -120.0;
	if(temp > 20.0)	//2009/8/6由于倾斜仪的问题（最大60度）
	{
		temp = 20.0;	
	}
	if(temp < -30.0)
	{
		temp = -30.0;
	}
	if(temp < StationEl)
	{	
		MotorCtrl(UPDOWN, temp, GetElS1());						//下到斜坡的角度
	}
	WriteEEPROM(0, AbnormalFlagAddress);						//改到这时候保存正常关机标志

/*2008-11-14加入，收藏上下可调*/
	TimeTest(5);
	temp = (float)ReadEEPROM(rightSIX);
	if(temp > 99.0)
	{
		temp = 90.0; 
	}
	
	if(temp < 50.0)
	{
		temp = 50.0;	
	}

	MotorCtrl(UPDOWN, StationEl - temp, GetElS1());	

	while(ChangeKit != 0)
	{	
		Delay(6000); 				//about 3ms
	}
	Delay(60000);					//about 3ms
	while(ChangeKit == 0)
	{
		;
	}
	status = INIT;
	BianBanFlag = 0;
	Abnormal = 0;
	ELDOWNlimit = ELDOWNlimitDef;

	return;
}


/***********************************************************************
*
* 函数原型：void TrackingSucceed(void)
*
* 入口参数：上下DeltaEl,左右DeltaAz
*
* 出口参数：无
*
* 功能描述：天线上下左右跟踪最大值,采用递推平均滤波法
*
***********************************************************************/
static void TackingUpDownRightLeft(float DeltaAz,float DeltaEl, float St)
{
 	UINT16 i, j, k;
	UINT16 AzNumbers, ElNumbers;				//动0.1度的个数
	float Filters[MaxFilterLen];				//滤波器存储agc
	float MaxStationAz_1;
	float MaxStationAz_2;
	float MaxStationEl_1;
	float MaxStationEl_2;
	float MaxStationAz;
	float MaxAverageAGC;
	float SumAGC = 0;
	AzNumbers = DeltaAz / St;
	ElNumbers = DeltaEl / St;

	MotorCtrl(RIGHTLEFT, StationAz - DeltaAz, SpeedTRACKING);
	for(i = 0; i < MaxFilterLen; i++)
	{
		 Filters[i] = GetAGC();
	}											//初始化滤波器

	MaxStationAz_1 = StationAz;					//初始化最大值
	MaxAverageAGC = Filters[0];					//初始化平均agc
	j = 0;										//初始化起始点
	for(i = 0; i < AzNumbers * 2; i++)
	{
	   MotorCtrl(RIGHTLEFT, StationAz + St, SpeedTRACKING);	//再向右逐步走0.1
	   watch();
	   if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
			return;
	   Filters[j++] = GetAGC();							//引进一个新的采样值
	   if(j == MaxFilterLen)
	   		j = 0;										//循环
	   for(k = 0; k < MaxFilterLen; k++)
		  SumAGC += Filters[k];							//算术和
	   if((SumAGC / MaxFilterLen) > MaxAverageAGC)		//判断最大值在哪
	   {
	   	  MaxStationAz_1 = StationAz;
		  MaxAverageAGC = (SumAGC / MaxFilterLen);
	   }
	   SumAGC = 0;										//算术平均清零
	}


	for(i = 0; i < MaxFilterLen; i++)
	{
		 Filters[i] = GetAGC();
	}
	MaxStationAz_2 = StationAz;
	MaxAverageAGC = Filters[0];
	j = 0;
	for(i = 0; i < 2 * AzNumbers; i++)
	{
	   MotorCtrl(RIGHTLEFT, StationAz - St, SpeedTRACKING);		//再向左逐步走0.1
	   watch();
	   if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
			return;
	   Filters[j++] = GetAGC();
	   if(j == MaxFilterLen)
	   		j = 0;
	   for(k = 0;k < MaxFilterLen; k++)
		  SumAGC += Filters[k];
	   if((SumAGC / MaxFilterLen) > MaxAverageAGC)
	   {
	   	  MaxStationAz_2 = StationAz;
		  MaxAverageAGC = (SumAGC / MaxFilterLen);
	   }
	   SumAGC = 0;
	}

	MaxStationAz = (MaxStationAz_1 + MaxStationAz_2) / 2;

	MotorCtrl(RIGHTLEFT, MaxStationAz, SpeedTRACKING);			//回到最大点方位
	Delay(60000);


	MotorCtrl(UPDOWN, StationEl - DeltaEl, SpeedTRACKING - SElspeed);			//先向下走2度
	for(i = 0;i < MaxFilterLen; i++)
	{
		 Filters[i] = GetAGC();
	}
	MaxStationEl_1 = StationEl;
	MaxAverageAGC = Filters[0];
	j = 0;
	for(i = 0;i < 2 * ElNumbers; i++)
	{
	   MotorCtrl(UPDOWN, StationEl + St, SpeedTRACKING - SElspeed);				//再向上逐步走0.01
	   watch();
	   if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
			return;
	   Filters[j++] = GetAGC();
	   if(j == MaxFilterLen)
	   		j = 0;
	   for(k = 0; k < MaxFilterLen; k++)
		  SumAGC += Filters[k];
	   if((SumAGC / MaxFilterLen) > MaxAverageAGC)
	   {
	   	  MaxStationEl_1 = StationEl;
		  MaxAverageAGC = (SumAGC / MaxFilterLen);
	   }
	   SumAGC = 0;
	}
//	MotorCtrl(UPDOWN, MaxStationEl_1, SpeedTRACKING);


	for(i = 0;i < MaxFilterLen; i++)											  
	{
		 Filters[i] = GetAGC();
	}
	MaxStationEl_2 = StationEl;
	MaxAverageAGC = Filters[0];
	j = 0;
	for(i = 0;i < 2 * ElNumbers; i++)
	{
	   MotorCtrl(UPDOWN, StationEl - St, SpeedTRACKING - SElspeed);		//再向上逐步走0.01
	   watch();
	   if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
			return;
	   Filters[j++] = GetAGC();
	   if(j == MaxFilterLen)
	   		j = 0;
	   for(k = 0; k < MaxFilterLen; k++)
		  SumAGC += Filters[k];
	   if((SumAGC / MaxFilterLen) > MaxAverageAGC)
	   {
	   	  MaxStationEl_2 = StationEl;
		  MaxAverageAGC = (SumAGC / MaxFilterLen);
	   }
	   SumAGC = 0;
	}
	MotorCtrl(UPDOWN, (MaxStationEl_1 + MaxStationEl_2) / 2.0, SpeedTRACKING - SElspeed);

	return;
}


/***********************************************************************
*
* 函数原型：void TrackingSucceed(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线跟踪上卫星,锁定状态
*
* 修    改：加入连续十秒有遮挡情况下的反应,连续的10秒遮挡则进入准备搜索
*
*           修改人：杨淳雯
*
*           修改时间：2008-11-04
*
***********************************************************************/
static void TrackingSucceed(void)
{
	float temp_agc;
	UINT8 i;
	UINT16 DelTemp1;

	LockFlag = 1;	
	LEDCLOSE;	   	
	while(1)
	{
		LEDGREEN;			//2009/10/12
		temp_agc = 0;
		for(i = 0; i < 3; i++)
		{
			Delay(60000);
			watch();
			if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
			{
				LockFlag = 0;
				LEDCLOSE;
				LEDRED;			//2009/10/12
				return;
			}
			temp_agc += GetAGC();
		}
		AGC = temp_agc / (float)i;
	
		/****************************************************		
		*如果有连续的10秒遮挡
		*则进入准备搜索
		****************************************************/
		if(AGC < AGCNoise + 0.8 * ThresholdS)
		{
			DelTemp1 = 0;
			while(AGC < AGCNoise + 0.8 * ThresholdS)
			{
				AGC = GetAGC();
				if(/*(AGC < AGCNoiseNor) || */(DelTemp1 > 350 - 1))//Rill delete 101202
				{
					status = SEARCHREADY;
					LockFlag = 0;
					LEDCLOSE;
					LEDRED;			//2009/10/12
					return;	
				}
				DelTemp1++;
				watch();
				if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
				{
					LockFlag = 0;
					LEDCLOSE;
					LEDRED;			//2009/10/12
					return;
				}
			}//end while(AGC <  AGCNoise + 0.8 * Threshold)
		}//end 10 sec. if(AGC <  AGCNoise + 0.8 * Threshold)
	}//end while(1)
}


static void LEDTest(void);
static void GradientTest(void);
static void MotorUp(void);
static void TestXianWei(void);
static void TestPolar(void);
static void TestXinBiao(void);
static void TestGPS(void);
/*************************测试用(2009-3-9)*********************/
static void AntTestFun(void)
{
	while(1)
	{
		LEDTest();//测试LED
		GradientTest();//测试倾斜仪
		MotorUp();//测试俯仰电机
		TestXianWei();//测试方位电机与限位
		TestPolar();//测试极化
		TestXinBiao();//测试信标
		TestGPS();//测试GPS
	}
}

#define TIME3 1

static void LEDTest(void)
{
	int a, b, c, d, i;
	float j;
	static int MAkitFlag = 0;
	ReceiverKindFlag = XINBIAOREC;
	TimeTest(1);
	/*清限位故障标志*/
	AzMidLimitF 	= 0xff;		   	//2010-7-7
	AZLeftLimitF 	= 0xff;
	AZRightLimitF 	= 0xff;

	WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);
	WriteEEPROM(AZLeftLimitF, AZLeftLimitFAddr);
	WriteEEPROM(AZRightLimitF, AZRightLimitFAddr);

	StationLong = 117.5;
	StationLat = 28.2;
	a = (int)(StationLong);
	b = (StationLong - (float)a) * 100.0;
	WriteEEPROM(a, StationLongAddress);	
	WriteEEPROM(b, StationLongAddress + 1);
	
	a = (int)(StationLat);
	b = (StationLat - (float)a) * 100.0;
	WriteEEPROM(a, StationLatAddress);		
	WriteEEPROM(b, StationLatAddress + 1);
	SatLong = 138.0;
	a = (int)(SatLong);
	b = (SatLong - (float)a) * 100.0;
	WriteEEPROM(a, SatLongAddress);				
	WriteEEPROM(b, SatLongAddress + 1);
	XinBiaoRecFreq = 951.0;
	i = (int)(XinBiaoRecFreq);
	j = (XinBiaoRecFreq - i)*1000;
	a = i / 100;								//a=14
	b = i % 100;								//b=49
	c = j / 10;									//c=50
	d = ((int)j) % 10;							//d=0
	WriteEEPROM(a, SatXinBiaoFreqAddress);		//存储信标接收机的频率整数部分千位百位值
	WriteEEPROM(b, SatXinBiaoFreqAddress + 1);	//存储信标接收机的频率整数部分十位个位值
	WriteEEPROM(c, SatXinBiaoFreqAddress + 2);	//存储信标接收机的频率小数部分十分百分值
	WriteEEPROM(d, SatXinBiaoFreqAddress + 3);	//存储信标接收机的频率小数部分千分位值
	StationPloMode = V;
	WriteEEPROM(StationPloMode, StationPloModeAddress);
	TimeTest(1);
	WriteEEPROM(ReceiverKindFlag, ReceiverKindAddress);

	SatLong = 134.0;
	TimeTest(1);
	a = (int)(SatLong);
	b = (SatLong - (float)a) * 100.0;
	WriteEEPROM(a, BaseStarLONGAddr);				
	WriteEEPROM(b, BaseStarLONGAddr + 1);
	TimeTest(1);

	LEDCLOSE;
	LEDGREEN;
	LEDGREENOPEN;
	TimeTest(TIME3 * 4);
	for(i  = 0; i < 3; i++)
	{
		LEDCLOSE;
		LEDGREENCLOSE;
		LEDREDCLOSE;
		TimeTest(TIME3);
		LEDRED;
		LEDREDCOPEN;
		TimeTest(TIME3);
	}
	LEDREDCLOSE;
	TimeTest(TIME3);
	LEDGREEN;
	LEDGREENOPEN;
	TimeTest(TIME3 * 4);

/*2009/11/4*/
	Tempearter = 1;
	TimeTest(TIME3 * 40);
	Tempearter = 0;

	if(MAkitFlag == 0)
	{	
		while(MAkit == 1)
		{
		LEDREDCOPEN;
		}
		while(MAkit == 0)
		{
			LEDGREENOPEN;
		}
		MAkitFlag = 1;
	}
	LEDREDCOPEN;		
}

static void GradientTest(void)
{
	LEDCLOSE;
	LEDGREENCLOSE;
	LEDREDCLOSE;
	ReadGradient();
	
	if(GradientNormal != 1)
	{
		while(1)
		{
			LEDRED;
			Delay(40000);
			LEDCLOSE;
			Delay(40000);
		}
	}
}

static void MotorUp(void)
{
	ReadGradient();
	StationEl = GradientY;
	MotorFun(ELUP, MOTORSTOP, GetElS1());	  //天线向上到65度
	while(65.0 > StationEl)
	{
		ReadGradient();
		StationEl = GradientY;
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	TimeTest(TIME3);

	MotorFun(ELDOWN, MOTORSTOP, GetElS1());	  //天线向下到40度
	while(StationEl > 40.0)
	{
		;
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	TimeTest(TIME3);	
}

static void TestXianWei(void)
{
	AzMidLimitF 	= 0x5;		   	//2010-10-26
	AZLeftLimitF 	= 0x5;
	AZRightLimitF 	= 0x5;
	StationAz = 150.0;
	MotorCtrl(RIGHTLEFT, 270.0, GetAzS2());
	StationAzLimit = 270.0;

	if(AZRightLimit != MotorLimitFlag)
	{
		Delay(5000);
		if(AZRightLimit != MotorLimitFlag)
		{
			while(1)
			{
				LEDRED;
				Delay(40000);
				LEDCLOSE;
				Delay(40000);
			}		
		}
	}


	TimeTest(TIME3);
	MotorFun(MOTORSTOP, AZLEFT, GetAzS2());
	while(AzMidLimit != MotorLimitFlag && StationAz > 150.0)
	{
		;
	}
	MotorFun(MOTORSTOP, MOTORSTOP, GetAzS2());
	if(AzMidLimit != MotorLimitFlag)
	{
		Delay(5000);
		if(AzMidLimit != MotorLimitFlag)
		{
			while(1)
			{
				LEDRED;
				Delay(40000);
				LEDCLOSE;
				Delay(40000);
			}		
		}
	}
	TimeTest(TIME3);
	StationAz = 200.0;
	MotorCtrl(RIGHTLEFT, 90.0, GetAzS2());
	if(AZLeftLimit != MotorLimitFlag)
	{
		Delay(5000);
		if(AZLeftLimit != MotorLimitFlag)
		{
		while(1)
		{
			LEDRED;
			Delay(40000);
			LEDCLOSE;
			Delay(40000);
		}		
	}
	}
	TimeTest(TIME3);
	StationAz = 90.0;
	MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
	while(AzMidLimit != MotorLimitFlag)
	{
		;
	}
	MotorFun(MOTORSTOP, MOTORSTOP, GetAzS2());
	TimeTest(TIME3);

	StationAz = AZ180;							 //2016/3/15
	MotorCtrl(RIGHTLEFT, AZ180 + 5, GetAzS2());	 //2016/3/15   弥补回到中间限位后的偏差
	return;	
}
static void TestPolar(void)
{
	StationPol = GetComPolA();
	if(StationPol > 30.0 || StationPol < -30.0)
	{
		while(1)
		{
			LEDRED;
			Delay(40000);
			LEDCLOSE;
			Delay(40000);
		}
	}

	POLAR_LEFT;
	TimeTest(7);
	POLAR_STOP;

	if(StationPol > GetPolarAngle())
	{
		while(1)
		{
			LEDRED;
			Delay(40000);
			LEDCLOSE;
			Delay(40000);
		}
	}
	TimeTest(TIME3);
	POLAR_RIGHT;
	while(GetPolarAngle() /* - StationPol */ > 0.1)
	{
		;
	}
	POLAR_STOP;
	TimeTest(TIME3);
	return;
}
static void TestXinBiao(void)
{
	int i;

	MotorFun(ELDOWN, MOTORSTOP, GetElS1());	  //天线向下到40度
	while(StationEl > 30.0)
	{
		;
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);	
	SetXinBiaoFreqKC(XinBiaoRecFreq);
	if(!XinBiaoRecNormalFlag)
	{
		for(i  = 0; i < 10; i++)
		{
			TimeTest(TIME3 + TIME3);
			LEDRED;
			TimeTest(TIME3 + TIME3);
			LEDCLOSE;
			Delay(40000);
			LEDRED;
			Delay(40000);
			LEDCLOSE;
		}
	}
	TimeTest(TIME3);
	return;
}
static void TestGPS(void)
{
	int i;
	ReadGPS();
	if(!GPSNormal)
	{
		while(1)
		{
			LEDRED;
			Delay(40000);
			LEDCLOSE;
			Delay(40000);
		}
	}
	if(GPSNormal == 1)
	{
		for(i = 0; i < 10; i++)
		{
			Delay(40000);
			LEDGREEN;
			Delay(40000);
			LEDCLOSE;
		}		
	}
	TimeTest(TIME3);
}


/***********************************************************************
*
* 函数原型：void StorSrcPara(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：保存目标卫星参数
*
***********************************************************************/
static void StorSrcPara(void)
{
	int a, b, c, d, i;
	float j;

	/****************************************************
	*存储各个值 
	*以下存储地球站经度,存储方法如下
	*设经度为118.78，则a=118，b=78
	****************************************************/
	TimeTest(1);
	a = (int)(StationLong);
	b = (StationLong - (float)a) * 100.0;
	WriteEEPROM(a, StationLongAddress);	
	WriteEEPROM(b, StationLongAddress + 1);
	
	/****************************************************
	*以下存储地球站东经标志,存储方法如下 
	*存储0或者1
	****************************************************/
	WriteEEPROM(StationEastFlag, StationEastFlagAddress);
	
	/****************************************************
	*以下存储地球站纬度,存储方法如下
	*设经度为32.04，则a=32，b=4
	****************************************************/
	a = (int)(StationLat);
	b = (StationLat - (float)a) * 100.0;
	WriteEEPROM(a, StationLatAddress);		
	WriteEEPROM(b, StationLatAddress + 1);
	
	/****************************************************		
	*以下存储地球站北纬标志,存储方法如下
	*存储0或者1
	****************************************************/		
	WriteEEPROM(StationNorthFlag, StationNorthFlagAddress);
	
	/****************************************************		
	*以下存储卫星经度,存储方法如下
	*设经度为87.5，则a=87，b=50
	****************************************************/
	a = (int)(SatLong);
	b = (SatLong - (float)a) * 100.0;
	WriteEEPROM(a, SatLongAddress);				
	WriteEEPROM(b, SatLongAddress + 1);
	
	/****************************************************		
	*以下存储地球站极化方式,存储方法如下
	*存储0或者1
	****************************************************/	
	WriteEEPROM(StationPloMode, StationPloModeAddress);	
	

	/****************************************************		
	*以下存储卫星东西经标致,存储方法如下
	*存储0或者1
	****************************************************/	
	WriteEEPROM(SatLongEastFlag, SatLongEastFlagAddr);

	/****************************************************		
	*以下存储地球站接收机类型,存储方法如下
	*存储0或者1
	****************************************************/	
	WriteEEPROM(ReceiverKindFlag, ReceiverKindAddress);		 
	
	/****************************************************		
	*以下存储夏普接收机频率,存储方法如下
	*设频率为1051.000，则a=10，b=51,c=00,d=0
	****************************************************/
	i = (int)(SharpRecFreq);
	j = (SharpRecFreq - i) * 1000;
	a = i / 100;								//a=10
	b = i % 100;								//b=51
	c = j / 10;									//c=00
	d = ((int)j) % 10;							//d=0
	WriteEEPROM(a, SatSharpFreqAddress);		//存储夏普接收机的频率整数千位百位值
	WriteEEPROM(b, SatSharpFreqAddress + 1);	//存储夏普接收机的频率整数十位个位值
	WriteEEPROM(c, SatSharpFreqAddress + 2);	//存储夏普接收机的频率小数十分百分值
	WriteEEPROM(d, SatSharpFreqAddress + 3);	//存储夏普接收机的频率小数百分百分值
	WriteEEPROM(SharpThreshold * 10.0, SharpAgcThresholdAddress);		  //存储门限
	
	/****************************************************		
	*以下存储信标接收机频率,存储方法如下
	*设频率为1449.500，则a=11，b=49,c=50,c=50,d=0
	****************************************************/
	i = (int)(XinBiaoRecFreq);
	j = (XinBiaoRecFreq - i)*1000;
	a = i / 100;								//a=14
	b = i % 100;								//b=49
	c = j / 10;									//c=50
	d = ((int)j) % 10;							//d=0
	WriteEEPROM(a, SatXinBiaoFreqAddress);		//存储信标接收机的频率整数部分千位百位值
	WriteEEPROM(b, SatXinBiaoFreqAddress + 1);	//存储信标接收机的频率整数部分十位个位值
	WriteEEPROM(c, SatXinBiaoFreqAddress + 2);	//存储信标接收机的频率小数部分十分百分值
	WriteEEPROM(d, SatXinBiaoFreqAddress + 3);	//存储信标接收机的频率小数部分千分位值	
	WriteEEPROM(XinBiaoThreshold * 10.0, XBThresholdAddress);				  //存储门限
}
