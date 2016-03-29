/***********************************************************************
* 
* 时    间：2008-10-10
*
* 文 件 名：timer.c
*
* 版    本：AKD10P07(发布版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：C8051F020Timer相关函数,测试得timer3比timer0快一点点
*
*           1,定时器0主要用来计时，2，定时器3用来驱动步进电机；
*
* 注    意：调用这些函数时必需成对调用，详细参考相关代码；
*
* 修    改: 加入MotorCtrl（）函数，用于方位俯仰走到某一定角度
*
*           修改人：杨淳雯
*
*           修改时间：2008-11-3
*
* 修    改: 加入函数GotoElMid（）（便携式没有这个函数），GotoAzMid();
*
*           修改人：杨淳雯
*
*           修改时间：2008-11-24
*
* 修   改：加入三个得到校正的速度函数GetElS1(),GetAzS2(),GetAzS1();
*  
*          杨淳雯  2008-11-25
*
* 修   改：改进MotorCtrl（）函数，时间2009-3-25，杨淳雯
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


/*全局变量*/
INT16 data OverflowT0;								//计数溢出用
float data StationAz;								//地球站方位角度 
float data StationEl;								//地球站俯仰角度
float data PolarDst;								//极化目地角度
INT16 data PolarRightFlag;							//极化到位标志


float data StationAzLimit;

UINT8 data AzMidLimitF;				//2010-7-7
UINT8 data AZLeftLimitF;			//
UINT8 data AZRightLimitF;			//限位故障标致


xdata UINT8  TEMPAGE;


/*静态变量*/
static UINT16 data CountT0;
static UINT16 data AZMotorflag = 0;					//AZ电机启动标志
static UINT16 data ELMotorflag = 0;					//EL电机启动标志


/*25KG机器用*/								
static double data AZMotorPlusStep = 0.00225;	   	//方位电机一个脉冲步长，缺省
static double data ELMotorPlusStep = 0.00225;	   	//2009-3-25


static float data PolarDefinition  = 0.1;	  		//极化精确度
static float data PolarSrc;							//极化源角度


