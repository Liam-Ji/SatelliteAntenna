/***********************************************************************
* 
* ʱ    �䣺2008-10-10
*
* �� �� ����timer.c
*
* ��    ����AKD10P07(������)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
* 
* ����˵����C8051F020Timer��غ���,���Ե�timer3��timer0��һ���
*
*           1,��ʱ��0��Ҫ������ʱ��2����ʱ��3�����������������
*
* ע    �⣺������Щ����ʱ����ɶԵ��ã���ϸ�ο���ش��룻
*
* ��    ��: ����MotorCtrl�������������ڷ�λ�����ߵ�ĳһ���Ƕ�
*
*           �޸��ˣ����
*
*           �޸�ʱ�䣺2008-11-3
*
* ��    ��: ���뺯��GotoElMid��������Яʽû�������������GotoAzMid();
*
*           �޸��ˣ����
*
*           �޸�ʱ�䣺2008-11-24
*
* ��   �ģ����������õ�У�����ٶȺ���GetElS1(),GetAzS2(),GetAzS1();
*  
*          ���  2008-11-25
*
* ��   �ģ��Ľ�MotorCtrl����������ʱ��2009-3-25�����
*
***********************************************************************/
#include "timer.h"
#include "station.h"
#include "protcol.h"
#include "rweeprom.h"
#include "math.h"
#include "adc.h"
#include "polar.h"
#include "uart.h"


/*ȫ�ֱ���*/
INT16 data OverflowT0;								//���������
float data StationAz;								//����վ��λ�Ƕ� 
float data StationEl;								//����վ�����Ƕ�
float data PolarDst;								//����Ŀ�ؽǶ�
INT16 data PolarRightFlag;							//������λ��־


float data StationAzLimit;

UINT8 data AzMidLimitF;				//2010-7-7
UINT8 data AZLeftLimitF;			//
UINT8 data AZRightLimitF;			//��λ���ϱ���


xdata UINT8  TEMPAGE;


/*��̬����*/
static UINT16 data CountT0;
static UINT16 data AZMotorflag = 0;					//AZ���������־
static UINT16 data ELMotorflag = 0;					//EL���������־


/*25KG������*/								
static double data AZMotorPlusStep = 0.00225;	   	//��λ���һ�����岽����ȱʡ
static double data ELMotorPlusStep = 0.00225;	   	//2009-3-25


static float data PolarDefinition  = 0.1;	  		//������ȷ��
static float data PolarSrc;							//����Դ�Ƕ�


/*��̬����*/
static void OpenTimer3Interrupt(UINT16 count);
static void CloseTimer3Interrupt(void);
static UINT8 GotoAzMidL(void);
static UINT8 GotoAzMidR(void);
static UINT8 GotoAzMidM(void);

extern UINT16 abcd1;
extern UINT16 abcd2;
extern UINT16 abcd3;
/***********************************************************************
*
* ����ԭ�ͣ�void Timer0Init(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ʱ��0��ʼ��;
*
***********************************************************************/
void Timer0Init(void)
{
	EA 	    =  0;
	SFRPAGE = TIMER01_PAGE;
	CKCON  |=  BIT2;			//��ʱ��0 ʹ��ϵͳʱ��,�ĵ��д��󣡣���
	TMOD   &= ~BIT3;
	TMOD   &= ~BIT2;			//ѡ��ʱ������
	TMOD   |=  BIT1;
	TMOD   &= ~BIT0;			//1 0 ��ʽ2���Զ���װ�ص�8 λ������/��ʱ��
	TL0     =  0x48;			//��ֵΪ72100us��ʱ
	TH0     =  0x48;			//����ֵ
	TR0     =  0;            	//��ֹ��ʱ��0
	EA      =  1;
}


/***********************************************************************
*
* ����ԭ�ͣ�void OpenTimer0Inrerupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ����������������ʱ��0�ж�
*
***********************************************************************/
void OpenTimer0Interrupt(void)
{
	CountT0 	= 0;
	OverflowT0 	= 0;

	TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = TIMER01_PAGE;

	TF0   		= 0;			//�����־��0
	TR0   		= 1;			//��ʼ����
	ET0  	 	= 1;			//����TF1 ��־λ��TCON.7�����ж�����

	SFRPAGE		= TEMPAGE;
}


/***********************************************************************
*
* ����ԭ�ͣ�void CloseTimer0Inrerupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������رն�ʱ��0�ж�
*
***********************************************************************/
void CloseTimer0Interrupt(void)
{
	TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = TIMER01_PAGE;

	TR0 	= 0;	
	ET0 	= 0;					//��ֹ��ʱ��0 �ж�

	SFRPAGE		= TEMPAGE;
}


