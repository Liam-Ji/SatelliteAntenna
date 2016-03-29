/***********************************************************************
* 
* ʱ    �䣺2008-10-9
*
* �� �� ����antenna.c
*
* ��    ����AKD10P07(������)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
* 
* ����˵������������״̬����          
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

/*��̬����*/
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
* ����ԭ�ͣ�static void	TestBaseStarPara(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������������Ƿ���ȷ
*
***********************************************************************/
static void TestBaseStarPara(void)
{
	if((BaseStarLONG > 180.0) || (BaseStarLONG < 0.0))				//����0.0��180.0�ķ�Χ
	{
		BaseStarLONG = 134.0;										//����
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
* ����ԭ�ͣ�static void	SetBianBan(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������ȴ�װ�߰꺯��(2009-3-11)
*
***********************************************************************/
static void SetBianBan(void)
{
	OpenTimer0Interrupt();								//�򿪼�����,��120��
	  while(ChangeKit != 0 && 	\
			 !BianBanFlag &&   	\
			 OverflowT0 < 1200)							//����������������ұ߰�û�а�װ
	{
		Delay(10000);											
		SendKJ();										//����װ�߰�����
		Delay(10000);											
		watch();		
	}
	CloseTimer0Interrupt();	
}


/***********************************************************************
*
* ����ԭ�ͣ�static void	TestSelf(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ����������GPS��DVB���ջ�,�ű���ջ��Ĳ���
*
***********************************************************************/
static void TestSelf(void)
{
	ReadGradient();

	StationPol = GetComPolA();
//	if(StationPol < 185.0 && StationPol > -5.0)
//	{
//		PloarNormal = 1;					 //����������
//	}
//	else
//	{
//		PloarNormal = 0;					 //��������
//	}

	XinBiaoFlagNum = FALSE;
	GetXinBiaoVER();	 //2009/9/11�����µĶ��ű���ջ��汾�ŵ�
	AGC = GetAGC();
	SharpRecNormalFlag = SetSharpFreq1(SharpRecFreq, SSrcStarKbS);
	SetXinBiaoFreqKC(XinBiaoRecFreq);
	ReadGPS();
}


/***********************************************************************
*
* ����ԭ�ͣ�void GetBaseStarPara(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ȡ��׼���ǲ���
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
* ����ԭ�ͣ�void GetPara(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ȡ��������ı�Ҫ����
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
					  ReadEEPROM(SatLongAddress + 1) * 0.01;			//��ȡĬ�����Ǿ���

	StationPloMode	= ReadEEPROM(StationPloModeAddress);				//��ȡĬ�ϵ���վ������ʽ
	
	ReceiverKindFlag= ReadEEPROM(ReceiverKindAddress);					//�ϴ�ѡ��Ľ��ջ����
	
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

	SharpThreshold	= ReadEEPROM(SharpAgcThresholdAddress) * 0.1;		//���޴洢����10��ֵ

	SSrcStarKbS		= ReadEEPROM(SSrcStarKbSAddr) * 10000.0 + \
					  ReadEEPROM(SSrcStarKbSAddr + 1) * 100.0 +	\
					  ReadEEPROM(SSrcStarKbSAddr + 2);

	SatLongEastFlag = ReadEEPROM(SatLongEastFlagAddr);


  	if(!((XinBiaoRecFreq > 899.0) && (XinBiaoRecFreq < 1760.1)))		//KLR3000���ջ�����Ƶ��Χ(950.0~1750.0 MHZ)
	{
		XinBiaoRecFreq = 1449.015;										//ˮƽ����Ƶ��
	}

	if(!((SharpRecFreq > 899.0) && (SharpRecFreq < 2160.1)))			//SHARP���ջ�����Ƶ��Χ(950.0~2150.0 MHZ)
	{
		SharpRecFreq = 1062.0;											//ˮƽ����Ƶ��
	}
}


/***********************************************************************
*
* ����ԭ�ͣ�void ParaProtect(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ����������1���Ե���վ��γ�ȣ�Ŀ�����Ǿ��ȣ����ջ�����Ƶ�ʣ����м�⣻
*
*		    2�����ݲ��Ϸ��򱣻��ɱ���λ�ö���������105.5�������
*
*		    3�����ݵ�ǰ���ջ�������ж�ѡ����ֽ��ջ�
*
***********************************************************************/
static void ParaProtect(void)
{
	if(!(StationLong < 180.0 && StationLong > 0.0) || \
		!(StationLat > 0.0 && StationLat < 90.0))				//����0.0��180.0�ķ�Χ��γ��0.0��90.0�ķ�Χ
	{
		StationLong = 116.46;									//��������116.46
		StationLat  = 39.92;									//����γ��39.92
	}	
	
	if(!(SatLong < 180.0 && SatLong > 0.0))						//����0.0��180.0�ķ�Χ
	{
		SatLong = 105.5;										//����
	}

	if(!(XinBiaoThreshold < 9.9 && XinBiaoThreshold > 0.0))
	{
		XinBiaoThreshold = 0.2;									//�ű���ջ����ޱ�������Ϊ0.3
	}

	if(!(SharpThreshold < 9.9 && SharpThreshold > 0.0))
	{
		SharpThreshold = 0.2;									//���ս��ջ����ޱ�������Ϊ0.3
	}

  	if(!((XinBiaoRecFreq > 899.0) && (XinBiaoRecFreq < 1760.1)))//KLR3000���ջ�����Ƶ��Χ(950.0~1750.0 MHZ)
	{
		XinBiaoRecFreq = 1449.015;								//ˮƽ����Ƶ��
		StationPloMode = H;
	}

	if(StationPloMode > V)										//������ʽ2009-3-12�޸�
	{
		StationPloMode = H;	
	}

	if(!((SharpRecFreq > 899.0) && (SharpRecFreq < 2160.1)))	//SHARP���ջ�����Ƶ��Χ(950.0~2150.0 MHZ)
	{
		SharpRecFreq = 1062.0;									//ˮƽ����Ƶ��
		StationPloMode = H;
	}

	if(ReceiverKindFlag == XINBIAOREC)							//����ǰѡ��Ϊ�ű���ջ�
	{
		if(XinBiaoRecNormalFlag == 1)							//�ű���ջ�����
		{
			Threshold = XinBiaoThreshold;						//ѡ���ű�����
		}
		else
		{
			ReceiverKindFlag = SHARPREC;
			Threshold = SharpThreshold;							//ѡ����������
		}
	}
	else
	{
		if(XinBiaoRecNormalFlag == 1 && SharpRecNormalFlag == 0)
		{
			ReceiverKindFlag = XINBIAOREC;
			Threshold = XinBiaoThreshold;						//ѡ���ű�����
		}
		else
		{
			ReceiverKindFlag = SHARPREC;
			Threshold = SharpThreshold;							//ѡ����������
		}
	}
	if(StationEastFlag > EAST)									//2009-2-17�޸�
	{
		 StationEastFlag = EAST;
	}
	if(StationNorthFlag > NORTH)								//2009-2-17�޸�
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
* ����ԭ�ͣ�void PolarAngleInit(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ���������������ǳ�ʼ������λ���뼫������תȦ��Ϊ3��1����˽�Բ�������ĽǶȷ�Ϊ3�Σ��˺�����������ת����λ������������ʼ��Ϊ0
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
* ����ԭ�ͣ�void AntennaInit(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������߳�ʼ��
*
* ��    �ģ�35�����¶�����װ�߰꣨2009-3-11�������
*
* ��    �ģ��������߿��Ե�����91����ش��루2009-4-13�������
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

	/*2009/11/4����Լ���оƬ�Ĵ���*/
	EA 	  = 0;								//2010/4/19���룬оƬ�Ż�	//Rill move here 101202
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
//*/��������2009-1-20	
	while(1) 												//��������2009-1-20
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
					LEDRED;									//�����				
				}
			}
		}
		CloseTimer0Interrupt();
		LEDRED;								  		
		if(OverflowT0 > 1)									//2009-3-9��Ϊ0.1�뿪��ʱ��	
		{
			break;
		}
	}//end while(1) ��������2009-1-20
/*---------------------------------------------------------------------*/
	LEDRED;

	SatLongEastFlag = EAST;
	watchwire();											//����������л�
	StationEl = 10.0;
	NoMonitorF = 1;											//Ĭ���޼��
	AntReadySearchF = 1;									//��ʼ��׼����������λ(2009/10/12)

	PolarAngleInit();
	StationPol = GetComPolA();
	GotoPolarAngle(45);										//

	Abnormal = ReadEEPROM(AbnormalFlagAddress);				//��ȡ�Ƿ�����������־��Abnormal=0Ϊ����������Abnormal=1Ϊ����������
	
	if(ReadEEPROM(PolCompensateAddress) == '+')				//������У��
	{
		AngleCom = ReadEEPROM(PolCompensateAddress + 1);
	}
	if(ReadEEPROM(PolCompensateAddress) == '-')
	{
		AngleCom = 0.0 - ReadEEPROM(PolCompensateAddress + 1);
	}
	if(ReadEEPROM(ELCompensateAddress) == '+')	   			//������У��
	{
		GradientRightR = ReadEEPROM(ELCompensateAddress + 1) / 10.0;
	}
	if(ReadEEPROM(ELCompensateAddress) == '-')
	{
		GradientRightR = 0.0 - ReadEEPROM(ELCompensateAddress + 1) / 10.0;
	}

	AzMidLimitF 	= 0xff;		   	//2010-7-7
	AZLeftLimitF 	= 0xff;			//
	AZRightLimitF 	= 0xff;			//�����������ǵ����λ��־��Ŀǰ����ֻ��ֵΪ1��ʱ���ʾ��Ӧ����λ���ϣ�����ֵ��������״̬

	Delay(1000);											// �Ż�ʱ�䣬2010/4/19
	WriteEEPROM(1, AbnormalFlagAddress);					//���治�����ػ���־


	SSrcStarKbS = 27500.0;
	SharpRecFreq = 1135.0;
	SetSharpFreq(SharpRecFreq, SSrcStarKbS);

	GetPara();
	GetBaseStarPara();
	Delay(1000);											//��ȡ���ǲ���
																
	TestSelf();												//�Լ�																
	StationAz = AZ180;										//��������
	Delay(1000);
	ReadGradient();
//	StationEl = GradientY;
//	MotorCtrl(UPDOWN, 35.0, GetElS1());
	/*2010-7-16����Ը����Ƕȵ��жϣ���Ҫ�Ƿ�ֹ��Ϊ�ֶ�������ҡ��*/
	if(GradientNormal == 1 && Abnormal == 0 && GradientY < 25.0)	//�����б������,������������
	{
		StationEl = GradientY;
		if(StationEl < 0)
	   	{
			WriteEEPROM('-', StoreELSignAddress);			//�洢��ʼ��б�½Ƕ�		
			WriteEEPROM(0.0 - StationEl, StoreElDownAddress);//�洢��ʼ��б�½Ƕ�
		}
		else
		{
	   		WriteEEPROM('+', StoreELSignAddress);			//�洢��ʼ��б�½Ƕ�		
	   		WriteEEPROM(StationEl, StoreElDownAddress);		//�洢��ʼ��б�½Ƕ�
		}
		ELUPlimit = StationEl - 14.0 + 80.0;				//�ٴεõ���������
		ELDOWNlimit = StationEl - 14.0 + ELDOWNlimitDef;								
	}

	//2010-7-29����
	Delay(3000);
	if((ReadEEPROM(StoreELSignAddress) != '-') && (ReadEEPROM(StoreELSignAddress) != '+'))
	{
	   	WriteEEPROM('+', StoreELSignAddress);			//�洢��ʼ��б�½Ƕ�		
	   	WriteEEPROM(0, StoreElDownAddress);				//�洢��ʼ��б�½Ƕ�		
	}



	if(GradientNormal == 1)								//�����б������
	{

		StationEl = GradientY;
/* 2016/3/18 ���룬�������߸��������Ƿ���������߰ڷŽǶȱ仯���仯 */
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
			AzLimitTest(); 									//2010-7-8�������λ�Ĳ��ԣ�
			GotoAzMid();
			BianBanFlag = 0;
			SetBianBan();									//�ȴ�װ�߰�		
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

   	BaseStarFlagThree = 0;									//ʹ�òο����Ǳ�־λ
	EnStorSrcPara = 1;
	status = SEARCHREADY;
	return;
}


/***********************************************************************
*
* ����ԭ�ͣ�void GetParaEn(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������׼������ǰ,ȷ����Ŀ�����ǻ��ǲο�����
*
***********************************************************************/
static void GetParaEn(void)
{
	/*����Ŀ�����ǲ���*/
	ParaProtect();
	if(EnStorSrcPara == 1)							//����Ŀ�����ǲ���
	{
		StorSrcPara();
		EnStorSrcPara = 0;
	}

	/*
	 * ������ѡ���˲ο����ǣ�������ʹ�ü�ظ��Ĳο�����
	 */
	if(BaseStarFlagThree == 2)
	{
		EnBaseStar = 2;	   //�Ӳο����Ƕ���ʼ�����������Ĳο�����
		BaseStarFlagThree = 0;
	}
	else
	{
		EnBaseStar = 1;	
	}

	/*
	 * �Ƿ�ʹ�ü������Ĳο�����
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
* ����ԭ�ͣ�void AntennaReadySearch(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������׼�������������۽Ǹ���
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

	GetParaEn(); 		//2009/6/12�õ�Ҫ�����ǲ���������ر�־λ

	FindAnt(StationLong, StationLat, SatLongS, StationPloModeS);	//���Ǽ��㣬�õ�����ֵ	
//	FindAnt(StationLong, StationLat, 113, H);	//���Ǽ��㣬�õ�����ֵ															   
//	PolarAngleInit();
	GotoPolarAngle(StationPolCal);									//��Ŀ�꼫��

	
	if(ReceiverKindFlagS == XINBIAOREC)
	{
//		SetXinBiaoFreqKC(XinBiaoRecFreqS);							//�����ű�Ƶ��
		SetXinBiaoFreqKC(1300.0);							//�����ű�Ƶ��	
	}	
	else
	{
		SharpRecNormalFlag = SetSharpFreq1(SharpRecFreqS, SrcStarKbS);	
	} 

	/****************************************************		
	*�ߵ����۸���
	****************************************************/
	TimeTest(1);
	ReadGradient();
	if(GradientNormal == 1)							//�����б������
	{
		if(GradientY - StationEl < 8.0 && GradientY - StationEl > -8.0)
		{
			StationEl = GradientY;
		}
	}
	StationElTemp = StationElCal;
	if(StationElCal > ELUPlimit)  					//�������۸���ֵ
	{
		StationElTemp = ELUPlimit;
	}
	if(StationElCal < ELDOWNlimit)
	{
		StationElTemp = ELDOWNlimit;
	}

	if(StationEl < StationElTemp)					//�����۸���λ��
	{
		MotorFun(ELUP, MOTORSTOP, GetElS1());
		while(StationEl < StationElTemp)
		{
			watch();
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12�ٴθ�λ��
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
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12�ٴθ�λ��
			{
				AntReadySearchF = 1;
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
				return;	
			}
		}
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	}//if(StationEl < StationElTemp)

	/****************************************************		
	*�õ�����
	*�ߵ����۷�λ�����Ҳ�30��ʱ
	****************************************************/
	TimeTest(2);
	a = 1;
	AGCNoise = GetAGC();
	j = StationAz;
	MotorLook = StationAzCal + AZSearchRange;		//�ҵ��ұ߽�
	if(MotorLook > AZ180 + AZlimit)
	{											    //��λ����
		MotorLook = AZ180 + AZlimit;
	}
	 
	if(StationAz < MotorLook)						//��λ�ߵ�Ŀ��
	{
		MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
		while(StationAz < MotorLook)
		{
			watch();
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12�ٴθ�λ��
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
			if((status > TRACKING) || (AntReadySearchF == 0))  //2009/10/12�ٴθ�λ��
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

	if(ResetFlag == 1)								//�����λ��־Ϊ��
	{
		ReturnSet();								//������������	
		ReturnResetSuccess();						//���ظ�λ�ɹ�
		ResetFlag = 0;								//�����λ��־
	}		
	return;
}


/***********************************************************************
*
* ����ԭ�ͣ�void AntennaTimeSearch(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������������
*
***********************************************************************/
void AntennaTimeSearch(void)
{
	float tempUP, tempDOWN;
	float LeftEdgeAz, RightEdgeAz, UpEdgeEl, DownEdgeEl;	//�������ұ߽�
	UINT8 DirFlag;							  				//�жϷ���  ��0��1��2��3��
	UINT8 r;
	UINT16 uiLockStatus;
	UINT16 FlagSpeed = 0;
	UINT8 CountHoop = 0;									//ȷ���������
	float tempCH = 0.0;										//2009/5/21
	float T1, T2, T3;


	/*ȷ����߽�*/
	LeftEdgeAz = StationAzCal - AZSearchRange;
	if(LeftEdgeAz < AZ180 - AZlimit)
	{
		LeftEdgeAz = AZ180 - AZlimit;
	}
	
	/*ȷ���ұ߽�*/
	RightEdgeAz = StationAzCal + AZSearchRange;
	if(RightEdgeAz > AZ180 + AZlimit)
	{
		RightEdgeAz	= AZ180 + AZlimit;
	}
	
	/*ȷ���ϱ߽�*/
	UpEdgeEl = StationElCal + ELSearchRange;
	if((UpEdgeEl > ELUPlimit) || (UpEdgeEl < ELDOWNlimit))
	{
		UpEdgeEl = ELUPlimit;
	}
	
	/*ȷ���±߽�*/ 
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
	T3 = StationAz;   	//�����жϷ�λ�Ƿ����ĳһ�������ٽ��и�������

	while(CountHoop < 6)
	{
		if(fabs(StationAz - T3) > 4.5)				//2009/10/12����ȲŽ��в���
		{							
			if(GradientNormal == 1)							//�����б������  	//2009/5/21����
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

		if(DirFlag == 0) 												//������������
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
				CountHoop++;    										//������Ȧ��
				MotorFun(MOTORSTOP, MOTORSTOP, SpeedSearchEl);
				DirFlag = 1;
				tempCH += 1.0;
				tempUP = StationElTemp + tempCH;					//2009/5/21
				if(tempUP > UpEdgeEl)									//�������첻�������
				{
					tempUP = UpEdgeEl;
				}	
			}	
		}//end if(DirFlag == 0) 
		
		if(DirFlag == 1) 												//������������
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
		
		if(DirFlag == 2) 												//������������
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
				if(tempDOWN < DownEdgeEl)								//���������߲��������
				{
					tempDOWN = DownEdgeEl;
				}
			}	
		}//end if(DirFlag == 2)
		
		if(DirFlag == 3) 												//������������
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

		/*LNB���ϵ���!!!��ͬ���ջ�����AGCNoiseNor��ͬ, �����ڳ�ʼ��ʱ��AGCNoiseNor = -1.0*/
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
* ����ԭ�ͣ�void GoToObjStar(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ߴӲο����ǵ�Ŀ�����ǵĹ���
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
		SetXinBiaoFreqKC(XinBiaoRecFreq);				//�����ű�Ƶ��
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

	if(StationEl < ElTemp)						//�����۸���λ��
	{
		MotorFun(ELUP, MOTORSTOP, GetElS1());
		while(StationEl < ElTemp)
		{
			ReadGradient();
			if(GradientNormal == 1 && 			\
			   GradientY - StationEl < 8.0 && 	\
			   GradientY - StationEl > -8.0)		//�����б������
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
			   GradientY - StationEl > -8.0)		//�����б������
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
* ����ԭ�ͣ�void AntennaTracking(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������׼������
*
***********************************************************************/
void AntennaTracking(void)
{
	UINT8 DIR_Flag;					//�����־,ȡֵ0,1,2,3,�ֱ��Ӧ��,��,��,��
	UINT8 count;					//������һȦAGC�ĸ���
	UINT8 Trackcount;				//����Ȧ��
	float AGCPrev = 0.0;			//��һ�β���ֵ
	float TrackingStep = 0.1;		//��λ���ٲ���
	float AverageAGC = 0.0;			//����״̬��ÿȦƽ��AGCֵ

	DIR_Flag 	= 0;
	AGCPrev  	= GetAGC();
	AverageAGC 	= AGCPrev;
	count	 	= 1;
	Trackcount  = 0;
	while(1)
	{
		if(DIR_Flag == 0)			//�����
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
		
		if(DIR_Flag == 2)			//�Ҹ���
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
* ����ԭ�ͣ�void AntennaManual(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ���������������ֶ�
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
* ����ԭ�ͣ�void AntennaStore(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ���������������ղ�
*
***********************************************************************/
void AntennaStore(void)
{
	float temp;

//	GotoPolarAngle(90.0);									//��������
	MotorCtrl(UPDOWN, 35.0, GetElS1());						//������35��
//	MotorCtrl(UPDOWN, 35.0, 0xF000);	

	if(AzMidLimit != MotorLimitFlag)						//���м���λ
	{
		TimeTest(2);
		GoAM();
		StoreStatus1 = 2;
	}
	else if(StoreStatus1 == 0)  //2008-07-31 �ղز��õ��м���λ����
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
		   ChangeKit != 0)										//��ѯ�߰��Ƿ�ж��
	{
		Delay(3000);
		SendSJ();
		Delay(3000);	
		watch();
	}
	CloseTimer0Interrupt();
	UninstallBianbanFlag = 0;

	if(AZDir == AZLEFT)											//������ұ߹������ټ���ת,��Ϊ�м���λ��ƫ
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
	if(temp > 20.0)	//2009/8/6������б�ǵ����⣨���60�ȣ�
	{
		temp = 20.0;	
	}
	if(temp < -30.0)
	{
		temp = -30.0;
	}
	if(temp < StationEl)
	{	
		MotorCtrl(UPDOWN, temp, GetElS1());						//�µ�б�µĽǶ�
	}
	WriteEEPROM(0, AbnormalFlagAddress);						//�ĵ���ʱ�򱣴������ػ���־

/*2008-11-14���룬�ղ����¿ɵ�*/
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
* ����ԭ�ͣ�void TrackingSucceed(void)
*
* ��ڲ���������DeltaEl,����DeltaAz
*
* ���ڲ�������
*
* ���������������������Ҹ������ֵ,���õ���ƽ���˲���
*
***********************************************************************/
static void TackingUpDownRightLeft(float DeltaAz,float DeltaEl, float St)
{
 	UINT16 i, j, k;
	UINT16 AzNumbers, ElNumbers;				//��0.1�ȵĸ���
	float Filters[MaxFilterLen];				//�˲����洢agc
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
	}											//��ʼ���˲���

	MaxStationAz_1 = StationAz;					//��ʼ�����ֵ
	MaxAverageAGC = Filters[0];					//��ʼ��ƽ��agc
	j = 0;										//��ʼ����ʼ��
	for(i = 0; i < AzNumbers * 2; i++)
	{
	   MotorCtrl(RIGHTLEFT, StationAz + St, SpeedTRACKING);	//����������0.1
	   watch();
	   if(status == MANUALMODE || status == SEARCHREADY || status == STORESTATUS)
			return;
	   Filters[j++] = GetAGC();							//����һ���µĲ���ֵ
	   if(j == MaxFilterLen)
	   		j = 0;										//ѭ��
	   for(k = 0; k < MaxFilterLen; k++)
		  SumAGC += Filters[k];							//������
	   if((SumAGC / MaxFilterLen) > MaxAverageAGC)		//�ж����ֵ����
	   {
	   	  MaxStationAz_1 = StationAz;
		  MaxAverageAGC = (SumAGC / MaxFilterLen);
	   }
	   SumAGC = 0;										//����ƽ������
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
	   MotorCtrl(RIGHTLEFT, StationAz - St, SpeedTRACKING);		//����������0.1
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

	MotorCtrl(RIGHTLEFT, MaxStationAz, SpeedTRACKING);			//�ص����㷽λ
	Delay(60000);


	MotorCtrl(UPDOWN, StationEl - DeltaEl, SpeedTRACKING - SElspeed);			//��������2��
	for(i = 0;i < MaxFilterLen; i++)
	{
		 Filters[i] = GetAGC();
	}
	MaxStationEl_1 = StationEl;
	MaxAverageAGC = Filters[0];
	j = 0;
	for(i = 0;i < 2 * ElNumbers; i++)
	{
	   MotorCtrl(UPDOWN, StationEl + St, SpeedTRACKING - SElspeed);				//����������0.01
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
	   MotorCtrl(UPDOWN, StationEl - St, SpeedTRACKING - SElspeed);		//����������0.01
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
* ����ԭ�ͣ�void TrackingSucceed(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������߸���������,����״̬
*
* ��    �ģ���������ʮ�����ڵ�����µķ�Ӧ,������10���ڵ������׼������
*
*           �޸��ˣ����
*
*           �޸�ʱ�䣺2008-11-04
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
		*�����������10���ڵ�
		*�����׼������
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
/*************************������(2009-3-9)*********************/
static void AntTestFun(void)
{
	while(1)
	{
		LEDTest();//����LED
		GradientTest();//������б��
		MotorUp();//���Ը������
		TestXianWei();//���Է�λ�������λ
		TestPolar();//���Լ���
		TestXinBiao();//�����ű�
		TestGPS();//����GPS
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
	/*����λ���ϱ�־*/
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
	WriteEEPROM(a, SatXinBiaoFreqAddress);		//�洢�ű���ջ���Ƶ����������ǧλ��λֵ
	WriteEEPROM(b, SatXinBiaoFreqAddress + 1);	//�洢�ű���ջ���Ƶ����������ʮλ��λֵ
	WriteEEPROM(c, SatXinBiaoFreqAddress + 2);	//�洢�ű���ջ���Ƶ��С������ʮ�ְٷ�ֵ
	WriteEEPROM(d, SatXinBiaoFreqAddress + 3);	//�洢�ű���ջ���Ƶ��С������ǧ��λֵ
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
	MotorFun(ELUP, MOTORSTOP, GetElS1());	  //�������ϵ�65��
	while(65.0 > StationEl)
	{
		ReadGradient();
		StationEl = GradientY;
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	TimeTest(TIME3);

	MotorFun(ELDOWN, MOTORSTOP, GetElS1());	  //�������µ�40��
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
	MotorCtrl(RIGHTLEFT, AZ180 + 5, GetAzS2());	 //2016/3/15   �ֲ��ص��м���λ���ƫ��
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

	MotorFun(ELDOWN, MOTORSTOP, GetElS1());	  //�������µ�40��
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
* ����ԭ�ͣ�void StorSrcPara(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������Ŀ�����ǲ���
*
***********************************************************************/
static void StorSrcPara(void)
{
	int a, b, c, d, i;
	float j;

	/****************************************************
	*�洢����ֵ 
	*���´洢����վ����,�洢��������
	*�辭��Ϊ118.78����a=118��b=78
	****************************************************/
	TimeTest(1);
	a = (int)(StationLong);
	b = (StationLong - (float)a) * 100.0;
	WriteEEPROM(a, StationLongAddress);	
	WriteEEPROM(b, StationLongAddress + 1);
	
	/****************************************************
	*���´洢����վ������־,�洢�������� 
	*�洢0����1
	****************************************************/
	WriteEEPROM(StationEastFlag, StationEastFlagAddress);
	
	/****************************************************
	*���´洢����վγ��,�洢��������
	*�辭��Ϊ32.04����a=32��b=4
	****************************************************/
	a = (int)(StationLat);
	b = (StationLat - (float)a) * 100.0;
	WriteEEPROM(a, StationLatAddress);		
	WriteEEPROM(b, StationLatAddress + 1);
	
	/****************************************************		
	*���´洢����վ��γ��־,�洢��������
	*�洢0����1
	****************************************************/		
	WriteEEPROM(StationNorthFlag, StationNorthFlagAddress);
	
	/****************************************************		
	*���´洢���Ǿ���,�洢��������
	*�辭��Ϊ87.5����a=87��b=50
	****************************************************/
	a = (int)(SatLong);
	b = (SatLong - (float)a) * 100.0;
	WriteEEPROM(a, SatLongAddress);				
	WriteEEPROM(b, SatLongAddress + 1);
	
	/****************************************************		
	*���´洢����վ������ʽ,�洢��������
	*�洢0����1
	****************************************************/	
	WriteEEPROM(StationPloMode, StationPloModeAddress);	
	

	/****************************************************		
	*���´洢���Ƕ���������,�洢��������
	*�洢0����1
	****************************************************/	
	WriteEEPROM(SatLongEastFlag, SatLongEastFlagAddr);

	/****************************************************		
	*���´洢����վ���ջ�����,�洢��������
	*�洢0����1
	****************************************************/	
	WriteEEPROM(ReceiverKindFlag, ReceiverKindAddress);		 
	
	/****************************************************		
	*���´洢���ս��ջ�Ƶ��,�洢��������
	*��Ƶ��Ϊ1051.000����a=10��b=51,c=00,d=0
	****************************************************/
	i = (int)(SharpRecFreq);
	j = (SharpRecFreq - i) * 1000;
	a = i / 100;								//a=10
	b = i % 100;								//b=51
	c = j / 10;									//c=00
	d = ((int)j) % 10;							//d=0
	WriteEEPROM(a, SatSharpFreqAddress);		//�洢���ս��ջ���Ƶ������ǧλ��λֵ
	WriteEEPROM(b, SatSharpFreqAddress + 1);	//�洢���ս��ջ���Ƶ������ʮλ��λֵ
	WriteEEPROM(c, SatSharpFreqAddress + 2);	//�洢���ս��ջ���Ƶ��С��ʮ�ְٷ�ֵ
	WriteEEPROM(d, SatSharpFreqAddress + 3);	//�洢���ս��ջ���Ƶ��С���ٷְٷ�ֵ
	WriteEEPROM(SharpThreshold * 10.0, SharpAgcThresholdAddress);		  //�洢����
	
	/****************************************************		
	*���´洢�ű���ջ�Ƶ��,�洢��������
	*��Ƶ��Ϊ1449.500����a=11��b=49,c=50,c=50,d=0
	****************************************************/
	i = (int)(XinBiaoRecFreq);
	j = (XinBiaoRecFreq - i)*1000;
	a = i / 100;								//a=14
	b = i % 100;								//b=49
	c = j / 10;									//c=50
	d = ((int)j) % 10;							//d=0
	WriteEEPROM(a, SatXinBiaoFreqAddress);		//�洢�ű���ջ���Ƶ����������ǧλ��λֵ
	WriteEEPROM(b, SatXinBiaoFreqAddress + 1);	//�洢�ű���ջ���Ƶ����������ʮλ��λֵ
	WriteEEPROM(c, SatXinBiaoFreqAddress + 2);	//�洢�ű���ջ���Ƶ��С������ʮ�ְٷ�ֵ
	WriteEEPROM(d, SatXinBiaoFreqAddress + 3);	//�洢�ű���ջ���Ƶ��С������ǧ��λֵ	
	WriteEEPROM(XinBiaoThreshold * 10.0, XBThresholdAddress);				  //�洢����
}