/*静态函数*/
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
* 函数原型：void Timer0Init(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：定时器0初始化;
*
***********************************************************************/
void Timer0Init(void)
{
	EA 	    =  0;
	SFRPAGE = TIMER01_PAGE;
	CKCON  |=  BIT2;			//定时器0 使用系统时钟,文档有错误！！！
	TMOD   &= ~BIT3;
	TMOD   &= ~BIT2;			//选择定时器功能
	TMOD   |=  BIT1;
	TMOD   &= ~BIT0;			//1 0 方式2：自动重装载的8 位计数器/定时器
	TL0     =  0x48;			//初值为72100us定时
	TH0     =  0x48;			//重载值
	TR0     =  0;            	//禁止定时器0
	EA      =  1;
}


/***********************************************************************
*
* 函数原型：void OpenTimer0Inrerupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：开启定时器0中断
*
***********************************************************************/
void OpenTimer0Interrupt(void)
{
	CountT0 	= 0;
	OverflowT0 	= 0;

	TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = TIMER01_PAGE;

	TF0   		= 0;			//溢出标志清0
	TR0   		= 1;			//开始计数
	ET0  	 	= 1;			//允许TF1 标志位（TCON.7）的中断请求

	SFRPAGE		= TEMPAGE;
}


/***********************************************************************
*
* 函数原型：void CloseTimer0Inrerupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：关闭定时器0中断
*
***********************************************************************/
void CloseTimer0Interrupt(void)
{
	TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = TIMER01_PAGE;

	TR0 	= 0;	
	ET0 	= 0;					//禁止定时器0 中断

	SFRPAGE		= TEMPAGE;
}


/***********************************************************************
*
*函数原型：void Timer0Interrupt(void)
*
*入口参数：无
*
*出口参数：无
*
*功能描述：定时器0中断函数,用来定时用，约每 23ms OVERFLOWT0加一
*
***********************************************************************/
void Timer0Interrupt(void) interrupt T0INTNUMBER
{	
    TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = TIMER01_PAGE;	  //页0	  120修改

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
* 函数原型：void Timer3Init(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：定时器3初始化，用于给电机脉冲
*
* 注：用的外部时钟，来给定时器3
*
***********************************************************************/
void Timer3Init(void)
{
    EA		  =  0;		     //IE所有页
    SFRPAGE   =  TMR3_PAGE;
    TMR3CN   &=  0x00;  //工作方式：16位自动重载，和定时器功能 
//	TMR3CN   |=  0x04;	//定时器3允许
	EIE2   	 &= ~BIT0;//先关闭中断
	EA  	  =  1;
}


/***********************************************************************
*
* 函数原型：void OpenTimer3Interrupt(UINT16 count)
*
* 入口参数：count，给电机的脉冲频率，可参考C8051F020手册
*
* 出口参数：无
*
* 功能描述：开启定时器0中断
*
***********************************************************************/
void OpenTimer3Interrupt(UINT16 count)
{
	SFRPAGE = TMR3_PAGE;
	RCAP3L	= 0x00ff & count;		 //注意
	count	= count>>8;
	RCAP3H	= 0x00ff & count;
	TMR3CN |= 0x04;	//定时器3允许
	EIE2   |= BIT0;  //开中断
}


/***********************************************************************
*
* 函数原型：void CloseTimer3Inrerupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：关闭定时器3中断
*
***********************************************************************/
static void CloseTimer3Interrupt(void)
{
	SFRPAGE =  TMR3_PAGE;
    TMR3CN &= ~BIT2;						//TR3=0,计数不允许
	EIE2   &= ~BIT0;						//不开中断
}


/***********************************************************************
*
* 函数原型：void Timer3Interrupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：定时器3中断函数,用来给电机方波脉冲
*
***********************************************************************/
void Timer3Interrupt(void) interrupt T3INTNUMBER
{	
	TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = TMR3_PAGE;	  //页1	120修改

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
		StationAz = AZ180 + AZlimit + 1.0;		//方位保护
		return;
	}

	if(AZDir == AZLEFT && AZLeftLimit == MotorLimitFlag \
		&& AZLeftLimitF > 0x3 && AZMotorflag == 1)
	{
		StationAz = AZ180 - AZlimit - 1.0;		//方位保护
		return;
	}



			
	if(ELMotorflag == 1)					   	//EL电机转动标志
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
		if(AZMotorflag == 1)					   //AZ电机转动标志
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
* 函数原型：void MotorFun(INT16 ElMode, INT16 AzMode, UINT16 speed)
*
* 入口参数：speed 给定时器用,控制电机速度
*           
*          ElMode, AzMode参考ELUP,ELDOWN,AZRIGHT,AZLEFT在timer.h中
*
* 出口参数：无
*
* 功能描述：电机转动控制
*
***********************************************************************/
void MotorFun(INT16 ElMode, INT16 AzMode, UINT16 speed)
{
	if(AzMode == MOTORSTOP && ElMode == MOTORSTOP)	//关定时器，关电机
	{
		CloseTimer3Interrupt();						//注这里可以加电机保护，如果电机都不转							
		return;
	}

	if(AzMode < MOTORSTOP)					  		//是否开方位电机
	{
		AZMotorflag = 1;
		if(AzMode == AZRIGHT)						//方位电机的方向
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
	
	if(ElMode < MOTORSTOP)					  	    //是否开俯仰电机
	{
		ELMotorflag = 1;
		if(ElMode == ELUP)							//俯仰电机方向
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

	OpenTimer3Interrupt(speed);			   			//电机开始转动
}


/***********************************************************************
*
* 函数原型：void MotorCtrl(INT16 DirFlag, float Angle, UINT16 speed)
*
* 入口参数：speed 给定时器用,控制电机速度
*           
*           DirFlag上下方向还是左右方向
*
*		    Angle目标角度
*
* 出口参数：无
*
* 功能描述：电机以speed速度走到方位或俯仰的Angle角度
*
***********************************************************************/
void MotorCtrl(INT16 DirFlag, float Angle, UINT16 speed)
{	
	if(DirFlag == UPDOWN)						  //如果为上下方向
	{
		if(Angle > ELUPlimit)					  //如果目标大于俯仰上限
		{
			Angle = ELUPlimit;
		}										  //如果目标小于俯仰下限
		if(Angle < ELDOWNlimit)
		{
			Angle = ELDOWNlimit;
		}
		if(Angle > StationEl)					  //如果目标角大于当前俯仰
		{
			MotorFun(ELUP, MOTORSTOP, speed);
			while(Angle > StationEl)
			{
				;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, speed);
		}
		else									  //如果目标角小于当前俯仰
		{
			MotorFun(ELDOWN, MOTORSTOP, speed);
			while(Angle < StationEl)
			{
				;
			}
			MotorFun(MOTORSTOP, MOTORSTOP, speed);
		}
	}//end : if(DirFlag == UPDOWN)

		
	if(DirFlag == RIGHTLEFT)					//如果为左右方向
	{
		if(Angle > AZlimit + AZ180)				//如果大于右限位	  	
		{
			Angle = AZlimit + AZ180;
		}										  
		if(Angle < AZ180 - AZlimit)				//如果小于左限位
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
* 函数原型：void GoAM(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线到方位中间限位
*
* 修    改：2010-07-16加入这个函数，用于收藏与测试时到中间限位
*
***********************************************************************/
void GoAM(void)
{
	/*当前是中间限位且上次中间限位正常*/
	if(AzMidLimit == MotorLimitFlag && AzMidLimitF > 0x10)
	{	
		StationAz = AZ180;
		return;
	}
	/*如果中间限位正常则找中间限位*/
	if(AzMidLimitF > 0x10)
	{
		/*如果方位大于180度，则向左转动*/
		if(StationAz > 180.0)
		{
			TimeTest(2);	//走电机前加入延时
			MotorFun(MOTORSTOP, AZLEFT, GetAzS2());			 	//天线向左转动（从天线后面看）
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
			
			/*如果找到中间限位则返回*/
			if(AzMidLimit == MotorLimitFlag)
			{
				StationAz = AZ180;
				return;
			}
			/*回到180度，中间限位故障*/
			else
			{
				TimeTest(2);	//走电机前加入延时
				MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
				AzMidLimitF = 1;
				WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);	
			}	
		}
		/*如果角度小于180度则向右转*/
		else
		{
			TimeTest(2);	//走电机前加入延时
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
				TimeTest(2);	//走电机前加入延时
				MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
				AzMidLimitF = 1;
				WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);
					
			}			
		}	
	}
	/*如果中间限位不正常则直接走到中间180度*/
	else
	{
		TimeTest(2);	//走电机前加入延时
		MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());		
	}
	StationAz = AZ180;
	return;
}


/***********************************************************************
*
* 函数原型：void GotoAzMid(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线到AZ中间限位
*
* 修    改：2008-11-24加入这个函数
*
***********************************************************************/
void GotoAzMid(void)
{
	/*情况一：如果中间限位没有故障而且目前正在中间限位*/
	if(AzMidLimit == MotorLimitFlag && AzMidLimitF > 0x10)
	{	
		StationAz = AZ180;
		StationAzLimit = StationAz;
		return;
	}

	/*情况二：三个限位都故障*/
	if(AZLeftLimitF == 1 && AZRightLimitF == 1 && AzMidLimitF == 1)
	{
		StationAz = AZ180;
		StationAzLimit = StationAz;
		return;
	}

	/*情况三：如果左限位没有故障*/
	if(AZLeftLimitF > 0x10)
	{
		if(GotoAzMidL())
		{
			return;		
		}
	}

	/*情况四：如果右限位没有故障*/
	if(AZRightLimitF > 0x10)
	{
		if(GotoAzMidR())
		{
			return;		
		}	
	}

	/*情况五：如果中间限位没有故障*/
	if(AzMidLimitF > 0x10)
	{
		GotoAzMidM();
	}
	return;
}


/***********************************************************************
*
* 函数原型：UINT8 GotoAzMidL(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线到AZ中间限位,左限位正常情况
*
* 修    改：2010-7-9加入这个函数
*
***********************************************************************/
static UINT8 GotoAzMidL(void)
{
	UINT8 tempL;
	UINT8 tempR;
	UINT8 tempM;
	UINT8 returnvalue;

	/*保存以前的限位情况*/
	tempL = AZLeftLimitF;
	tempR = AZRightLimitF;
	tempM = AzMidLimitF;
	
	/*屏蔽左右限位功能*/
	AZLeftLimitF = 1;
	AZRightLimitF = 1;

	StationAz = AZ180;

	/*向左走90度，如果碰到限位则停止*/
	TimeTest(2);	//走电机前加入延时
	MotorFun(MOTORSTOP, AZLEFT, GetAzS2());			 	//天线向左转动（从天线后面看）
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

	/*情况一：找到中间限位*/
	if(AzMidLimit == MotorLimitFlag && tempM > 0x10)	//如果是中间限位则确定方位值
	{
		StationAz = AZ180;
		StationAzLimit = StationAz;
		AZLeftLimitF = tempL;
		AZRightLimitF = tempR;
//		return 1;
		returnvalue = 1;
	}

	/*情况二：找到左限位*/
	if(AZLeftLimit == MotorLimitFlag)
	{
		/*如果中间限位正常，则再找中间限位*/
		if(tempM > 0x10)
		{
			StationAz = 90.0;
			TimeTest(2);
 			MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());	//天线向左转动（从天线后面看）
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
			
			/*如果找到中间限位则正常返回*/
			StationAz = AZ180;
			StationAzLimit = StationAz;
			AZLeftLimitF = tempL;
			AZRightLimitF = tempR;

			/*如果没有找到中间限位，则将中间限位标志为故障*/
			if(AzMidLimit != MotorLimitFlag)
			{	
				AzMidLimitF = 1;		
			}
//			return 1;
			returnvalue = 1;	
		}
		/*如果中间限位故障，则直接走到中间限位*/
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

	/*情况三：什么都没有找到*/
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
* 函数原型：UINT8 GotoAzMidLR(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线到AZ中间限位,左限位正常情况
*
* 修    改：2010-7-9加入这个函数
*
***********************************************************************/
static UINT8 GotoAzMidR(void)
{
	UINT8 tempL;
	UINT8 tempR;
	UINT8 tempM;
	UINT8 returnvalue;

	/*保存以前的限位情况*/
	tempL = AZLeftLimitF;
	tempR = AZRightLimitF;
	tempM = AzMidLimitF;
	
	/*屏蔽左右限位功能*/
	AZLeftLimitF = 1;
	AZRightLimitF = 1;

	StationAz = AZ180;

	/*向右走90度，如果碰到限位则停止*/
	TimeTest(2);	//走电机前加入延时
	MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());			 //天线向右转动（从天线后面看）
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

	/*情况一：找到中间限位*/
	if(AzMidLimit == MotorLimitFlag && tempM > 0x10)	//如果是中间限位则确定方位值
	{
		StationAz = AZ180;
		StationAzLimit = StationAz;
		AZLeftLimitF = tempL;
		AZRightLimitF = tempR;
//		return 1;
		returnvalue = 1;
	}

	/*情况二：找到右限位*/
	if(AZRightLimit == MotorLimitFlag)
	{
		/*如果中间限位正常，则再找中间限位*/
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
			/*如果找到中间限位则正常返回*/
			StationAz = AZ180;
			StationAzLimit = StationAz;
			AZLeftLimitF = tempL;
			AZRightLimitF = tempR;

			/*如果没有找到中间限位，则将中间限位标志为故障*/
			if(AzMidLimit != MotorLimitFlag)
			{	
				AzMidLimitF = 1;	
			}
//			return 1;
			returnvalue = 1;	
		}
		/*如果中间限位故障，则直接走到中间限位*/
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

	/*情况三：什么都没有找到*/
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
* 函数原型：UINT8 GotoAzMidLM(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：天线到AZ中间限位,左限位正常情况
*
* 修    改：2010-7-9加入这个函数
*
***********************************************************************/
static UINT8 GotoAzMidM(void)
{
	StationAz = AZ180;

	/*情况一：向右走90度，如果碰到限位则停止*/
	TimeTest(2);	//走电机前加入延时
	MotorFun(MOTORSTOP, AZRIGHT, GetAzS2());			 //天线向左转动（从天线后面看）
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

	/*情况二：向左走180度，如果碰到中间限位则停止*/
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

	/*情况三：什么也没有碰到*/
	TimeTest(2);
	MotorCtrl(RIGHTLEFT, 180.0, GetAzS2());
	StationAz = AZ180;
	AzMidLimitF = 1;						//中间限位故障
	WriteEEPROM(AzMidLimitF, AzMidLimitFAddr);
	return 0;
}


/***********************************************************************
*
* 函数原型：void GetAzS1(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：得到天线的方位速度1
*
* 修    改：2008-11-25加入这个函数
*
***********************************************************************/
UINT16 GetAzS1(void)
{
	unsigned int speedAZR;
	unsigned int tempAZ = (unsigned int)ReadEEPROM(rightFOUR);

	/*得到校正方位速度1*/
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
* 函数原型：void GetAzS2(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：得到天线的方位速度2
*
* 修    改：2008-11-25加入这个函数
*
***********************************************************************/
UINT16 GetAzS2(void)
{
	unsigned int speedAZR;
	unsigned int tempAZ = (unsigned int)ReadEEPROM(rightFIVE);

	/*得到校正方位速度2*/
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
* 函数原型：void GetElS1(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：得到天线的俯仰速度1
*
* 修    改：2008-11-25加入这个函数
*
***********************************************************************/
UINT16 GetElS1(void)
{
	unsigned int speedELR;
	unsigned int tempEL = (unsigned int)ReadEEPROM(rightTHREE);

	/*得到俯仰速度*/
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
* 函数原型：void TestStor(char flag)
*
* 入口参数：flag 用来判断是哪种测试
*
* 出口参数：无
*
* 功能描述：机器出厂前用的，可以参考相关手册
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
	if(flag == 'L')									 //判断是否左收藏
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
	GoAM();									//到中间限位
	GotoPolarAngle(90.0);							//极化到零

	if(tempS > 24.0)
	{
		tempS = 2.0;	
	}
	if(flag == 'L')									//如果从右边过来，再继续转3度因为中间限位较偏
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
	if(tempS > 20.0)	//2009/8/6由于倾斜仪的问题（最大60度）
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
		MotorCtrl(UPDOWN, tempS, GetElS1());		//下到斜坡的角度
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

	OpenTimer0Interrupt();						   //等待十秒
	while(OverflowT0 < 100)
	{
		;
	}
	CloseTimer0Interrupt();

	TimeTest(2);
	MotorCtrl(UPDOWN, 35.0, GetElS1());			  //2008-12-30修改

	ELDOWNlimit	= ELDOWNlimitDef;

	return;
}


/***********************************************************************
*
* 函数原型：void Timer2Init(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：定时器2初始化，极化用
*
***********************************************************************/
void Timer2Init(void)
{
    EA			=  0;		     //IE所有页
    SFRPAGE   	=  TMR2_PAGE;
	TMR2CF    	=  0x08;	 //系统时钟
    TMR2CN     &= ~0x03;  //工作方式：16位自动重载，和定时器功能
	
	ET2			=  0;       //不开中断
	RCAP2L		=  0xFF;   // 把计数值的低8位赋给定时器4捕捉寄存器低字节  
	TMR2L		=  RCAP2L;	  //赋值给定时器4低字节 
	RCAP2H		=  0x7F;	 //把计数值的高8位赋给定时器4捕捉寄存器高字节 
	TMR2H		=  RCAP2H;
   
	SFRPAGE   	=  TMR2_PAGE;
	TMR2CN     |=  0x04;	//定时器2允许
	EA  		=  1;
}


/***********************************************************************
*
* 函数原型：void OpenTimer2Interrupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：开定时器2中断
*
***********************************************************************/
void OpenTimer2Interrupt(void)
{
	SFRPAGE = TMR2_PAGE;
	TR2 = 1;			//允许计数
	ET2	= 1;			//开中断
}


/***********************************************************************
*
* 函数原型：void CloseTimer2Interrupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：关定时器2中断
*
***********************************************************************/
void CloseTimer2Interrupt(void)
{
	SFRPAGE   = TMR2_PAGE;
	ET2	= 0;			//不开中断
	TR2 = 0;			//不允许计数
}


/***********************************************************************
*
* 函数原型：void Timer2Interrupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：定时器2中断函数
*
***********************************************************************/
void Timer2Interrupt(void) interrupt T2INTNUMBER
{

	TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = TMR2_PAGE;	  //页0	  120修改

	TF2 = 0;

	POLAR_STOP;											//先停止极化
	PolarSrc = GetPolarAngle();							//极化向右转，角度增加，向左转角度减小

	if(fabs(PolarSrc - PolarDst) < PolarDefinition)
	{
		POLAR_STOP;
		PolarRightFlag = 1;								//极化角到位
		CloseTimer2Interrupt();
		return;
	}
	if(PolarSrc > PolarDst)								//当前极化角大于目标角度
	{
//		POLAR_RIGHT;
		POLAR_LEFT;
	}
	else
	{
//		POLAR_LEFT;
		POLAR_RIGHT;
	}

	SFRPAGE	= TEMPAGE;  //120修改
}


/***********************************************************************
*
* 函数原型：void AzLimitTest(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：方位限位出错检查
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

	/*保存以前的限位情况*/
	tempL = 0xff;
	tempR = 0xff;
	tempM = 0xff;
	
	/*屏蔽左右限位功能*/
	AZLeftLimitF = 1;
	AZRightLimitF = 1;

	/*初始化方位角度*/
	StationAz = AZ180;

	/*得到当前限位情况*/
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

	/*天线向右转动15度*/
	TimeTest(2);
	MotorCtrl(RIGHTLEFT, StationAz + 15.0, GetAzS2());
	Delay(10000);

	/*判断限位是否高电平故障*/
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

	/*天线向左转动15度*/
	TimeTest(2);
	MotorCtrl(RIGHTLEFT, StationAz - 15.0, GetAzS2());
	
	/*恢复方位保护与得到正确的限位情况*/
	AZLeftLimitF 	= tempL;
	AZRightLimitF 	= tempR;
	AzMidLimitF 	= tempM;
	TimeTest(2);
				
	return;
}