/***********************************************************************
*
*����ԭ�ͣ�void Timer0Interrupt(void)
*
*��ڲ�������
*
*���ڲ�������
*
*������������ʱ��0�жϺ���,������ʱ�ã�Լÿ 23ms OVERFLOWT0��һ
*
***********************************************************************/
void Timer0Interrupt(void) interrupt T0INTNUMBER
{	
    TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = TIMER01_PAGE;	  //ҳ0	  120�޸�

	TF0 = 0;
	CountT0++;
	if(CountT0 > (OFT0NUMBER - 1))
	{
		OverflowT0++;
		CountT0 = 0;
	}

	SFRPAGE	= TEMPAGE;
}


/***********************************************************************
*
* ����ԭ�ͣ�void Timer3Init(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ʱ��3��ʼ�������ڸ��������
*
* ע���õ��ⲿʱ�ӣ�������ʱ��3
*
***********************************************************************/
void Timer3Init(void)
{
    EA		  =  0;		     //IE����ҳ
    SFRPAGE   =  TMR3_PAGE;
    TMR3CN   &=  0x00;  //������ʽ��16λ�Զ����أ��Ͷ�ʱ������ 
//	TMR3CN   |=  0x04;	//��ʱ��3����
	EIE2   	 &= ~BIT0;//�ȹر��ж�
	EA  	  =  1;
}


/***********************************************************************
*
* ����ԭ�ͣ�void OpenTimer3Interrupt(UINT16 count)
*
* ��ڲ�����count�������������Ƶ�ʣ��ɲο�C8051F020�ֲ�
*
* ���ڲ�������
*
* ����������������ʱ��0�ж�
*
***********************************************************************/
void OpenTimer3Interrupt(UINT16 count)
{
	SFRPAGE = TMR3_PAGE;
	RCAP3L	= 0x00ff & count;		 //ע��
	count	= count>>8;
	RCAP3H	= 0x00ff & count;
	TMR3CN |= 0x04;	//��ʱ��3����
	EIE2   |= BIT0;  //���ж�
}


/***********************************************************************
*
* ����ԭ�ͣ�void CloseTimer3Inrerupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������رն�ʱ��3�ж�
*
***********************************************************************/
static void CloseTimer3Interrupt(void)
{
	SFRPAGE =  TMR3_PAGE;
    TMR3CN &= ~BIT2;						//TR3=0,����������
	EIE2   &= ~BIT0;						//�����ж�
}


/***********************************************************************
*
* ����ԭ�ͣ�void Timer3Interrupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ʱ��3�жϺ���,�����������������
*
***********************************************************************/
void Timer3Interrupt(void) interrupt T3INTNUMBER
{	
	TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = TMR3_PAGE;	  //ҳ1	120�޸�

	TMR3CN &= ~BIT7;

	if(AZLeftLimitF > 0x10 && AZLeftLimit == MotorLimitFlag && StationAzLimit > 120.0)
	{
		AZLeftLimitF = 1;
		StationAz = StationAzLimit;	
	}

	if(AZRightLimitF > 0x10 && AZRightLimit == MotorLimitFlag && StationAzLimit < 240.0)
	{
		AZRightLimitF = 1;
		StationAz = StationAzLimit;	
	}

	if(AZDir == AZRIGHT && AZRightLimit == MotorLimitFlag \
		&& AZRightLimitF > 0x3 && AZMotorflag == 1)
	{
		StationAz = AZ180 + AZlimit + 1.0;		//��λ����
		return;
	}

	if(AZDir == AZLEFT && AZLeftLimit == MotorLimitFlag \
		&& AZLeftLimitF > 0x3 && AZMotorflag == 1)
	{
		StationAz = AZ180 - AZlimit - 1.0;		//��λ����
		return;
	}



			
	if(ELMotorflag == 1)					   	//EL���ת����־
	{
		if(((ELDir == ELDOWN)&&(StationEl < ELDOWNlimit)) || ((ELDir == ELUP)&&(StationEl > ELUPlimit)))//Rill add to protect EL motor 101202
		{	 
			 ELMotorflag = 0;
			 return;  
		}
		if(ELPlus == 1)
		{
			ELPlus = 0;
		}
		else
		{
			ELPlus = 1;
			if(ELDir == ELUP)
			{
				StationEl += ELMotorPlusStep;
			}
			else
			{
				StationEl -= ELMotorPlusStep;
			}	
		}
	}//end if(ELMotorflag == 1)
		if(AZMotorflag == 1)					   //AZ���ת����־
	{
		if(AZPlus == 1)
		{
			AZPlus = 0;
		}
		else
		{
			AZPlus = 1;
			if(AZDir == AZRIGHT)
			{
				StationAz += AZMotorPlusStep;
				StationAzLimit += AZMotorPlusStep;
			}
			else
			{
				StationAz -= AZMotorPlusStep;
				StationAzLimit -= AZMotorPlusStep;
			}	
		}
	}//end if(AZMotorflag == 1)
	SFRPAGE	= TEMPAGE;	
}


/***********************************************************************
*
* ����ԭ�ͣ�void MotorFun(INT16 ElMode, INT16 AzMode, UINT16 speed)
*
* ��ڲ�����speed ����ʱ����,���Ƶ���ٶ�
*           
*          ElMode, AzMode�ο�ELUP,ELDOWN,AZRIGHT,AZLEFT��timer.h��
*
* ���ڲ�������
*
* �������������ת������
*
***********************************************************************/
void MotorFun(INT16 ElMode, INT16 AzMode, UINT16 speed)
{
	if(AzMode == MOTORSTOP && ElMode == MOTORSTOP)	//�ض�ʱ�����ص��
	{
		CloseTimer3Interrupt();						//ע������Լӵ������������������ת							
		return;
	}

	if(AzMode < MOTORSTOP)					  		//�Ƿ񿪷�λ���
	{
		AZMotorflag = 1;
		if(AzMode == AZRIGHT)						//��λ����ķ���
		{
			AZDir = AZRIGHT;
		}
		else
		{
			AZDir = AZLEFT;
		}
	}
	else
	{
		AZMotorflag = 0;
	}//end if(AzMode < MOTORSTOP)
	
	if(ElMode < MOTORSTOP)					  	    //�Ƿ񿪸������
	{
		ELMotorflag = 1;
		if(ElMode == ELUP)							//�����������
		{
			ELDir = ELUP;
		}
		else
		{
			ELDir = ELDOWN;
		}
	}
	else
	{
		ELMotorflag = 0;
	}//end if(ElMode < MOTORSTOP)

	OpenTimer3Interrupt(speed);			   			//�����ʼת��
}


/***********************************************************************
*
* ����ԭ�ͣ�void MotorCtrl(INT16 DirFlag, float Angle, UINT16 speed)
*
* ��ڲ�����speed ����ʱ����,���Ƶ���ٶ�
*           
*           DirFlag���·��������ҷ���
*
*		    AngleĿ��Ƕ�
*
* ���ڲ�������
*
* ���������������speed�ٶ��ߵ���λ������Angle�Ƕ�
*
***********************************************************************/
void MotorCtrl(INT16 DirFlag, float Angle, UINT16 speed)
{	
	if(DirFlag == UPDOWN)						  //���Ϊ���·���
	{
		if(Angle > ELUPlimit)					  //���Ŀ����ڸ�������
		{
			Angle = ELUPlimit;
		}										  //���Ŀ��С�ڸ�������
		if(Angle < ELDOWNlimit)
		{
			Angle = ELDOWNlimit;
		}
		if(Angle > StationEl)					  //���Ŀ��Ǵ��ڵ�ǰ����
		{
			MotorFun(ELUP, MOTORSTOP, speed);
			while(Angle > StationEl)
			{
				;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, speed);
		}
		else									  //���Ŀ���С�ڵ�ǰ����
		{
			MotorFun(ELDOWN, MOTORSTOP, speed);
			while(Angle < StationEl)
			{
				;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, speed);
		}
	}//end : if(DirFlag == UPDOWN)

		
	if(DirFlag == RIGHTLEFT)					//���Ϊ���ҷ���
	{
		if(Angle > AZlimit + AZ180)				//�����������λ	  	
		{
			Angle = AZlimit + AZ180;
		}										  
		if(Angle < AZ180 - AZlimit)				//���С������λ
		{
			Angle = AZ180 - AZlimit;
		}
		if(Angle > StationAz)					  
		{
			MotorFun(MOTORSTOP, AZRIGHT, speed);
			while(Angle > StationAz)
			{
				;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, speed);
		}
		else					 
		{
			MotorFun(MOTORSTOP, AZLEFT, speed);
			while(Angle < StationAz)
			{
				;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, speed);
		}
	}//end : if(DirFlag == RIGHTLEFT)		
}


/***********************************************************************
*
* ����ԭ�ͣ�void GoAM(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ߵ���λ�м���λ
*
* ��    �ģ�2010-07-16������������������ղ������ʱ���м���λ
*
***********************************************************************/
void GoAM(void)
{
	/*��ǰ���м���λ���ϴ��м���λ����*/
	if(AzMidLimit == MotorLimitFlag && AzMidLimitF > 0x10)
	{	
		StationAz = AZ180;
		return;
	}
	/*����м���λ���������м���λ*/
	if(AzMidLimitF > 0x10)
	{
		/*�����λ����180�ȣ�������ת��*/
		if(StationAz > 180.0)
		{
			TimeTest(2);	//�ߵ��ǰ������ʱ
			MotorFun(MOTORSTOP, AZLEFT, GetAzS2());			 	//��������ת���������ߺ��濴��
			while(StationAz > 160.0)
			{
				Delay(20);
				if(status != MANUALMODE)
				{
					watch();
				}
				if(AzMidLimit == MotorLimitFlag)
				{
					MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
					Delay(5000);
					if(AzMidLimit == MotorLimitFlag)
					{
						break;
					}
					else
					{
						MotorFun(MOTORSTOP, AZLEFT, GetAzS2());
					}
				}
			}
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			
			/*����ҵ��м���λ�򷵻�*/
			if(AzMidLimit == MotorLimitFlag)
			{
				StationAz = AZ180;
				return;
			}
			/*�ص�180�ȣ��м���λ����*/
			else
			{
				TimeTest(2);	//�ߵ��ǰ������ʱ
				MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
				AzMidLimitF = 1;
				WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);	
			}	
		}
		/*����Ƕ�С��180��������ת*/
		else
		{
			TimeTest(2);	//�ߵ��ǰ������ʱ
			MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
			while(StationAz < 200.0)
			{
				Delay(20);
				if(status != MANUALMODE)
				{
					watch();
				}
				if(AzMidLimit == MotorLimitFlag)
				{
					MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
					Delay(5000);
					if(AzMidLimit == MotorLimitFlag)
					{
						break;
					}
					else
					{
						MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
					}
				}
			}
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			
			if(AzMidLimit == MotorLimitFlag)
			{
				StationAz = AZ180;
				return;
			}
			else
			{
				TimeTest(2);	//�ߵ��ǰ������ʱ
				MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
				AzMidLimitF = 1;
				WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);
					
			}			
		}	
	}
	/*����м���λ��������ֱ���ߵ��м�180��*/
	else
	{
		TimeTest(2);	//�ߵ��ǰ������ʱ
		MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());		
	}
	StationAz = AZ180;
	return;
}


/***********************************************************************
*
* ����ԭ�ͣ�void GotoAzMid(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ߵ�AZ�м���λ
*
* ��    �ģ�2008-11-24�����������
*
***********************************************************************/
void GotoAzMid(void)
{
	/*���һ������м���λû�й��϶���Ŀǰ�����м���λ*/
	if(AzMidLimit == MotorLimitFlag && AzMidLimitF > 0x10)
	{	
		StationAz = AZ180;
		StationAzLimit = StationAz;
		return;
	}

	/*�������������λ������*/
	if(AZLeftLimitF == 1 && AZRightLimitF == 1 && AzMidLimitF == 1)
	{
		StationAz = AZ180;
		StationAzLimit = StationAz;
		return;
	}

	/*��������������λû�й���*/
	if(AZLeftLimitF > 0x10)
	{
		if(GotoAzMidL())
		{
			return;		
		}
	}

	/*����ģ��������λû�й���*/
	if(AZRightLimitF > 0x10)
	{
		if(GotoAzMidR())
		{
			return;		
		}	
	}

	/*����壺����м���λû�й���*/
	if(AzMidLimitF > 0x10)
	{
		GotoAzMidM();
	}
	return;
}


/***********************************************************************
*
* ����ԭ�ͣ�UINT8 GotoAzMidL(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ߵ�AZ�м���λ,����λ�������
*
* ��    �ģ�2010-7-9�����������
*
***********************************************************************/
static UINT8 GotoAzMidL(void)
{
	UINT8 tempL;
	UINT8 tempR;
	UINT8 tempM;
	UINT8 returnvalue;

	/*������ǰ����λ���*/
	tempL = AZLeftLimitF;
	tempR = AZRightLimitF;
	tempM = AzMidLimitF;
	
	/*����������λ����*/
	AZLeftLimitF = 1;
	AZRightLimitF = 1;

	StationAz = AZ180;

	/*������90�ȣ����������λ��ֹͣ*/
	TimeTest(2);	//�ߵ��ǰ������ʱ
	MotorFun(MOTORSTOP, AZLEFT, GetAzS2());			 	//��������ת���������ߺ��濴��
	while((AZLeftLimit != MotorLimitFlag)  &&  	\
			(StationAz > 90.0))
	{
		Delay(20);
		if(status != MANUALMODE)
		{
			watch();
		}
		if(AzMidLimit == MotorLimitFlag && tempM > 0x10)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			Delay(5000);
			if(AzMidLimit == MotorLimitFlag)
			{
				break;
			}
			else
			{
				MotorFun(MOTORSTOP, AZLEFT, GetAzS2());
			}
		}
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);

	/*���һ���ҵ��м���λ*/
	if(AzMidLimit == MotorLimitFlag && tempM > 0x10)	//������м���λ��ȷ����λֵ
	{
		StationAz = AZ180;
		StationAzLimit = StationAz;
		AZLeftLimitF = tempL;
		AZRightLimitF = tempR;
//		return 1;
		returnvalue = 1;
	}

	/*��������ҵ�����λ*/
	if(AZLeftLimit == MotorLimitFlag)
	{
		/*����м���λ�������������м���λ*/
		if(tempM > 0x10)
		{
			StationAz = 90.0;
			TimeTest(2);
 			MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());	//��������ת���������ߺ��濴��
			while((AzMidLimit != MotorLimitFlag)  &&  \
			(StationAz < 190.0))
			{
				Delay(20);
				if(status != MANUALMODE)
				{
					watch();
				}
				if(AzMidLimit == MotorLimitFlag && tempM > 0x10)
				{
					MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
					Delay(5000);
					if(AzMidLimit == MotorLimitFlag)
					{
						break;
					}
					else
					{
						MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
					}
				}
			}
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			
			/*����ҵ��м���λ����������*/
			StationAz = AZ180;
			StationAzLimit = StationAz;
			AZLeftLimitF = tempL;
			AZRightLimitF = tempR;

			/*���û���ҵ��м���λ�����м���λ��־Ϊ����*/
			if(AzMidLimit != MotorLimitFlag)
			{	
				AzMidLimitF = 1;		
			}
//			return 1;
			returnvalue = 1;	
		}
		/*����м���λ���ϣ���ֱ���ߵ��м���λ*/
		else
		{
			TimeTest(2);
			MotorCtrl(RIGHTLEFT, StationAz + 90.0, GetAzS2());
			StationAz = AZ180;
			StationAzLimit 	= StationAz;
			AZLeftLimitF 	= tempL;
			AZRightLimitF 	= tempR;
//			return 1;
			returnvalue = 1;	
		}	
	}

	/*�������ʲô��û���ҵ�*/
	else
	{
		TimeTest(2);
		MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
		StationAz = AZ180;
		StationAzLimit 	= StationAz;
		AZLeftLimitF 	= tempL;
		AZRightLimitF 	= tempR;
//		return 0;
		returnvalue = 0;	
	}
	return returnvalue;			
}


/***********************************************************************
*
* ����ԭ�ͣ�UINT8 GotoAzMidLR(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ߵ�AZ�м���λ,����λ�������
*
* ��    �ģ�2010-7-9�����������
*
***********************************************************************/
static UINT8 GotoAzMidR(void)
{
	UINT8 tempL;
	UINT8 tempR;
	UINT8 tempM;
	UINT8 returnvalue;

	/*������ǰ����λ���*/
	tempL = AZLeftLimitF;
	tempR = AZRightLimitF;
	tempM = AzMidLimitF;
	
	/*����������λ����*/
	AZLeftLimitF = 1;
	AZRightLimitF = 1;

	StationAz = AZ180;

	/*������90�ȣ����������λ��ֹͣ*/
	TimeTest(2);	//�ߵ��ǰ������ʱ
	MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());			 //��������ת���������ߺ��濴��
	while((AZRightLimit != MotorLimitFlag)  &&  	\
			(StationAz < 270.0))
	{
		Delay(20);
		if(status != MANUALMODE)
		{
			watch();
		}
		if(AzMidLimit == MotorLimitFlag && tempM > 0x10)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			Delay(5000);
			if(AzMidLimit == MotorLimitFlag)
			{
				break;
			}
			else
			{
				MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
			}
		}
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);

	/*���һ���ҵ��м���λ*/
	if(AzMidLimit == MotorLimitFlag && tempM > 0x10)	//������м���λ��ȷ����λֵ
	{
		StationAz = AZ180;
		StationAzLimit = StationAz;
		AZLeftLimitF = tempL;
		AZRightLimitF = tempR;
//		return 1;
		returnvalue = 1;
	}

	/*��������ҵ�����λ*/
	if(AZRightLimit == MotorLimitFlag)
	{
		/*����м���λ�������������м���λ*/
		if(tempM > 0x10)
		{
			StationAz = 270.0;
			TimeTest(2);
 			MotorFun(MOTORSTOP, AZLEFT, GetAzS2());	
			while((AzMidLimit != MotorLimitFlag)  &&  \
			(StationAz > 170.0))
			{
				Delay(20);
				if(status != MANUALMODE)
				{
					watch();
				}
				if(AzMidLimit == MotorLimitFlag && tempM > 0x10)
				{
					MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
					Delay(5000);
					if(AzMidLimit == MotorLimitFlag)
					{
						break;
					}
					else
					{
						MotorFun(MOTORSTOP, AZLEFT, GetAzS2());
					}
				}
			}
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			/*����ҵ��м���λ����������*/
			StationAz = AZ180;
			StationAzLimit = StationAz;
			AZLeftLimitF = tempL;
			AZRightLimitF = tempR;

			/*���û���ҵ��м���λ�����м���λ��־Ϊ����*/
			if(AzMidLimit != MotorLimitFlag)
			{	
				AzMidLimitF = 1;	
			}
//			return 1;
			returnvalue = 1;	
		}
		/*����м���λ���ϣ���ֱ���ߵ��м���λ*/
		else
		{
			TimeTest(2);
			MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
			StationAz = AZ180;
			StationAzLimit 	= StationAz;
			AZLeftLimitF 	= tempL;
			AZRightLimitF 	= tempR;
//			return 1;
			returnvalue = 1;	
		}	
	}

	/*�������ʲô��û���ҵ�*/
	else
	{
		TimeTest(2);
		MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
		StationAz = AZ180;
		StationAzLimit 	= StationAz;
		AZLeftLimitF 	= tempL;
		AZRightLimitF 	= tempR;
//		return 0;
		returnvalue = 0;	
	}
	return returnvalue;
}


/***********************************************************************
*
* ����ԭ�ͣ�UINT8 GotoAzMidLM(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ߵ�AZ�м���λ,����λ�������
*
* ��    �ģ�2010-7-9�����������
*
***********************************************************************/
static UINT8 GotoAzMidM(void)
{
	StationAz = AZ180;

	/*���һ��������90�ȣ����������λ��ֹͣ*/
	TimeTest(2);	//�ߵ��ǰ������ʱ
	MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());			 //��������ת���������ߺ��濴��
	while((AZRightLimit != MotorLimitFlag)  &&  	\
			(StationAz < 270.0))
	{
		Delay(20);
		if(status != MANUALMODE)
		{
			watch();
		}
		if(AzMidLimit == MotorLimitFlag)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			Delay(5000);
			if(AzMidLimit == MotorLimitFlag)
			{
				break;	
			}
			else
			{
				MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());
			}
			
		}
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	if(AzMidLimit == MotorLimitFlag)
	{
		StationAz = AZ180;
		return 1;	
	}

	/*�������������180�ȣ���������м���λ��ֹͣ*/
	TimeTest(2);
 	MotorFun(MOTORSTOP, AZLEFT, GetAzS2());
	while((AzMidLimit != MotorLimitFlag)  &&  \
	(StationAz > 90.0))
	{
		Delay(20);
		if(status != MANUALMODE)
		{
			watch();
		}
		if(AzMidLimit == MotorLimitFlag)
		{	
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			Delay(5000);
			if(AzMidLimit == MotorLimitFlag)
			{
				break;
			}
			else
			{
				MotorFun(MOTORSTOP, AZLEFT, GetAzS2());	
			}	
		}
	}
	MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
	if(AzMidLimit == MotorLimitFlag)
	{
		StationAz = AZ180;
		return 1;	
	}

	/*�������ʲôҲû������*/
	TimeTest(2);
	MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
	StationAz = AZ180;
	AzMidLimitF = 1;						//�м���λ����
	WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);
	return 0;
}


/***********************************************************************
*
* ����ԭ�ͣ�void GetAzS1(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������õ����ߵķ�λ�ٶ�1
*
* ��    �ģ�2008-11-25�����������
*
***********************************************************************/
UINT16 GetAzS1(void)
{
	unsigned int speedAZR;
	unsigned int tempAZ = (unsigned int)ReadEEPROM(rightFOUR);

	/*�õ�У����λ�ٶ�1*/
	if(tempAZ < 200)
	{	
		speedAZR = SpeedSearchAzF + (tempAZ - 100) * 20;
		if(speedAZR > 0xFEFF)
		{
			speedAZR = SpeedSearchAzF;
		}
		if(speedAZR < 0xE000)
		{
			speedAZR = SpeedSearchAzF;
		}
	}
	else
	{
		speedAZR = SpeedSearchAzF;
	}//end if(tempAZ < 200)
	return speedAZR;
}


/***********************************************************************
*
* ����ԭ�ͣ�void GetAzS2(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������õ����ߵķ�λ�ٶ�2
*
* ��    �ģ�2008-11-25�����������
*
***********************************************************************/
UINT16 GetAzS2(void)
{
	unsigned int speedAZR;
	unsigned int tempAZ = (unsigned int)ReadEEPROM(rightFIVE);

	/*�õ�У����λ�ٶ�2*/
	if(tempAZ < 200)
	{
		speedAZR = SpeedInitF + (tempAZ - 100) * 20;
		if(speedAZR > 0xFEFF)
		{
			speedAZR = SpeedInitF;
		}
		if(speedAZR < 0xE000)
		{
			speedAZR = SpeedInitF;
		}
	}
	else
	{
		speedAZR = SpeedInitF;
	}//end if(tempAZ < 200)
	return speedAZR;
}

/***********************************************************************
*
* ����ԭ�ͣ�void GetElS1(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������õ����ߵĸ����ٶ�1
*
* ��    �ģ�2008-11-25�����������
*
***********************************************************************/
UINT16 GetElS1(void)
{
	unsigned int speedELR;
	unsigned int tempEL = (unsigned int)ReadEEPROM(rightTHREE);

	/*�õ������ٶ�*/
	if(tempEL < 200)
	{
		speedELR = SpeedInitS + (tempEL - 100) * 20;
		if(speedELR > 0xFEFF)
		{
			speedELR = SpeedInitS;
		}
		if(speedELR < 0xE000)
		{
			speedELR = SpeedInitS;
		}
	}
	else
	{
		speedELR = SpeedInitS;
	}//end if(tempEL < 200)
	return speedELR;
}


/***********************************************************************
*
* ����ԭ�ͣ�void TestStor(char flag)
*
* ��ڲ�����flag �����ж������ֲ���
*
* ���ڲ�������
*
* ������������������ǰ�õģ����Բο�����ֲ�
*
***********************************************************************/
void TestStor(char flag)
{
	float tempS;
	unsigned int tempC;

	if(status == INIT || status == STORESTATUS)
	{
		return;
	}

	tempC = (unsigned int)ReadEEPROM(rightFIVE);
	if(flag == 'L')									 //�ж��Ƿ����ղ�
	{
		tempS = (float)ReadEEPROM(rightONE) / 10.0;
		if(StationAz < 200.0)
		{
			MotorCtrl(RIGHTLEFT, 200.0, GetAzS2());
		}
	}
	else
	{
		tempS = (float)ReadEEPROM(rightTWO) / 10.0;
		if(StationAz > 160.0)
		{
			MotorCtrl(RIGHTLEFT, 160.0, GetAzS2());
		}
	}
												  	
	TimeTest(2);
	GoAM();									//���м���λ
	GotoPolarAngle(90.0);							//��������

	if(tempS > 24.0)
	{
		tempS = 2.0;	
	}
	if(flag == 'L')									//������ұ߹������ټ���ת3����Ϊ�м���λ��ƫ
	{
		MotorCtrl(RIGHTLEFT, AZ180 - tempS, GetAzS2());
	}
	else
	{
		MotorCtrl(RIGHTLEFT, AZ180 + tempS, GetAzS2());
	}

	if(ReadEEPROM(StoreELSignAddress) == '-')
	{
		tempS = 0.0 - ReadEEPROM(StoreElDownAddress);
	}
	else if(ReadEEPROM(StoreELSignAddress) == '+')
	{
		tempS = ReadEEPROM(StoreElDownAddress);	
	}
	else
	{
		tempS = 0.0;
	}
	TimeTest(2);
	if(tempS > 20.0)	//2009/8/6������б�ǵ����⣨���60�ȣ�
	{
		tempS = 20.0;	
	}
	if(tempS < -30.0)
	{
		tempS = -30.0;
	}	
	if(tempS < StationEl)
	{
		ELDOWNlimit	= -120.0;
		MotorCtrl(UPDOWN, tempS, GetElS1());		//�µ�б�µĽǶ�
	}

	if(flag == 'U')
	{
		tempS = (float)ReadEEPROM(rightSIX);
		if(tempS > 99.0)
		{
			tempS = 80.0; 
		}
		if(tempS < 50.0)
		{
			tempS = 50.0;
		}
		MotorCtrl(UPDOWN, StationEl - tempS, GetElS1());		
	}

	OpenTimer0Interrupt();						   //�ȴ�ʮ��
	while(OverflowT0 < 100)
	{
		;
	}
	CloseTimer0Interrupt();

	TimeTest(2);
	MotorCtrl(UPDOWN, 35.0, GetElS1());			  //2008-12-30�޸�

	ELDOWNlimit	= ELDOWNlimitDef;

	return;
}


/***********************************************************************
*
* ����ԭ�ͣ�void Timer2Init(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ʱ��2��ʼ����������
*
***********************************************************************/
void Timer2Init(void)
{
    EA			=  0;		     //IE����ҳ
    SFRPAGE   	=  TMR2_PAGE;
	TMR2CF    	=  0x08;	 //ϵͳʱ��
    TMR2CN     &= ~0x03;  //������ʽ��16λ�Զ����أ��Ͷ�ʱ������
	
	ET2			=  0;       //�����ж�
	RCAP2L		=  0xFF;   // �Ѽ���ֵ�ĵ�8λ������ʱ��4��׽�Ĵ������ֽ�  
	TMR2L		=  RCAP2L;	  //��ֵ����ʱ��4���ֽ� 
	RCAP2H		=  0x7F;	 //�Ѽ���ֵ�ĸ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR2H		=  RCAP2H;
   
	SFRPAGE   	=  TMR2_PAGE;
	TMR2CN     |=  0x04;	//��ʱ��2����
	EA  		=  1;
}


/***********************************************************************
*
* ����ԭ�ͣ�void OpenTimer2Interrupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������ʱ��2�ж�
*
***********************************************************************/
void OpenTimer2Interrupt(void)
{
	SFRPAGE = TMR2_PAGE;
	TR2 = 1;			//�������
	ET2	= 1;			//���ж�
}


/***********************************************************************
*
* ����ԭ�ͣ�void CloseTimer2Interrupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������ض�ʱ��2�ж�
*
***********************************************************************/
void CloseTimer2Interrupt(void)
{
	SFRPAGE   = TMR2_PAGE;
	ET2	= 0;			//�����ж�
	TR2 = 0;			//���������
}


/***********************************************************************
*
* ����ԭ�ͣ�void Timer2Interrupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������ʱ��2�жϺ���
*
***********************************************************************/
void Timer2Interrupt(void) interrupt T2INTNUMBER
{

	TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = TMR2_PAGE;	  //ҳ0	  120�޸�

	TF2 = 0;

	POLAR_STOP;											//��ֹͣ����
	PolarSrc = GetPolarAngle();							//��������ת���Ƕ����ӣ�����ת�Ƕȼ�С

	if(fabs(PolarSrc - PolarDst) < PolarDefinition)
	{
		POLAR_STOP;
		PolarRightFlag = 1;								//�����ǵ�λ
		CloseTimer2Interrupt();
		return;
	}
	if(PolarSrc > PolarDst)								//��ǰ�����Ǵ���Ŀ��Ƕ�
	{
//		POLAR_RIGHT;
		POLAR_LEFT;
	}
	else
	{
//		POLAR_LEFT;
		POLAR_RIGHT;
	}

	SFRPAGE	= TEMPAGE;  //120�޸�
}


/***********************************************************************
*
* ����ԭ�ͣ�void AzLimitTest(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ������������λ��λ������
*
***********************************************************************/
void AzLimitTest(void)
{
	UINT8 tempL;
	UINT8 tempR;
	UINT8 tempM;

	UINT8 tempLF;
	UINT8 tempRF;
	UINT8 tempMF;

	/*������ǰ����λ���*/
	tempL = 0xff;
	tempR = 0xff;
	tempM = 0xff;
	
	/*����������λ����*/
	AZLeftLimitF = 1;
	AZRightLimitF = 1;

	/*��ʼ����λ�Ƕ�*/
	StationAz = AZ180;

	/*�õ���ǰ��λ���*/
	if(AzMidLimit == MotorLimitFlag)
	{
		Delay(5000);
		if(AzMidLimit == MotorLimitFlag)
		{
			tempMF = AzMidLimit;	
		}
		else
		{
			tempMF = 1;
		} 
	}
	else
	{
		tempMF = 1;	
	}

	if(AZRightLimit == MotorLimitFlag)
	{
		Delay(5000);
		if(AZRightLimit == MotorLimitFlag)
		{
			tempRF = AZRightLimit;	
		}
		else
		{
			tempRF = 1;
		} 
	}
	else
	{
		tempRF = 1;	
	}


	if(AZLeftLimit == MotorLimitFlag)
	{
		Delay(5000);
		if(AZLeftLimit == MotorLimitFlag)
		{
			tempLF = AZLeftLimit;	
		}
		else
		{
			tempLF = 1;
		} 
	}
	else
	{
		tempLF = 1;	
	}

	/*��������ת��15��*/
	TimeTest(2);
	MotorCtrl(RIGHTLEFT, StationAz + 15.0, GetAzS2());
	Delay(10000);

	/*�ж���λ�Ƿ�ߵ�ƽ����*/
	if(tempMF == MotorLimitFlag && AzMidLimit == MotorLimitFlag)
	{
		Delay(5000);
		if(AzMidLimit == MotorLimitFlag)
		{
			tempM = 1;
		}	
	}
	if(tempLF == MotorLimitFlag && AZLeftLimit == MotorLimitFlag)
	{
		Delay(5000);
		if(AZLeftLimit == MotorLimitFlag)
		{
			tempL = 1;	
		}
	}
	if(tempRF == MotorLimitFlag && AZRightLimit == MotorLimitFlag)
	{
		Delay(5000);
		if(AZRightLimit == MotorLimitFlag)
		{
			tempR = 1;	
		}
	}

	/*��������ת��15��*/
	TimeTest(2);
	MotorCtrl(RIGHTLEFT, StationAz - 15.0, GetAzS2());
	
	/*�ָ���λ������õ���ȷ����λ���*/
	AZLeftLimitF 	= tempL;
	AZRightLimitF 	= tempR;
	AzMidLimitF 	= tempM;
	TimeTest(2);
				
	return;
}
