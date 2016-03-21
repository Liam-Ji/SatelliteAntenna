/***********************************************************************
* 
* 时    间：2008-10-13
*
* 文 件 名：uart0.c
*
* 版    本：AKD10P02/AKD09P02(测试版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：C8051F020串口0,1的相关功能函数
*
*           1，串口0主要与GPS，倾斜仪通信，还可以括搌
*
*           2，串口1主要与上位机通信功能，有无线切换功能
*
***********************************************************************/
#include "uart.h"
#include "c8051f120.h"
#include "timer.h"
#include "station.h"
#include "rweeprom.h"


/*全局变量*/
char  xdata Uart0R[MAXLENGTH];				//串口0接收数组
char  xdata Uart0T[MAXLENGTH];				//串口0发送数组
float xdata GradientY;						//倾斜仪输出的倾斜度Y
float xdata GradientRightR = 0.0;			//倾斜仪输出的倾斜度校正度数
int   xdata GradientNormal;					//倾斜仪正常


int   xdata GPSNormal;						//GPS正常
float xdata GPSLong;						//GPS输出的经度
BOOL  xdata GPSEastFlag;					//GPS东经标志
float xdata GPSLat;							//GPS输出的纬度
BOOL  xdata GPSNorthFlag;					//GPS北纬标志


char  xdata Uart1ReceData[MAXLENGTH] = {0};	//串口1接收数据
char  xdata Uart1SendData[MAXLENGTH] = {0};	//串口1发送数据
UINT8 xdata Uart1ReceDataLength 	 = 0;	//当前串口1要接收的数据缓冲长度
UINT8 xdata Uart1SendDataLength 	 = 0;	//当前串口1要发送的数据序列
UINT8 xdata Uart1TXMAXLenth 	 	 = 0;	//串口1要发送的最大长度
UINT8 xdata NumberOfComand 			 = 0;	//命令个数 


/*静态变量*/
static int  data Uart0Enflag;
static int  data Uart0RNum;
static int  data Uart0TNum;
static char data Uart0StartF;
static char data Uart0TxFlag;


/*静态函数*/
static void SelectGradient(void);
static void SelectGPS(void);
static void SelectXinBiao(void);
static void OpenUart1(void);
static void CloseUart1(void);
static void SetXinBiaoFreqNum(double Freq);		//2009-2-17
static void PutXinBiaoS(void);


/*有无线切换相关*/
static int  GetG8(void);
static char GetG8Flag(void);
static void SetFlag(char);
static void InitWire(void);


/***********************************************************************
*
* 函数原型：void Uart0Init(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口0初始化,定时器1作为时钟源，初值自动重装
*
***********************************************************************/
void Uart0Init(void)
{
	UINT16 Counter;  //定义unsigned short int Counter  
	Counter = 65536-(SYSCLK/UARTBR19200/16);	  //原来是16，注意
	EA=0;  			       //所有页
   
	SFRPAGE   = TMR4_PAGE; 
	TMR4CF    |= 0x08;	 //系统时钟
	TMR4CN    &= 0xFC;  //工作方式：16位自动重载，和定时器功能 

	RCAP4L |= Counter%256;   // 把计数值的低8位赋给定时器4捕捉寄存器低字节 
	TMR4L=RCAP4L;	         //赋值给定时器4低字节 
	RCAP4H |=Counter/256;
 
	TMR4H=RCAP4H;	

	SFRPAGE   = UART0_PAGE;
	SCON0     |= 0x50;	//方式1：8 位UART，可变波特率 ,	并且UART0 接收允许 
	SSTA0     |= 0x1F;	//定时器4作为接收和发送时钟源（波特率发生器），禁止波特率加倍


	SFRPAGE   = TMR4_PAGE;
	TMR4CN    |= 0x04;	//定时器4允许

	ES0	   = 0;		   //开串口0中断，所有页	  
	EA=1;
}


/***********************************************************************
*
* 函数原型：void OpenUart0(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口0开
*
***********************************************************************/
void OpenUart0(void)
{
	ES0	= 1;							//打开串口0中断
	SFRPAGE   = TMR4_PAGE;
	TMR4CN    &= ~0x80;		//溢出清空  
	TMR4CN    |= 0x04;	   //允许定时器4
}


/***********************************************************************
*
* 函数原型：void CloseUart0(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口0关
*
***********************************************************************/
void CloseUart0(void)
{
	SFRPAGE   = TMR4_PAGE;
	TMR4CN    &= ~0x04;	//禁止定时器4
	ES0	= 0;							//关闭串口0中断
}


/***********************************************************************
*
* 函数原型：void Uart0Interrupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口0中断
*
* 修    改： 2009/9/11加入对接收计数变量的保护
*
***********************************************************************/
void Uart0Interrupt(void) interrupt UART0INTNUMBER
{
	TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = UART0_PAGE;

	if(TI0)							  	//发送中断响应
	{
		TI0 = 0;
		if(Uart0TNum > MAXLENGTH - 1)
		{
			Uart0TNum = 0;
		}
		if(Uart0TxFlag == TRUE)
		{
			SBUF0 = Uart0T[Uart0TNum++];	
		}
	}
	
	if(RI0)								//接收中断响应
	{
		RI0 = 0;
		if(Uart0RNum < MAXLENGTH)		//溢出保护2009.9.11
		{
			if(SBUF0 == Uart0StartF)	//如果串口0接收寄存器中的字符是$
			{
				Uart0Enflag = 1;		//理解为“使能”标志，用于判断是否开始存储数据
			}
			if(Uart0Enflag == 1)
			{
				Uart0R[Uart0RNum++] = SBUF0;
			}
		}
		else
		{
			Uart0RNum = 0;				 //数组溢出时，将数组下标置为0，重新开始存储
		}
	}
	SFRPAGE = TEMPAGE;  //120修改
}


/***********************************************************************
*
* 函数原型：void SelectGPS(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口0切换到GPS，并且将波特率切换到4800
*
***********************************************************************/
static void SelectGPS(void)
{ 
	UINT16 Counter;  //定义unsigned short int Counter 

	CloseUart0();
		
	SFRPAGE = 0x0F;	  //120修改	 页F
//	SELECT0H;		  //P6^5=1
//	SELECT1L;		  //P6^6=0	 选择GPS
	SELECT0H;		  //P6^5=1
	SELECT1H;		  //P6^6=1	 选择GPS

    SFRPAGE   = TMR4_PAGE;
	TMR4CN    &= ~0x04;	//禁止定时器4
							 
	Counter=65536-(SYSCLK/UARTBR9600/16);	
   	RCAP4L=Counter&0x00ff;   // 把计数值的低8位赋给定时器4捕捉寄存器低字节 
	TMR4L=RCAP4L;	         //赋值给定时器4低字节 
	Counter=Counter>>8;
	RCAP4H=Counter&0x00ff;	 //把计数值的高8位赋给定时器4捕捉寄存器高字节 
	TMR4H=RCAP4H;
							
	TMR4CN    &= ~0x80;		//溢出清空  
	TMR4CN    |= 0x04;	   //允许定时器4 

	OpenUart0();		   //打开串口中断，准备接收GPS信息
}


/***********************************************************************
*
* 函数原型：void SelectGradient(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口0切换到倾斜仪，并且将波特率切换到9600
*
***********************************************************************/
static void SelectGradient(void)
{
	UINT16 Counter;  //定义unsigned short int Counter  
	
	CloseUart0(); 	

	SFRPAGE = 0x0F;	  //120修改	 页F
	SELECT0L;
	SELECT1L;

    SFRPAGE   = TMR4_PAGE;
	TMR4CN    &=0xFB;	//禁止定时器4
							 
	Counter=65536-(SYSCLK/UARTBR9600/16);	
   	RCAP4L=Counter&0x00ff;   // 把计数值的低8位赋给定时器4捕捉寄存器低字节 
	TMR4L=RCAP4L;	         //赋值给定时器4低字节 
	Counter=Counter>>8;
	RCAP4H=Counter&0x00ff;	 //把计数值的高8位赋给定时器4捕捉寄存器高字节 
	TMR4H=RCAP4H;
 
	TMR4CN    |= 0x04;	   //允许定时器4 

	OpenUart0();
}


/***********************************************************************
*
* 函数原型：void SelectXinBiao(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：选择信标接收机

*
* 加入时间：2009-1-4，杨淳雯
*
***********************************************************************/
static void SelectXinBiao(void)
{
	UINT16 Counter;  //定义unsigned short int Counter  
	
	CloseUart0(); 	

	SFRPAGE = 0x0F;	  //120修改	 页F
	SELECT0L;
	SELECT1H;

    SFRPAGE   = TMR4_PAGE;
	TMR4CN    &=0xFB;	//禁止定时器4
							 
	Counter=65536-(SYSCLK/UARTBR19200/16);	
   	RCAP4L=Counter&0x00ff;   // 把计数值的低8位赋给定时器4捕捉寄存器低字节 
	TMR4L=RCAP4L;	         //赋值给定时器4低字节 
	Counter=Counter>>8;
	RCAP4H=Counter&0x00ff;	 //把计数值的高8位赋给定时器4捕捉寄存器高字节 
	TMR4H=RCAP4H;
 
	TMR4CN    |= 0x04;	   //允许定时器4 

	OpenUart0();
}


/***********************************************************************
*
* 函数原型：float SelectFreq(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：给信标接收机写频率字（南京宽超信标接收机用）
*
* 加入时间：2009-1-4，杨淳雯
*
* 说    明：参考相关信标接收机说明文档，
*
***********************************************************************/
void SetXinBiaoFreqKC(double freq)
{
	int i = 5;
	while(i--)
	{
		Delay(10000);
		SetXinBiaoFreqNum(freq);
		if(XinBiaoFlagNum == TRUE)
		{
;//			PutXinBiaoS();
		}
		if(XinBiaoRecNormalFlag == TRUE)
		{
			break;
		}	
	}
}


/***********************************************************************
*
* 函数原型：void SetXinBiaoFreqNum(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：给信标接收机写频率字（南京宽超信标接收机用）
*
* 修    改：（2009-3-5）
*
***********************************************************************/
static void SetXinBiaoFreqNum(double Freq)
{
	int FreqT;

	FreqT = (int)Freq;

	Uart0T[0] = '<';
	Uart0T[1] = 'F';
	Uart0T[2] = FreqT / 1000 + '0';
	FreqT %= 1000;
	Uart0T[3] = FreqT / 100 + '0';
	FreqT %= 100;
	Uart0T[4] = FreqT / 10 + '0';
	FreqT %= 10;
	Uart0T[5] = FreqT % 10 + '0';
	Uart0T[6] = '.';

	FreqT = (int)Freq;
	FreqT = (Freq - FreqT) * 1000;

	FreqT %= 1000;
	Uart0T[7] = FreqT / 100 + '0';
	FreqT %= 100;
	Uart0T[8] = FreqT / 10 + '0';
	FreqT %= 10;
	Uart0T[9] = FreqT % 10 + '0';

	Uart0T[10] = 0xd;
	Uart0T[11] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '<';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;
	SelectXinBiao();
	Delay(40);

	SFRPAGE = UART0_PAGE;
	SBUF0 = Uart0T[0];							  	//开始发送
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 'T')	  	//得到信标接收机的返回值，放入Uart0R[]中
	{
		if(Uart0TNum > 11)
		{
			 Uart0TxFlag = FALSE;				 	//停止发送，关发送标志
			 Uart0Enflag = 0;						//2009/9/8
		}
	}
	CloseUart0();
	CloseTimer0Interrupt();
	TI0 = 0;
	Uart0Enflag = 0;
	Uart0TxFlag = FALSE;
	if(OverflowT0 > 9)
	{
		XinBiaoRecNormalFlag = FALSE;			   	//判断信标为故障
	}
	else
	{
		XinBiaoRecNormalFlag = TRUE;				//判断信标志着为正常		
	}	
}


/***********************************************************************
*
* 函数原型：void GetXinBiaoVER(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：给信标接收机写频率字（南京宽超信标接收机用）
*
* 修    改：（2009-3-5）
*
***********************************************************************/
void GetXinBiaoVER(void)
{
	char i;
	char j;

	Uart0T[0] = '<';
	Uart0T[1] = 'V';
	Uart0T[2] = 'E';
	Uart0T[3] = 'R';
	Uart0T[4] = '?';
	Uart0T[5] = 0xd;
	Uart0T[6] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '<';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;

	SelectXinBiao();
	Delay(40);

	SFRPAGE = UART0_PAGE;
	SBUF0 = Uart0T[0];							  	//开始发送
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 'K')	  	//得到gradient的值，放入Uart0R[]中
	{
		i = SBUF0;
		if(i != j)
		{
			if((i == 'F') && (j == 'R'))
			{
				break;
			}
			j = i;
		}
		if(Uart0TNum > 6)
		{
			 Uart0TxFlag = FALSE;				 	//停止发送，关发送标志
			 Uart0Enflag = 0;
		}
	}
	CloseUart0();
	CloseTimer0Interrupt();

	Uart0Enflag = 0;
	TI0 = 0;
	Uart0TxFlag = FALSE;
	if(OverflowT0 < 10)
	{
		XinBiaoFlagNum = TRUE;						//判断信标志着为正常		
	}
	else
	{
		XinBiaoFlagNum = FALSE;
	}	
}


/***********************************************************************
*
* 函数原型：void GetXinBiaoVER(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：给信标接收机写频率字（南京宽超信标接收机用）
*
* 修    改：（2009-3-5）
*
***********************************************************************/
void GetXinBiaoVorX(void)
{
	Uart0T[0] = '<';
	Uart0T[1] = 'C';
	Uart0T[2] = 0xd;
	Uart0T[3] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '>';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;
	SelectXinBiao();
	Delay(40);

	SFRPAGE = UART0_PAGE;
	SBUF0 = Uart0T[0];							  	//开始发送
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 'V' && SBUF0 != 'X')	  	//得到gradient的值，放入Uart0R[]中
	{
		if(Uart0TNum > 3)
		{
			 Uart0TxFlag = FALSE;				 	//停止发送，关发送标志
			 Uart0Enflag = 0;
		}
	}
	XinBiaoVorX = SBUF0;
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag = 0;
	TI0 = 0;
	Uart0TxFlag = FALSE;				
}


/*以后可能用不到*/
/*
static void PutXinBiaoS(void)
{
	Uart0T[0] = '<';
	Uart0T[1] = 'S';
	Uart0T[2] = 0xd;
	Uart0T[3] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '<';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;
	SelectXinBiao();
	Delay(40);
	SBUF0 = Uart0T[0];							  	//开始发送
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 0xa)	  	//得到gradient的值，放入Uart0R[]中
	{
		if(Uart0TNum > 3)
		{
			 Uart0TxFlag = FALSE;				 	//停止发送，关发送标志
			 Uart0Enflag = 0;
		}
	}
	CloseUart0();
	CloseTimer0Interrupt();
	TI0 = 0;
	Uart0Enflag = 0;
	Uart0TxFlag = FALSE;				
}
*/
/***********************************************************************
*
* 函数原型：float GetKCAGC(void)
*
* 入口参数：无
*
* 出口参数：float agc，从串口得到AGC
*
* 功能描述：得到信标接收机AGC
*
***********************************************************************/
float GetKCAGC(void)
{
	float agc;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '>';
	
	OpenTimer0Interrupt();
	SelectXinBiao();
	while((OverflowT0 < 10) && Uart0RNum < 7)		//得到gradient的值，放入Uart0R[]中
	{
		;
	}
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag = 0;
	if(OverflowT0 > 9 || Uart0RNum < 7)
	{
		return 0.0;
	}
	if(Uart0R[1] == 'D')
	{
		return 0.5;
	}
	agc = (Uart0R[2] - '0') + 		\
		  (Uart0R[4] - '0') * 0.1 + \
		  (Uart0R[5] - '0') * 0.01;
	if(Uart0R[1] == 'T')
	{
		return AGC;	
	}

	return agc;				
}


/***********************************************************************
*
* 函数原型：void ReadGradient(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：打开串口0中断,接收倾斜仪数据
*
***********************************************************************/
void ReadGradient(void)
{
	Uart0Enflag		= 0;
	GradientNormal 	= 0;	

	for(Uart0RNum = MAXLENGTH-1; Uart0RNum >= 0; Uart0RNum--)
		Uart0R[Uart0RNum] = '\0';
	Uart0RNum 		= 0;
	Uart0StartF		= '$';							//倾斜仪开始标志
	OpenTimer0Interrupt();
	SelectGradient();

	while(OverflowT0 < 15 && Uart0RNum < 12)		//得到gradient的值，放入Uart0R[]中
	{
		;
	}
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag	= 0;

/*
	if((Uart0R[3] == '+') || (Uart0R[3] == '-'))
	{
		GradientY = (Uart0R[4] - '0') * 10.0  + \		 
					(Uart0R[5] - '0')		  + \
					(Uart0R[6] - '0') * 0.1   + \
					(Uart0R[7] - '0') * 0.01;
	}
*/
	if((Uart0R[6] == '+') || (Uart0R[6] == '-'))
	{
		GradientY = (Uart0R[7] - '0') * 10.0  + \		 
					(Uart0R[8] - '0')		  + \
					(Uart0R[9] - '0') * 0.1   + \
					(Uart0R[10] - '0') * 0.01;
		if(Uart0R[6] == '-')
			GradientY = -GradientY;
	}
	
	if(Uart0R[0] != '$' 	|| \
		Uart0RNum < 10  	|| \
		OverflowT0 > 14  	|| \
		GradientY > 104.5)
	{
		GradientNormal = 0;
		GradientY = StationEl;
	}
	else
	{
//		if((Uart0R[3] == '+') || (Uart0R[3] == '-'))
//		{
//			if(Uart0R[3] == '-')
//			{
//				GradientY = -GradientY;	
//			}
//		}
//		else if((Uart0R[1] == '+') || (Uart0R[1] == '-'))
//		{
//			if(Uart0R[1] == '-')
//			{
//				GradientY = -GradientY;	
//			}
//		}

		GradientY += 44.5;				//151227  由于机器倾斜仪安装位置的该变，31.6改为44.5
		GradientY += GradientRightR;
		GradientNormal = 1;
	} 
	return;
}


/***********************************************************************
*
* 函数原型：void ReadGPS(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：打开串口0中断，接收GPS数据
*
***********************************************************************/
void ReadGPS(void)
{
	Uart0Enflag = 0;
	Uart0RNum 	= 0;							  //串口0接受到的字符数，此处初始化为0
	GPSNormal 	= 0;
	Uart0StartF	= '$';							  //GPS信息头标志
	
	OpenTimer0Interrupt();
	SelectGPS();

	SFRPAGE = UART0_PAGE;
	
	while((OverflowT0 < 20) && !(SBUF0 == 0x0a && Uart0RNum > 0))	//读取串口0的数据时间约为20s，20s内如果接收到字符0x0a(回车标志)并且数组元素个数大于0，也会直接退出循坏
	{
		;
	}
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag	= 0;

	if(Uart0R[42] == 'E')
	{
		GPSEastFlag = EAST;
	}
	if(Uart0R[42 ] == 'W')
	{
		GPSEastFlag = WEST;
	}
	if(Uart0R[29] == 'N')
	{
		GPSNorthFlag = NORTH;
	}
	if(Uart0R[29] == 'S')
	{
		GPSNorthFlag = SOUTH;
	}
	GPSLat = (Uart0R[19] - '0') *   \
		10.0 + (Uart0R[20] - '0') + \
		(Uart0R[21] - '0') * 0.1 +  \
		(Uart0R[22] - '0') * 0.01;
	
	GPSLong = (Uart0R[31] - '0') *   \
		100.0 + (Uart0R[32] - '0') * \
		10.0 + (Uart0R[33] - '0') +  \
		(Uart0R[34] - '0') * 0.1 +   \
		(Uart0R[35] - '0') * 0.01;
	
	if(Uart0R[17] == 'V' && OverflowT0 < 15)
	{
		GPSNormal = 2;
		return;
	}
	if(Uart0R[17] == 'A' && OverflowT0 < 15)
	{
		GPSNormal = 1;
	}
}


/***********************************************************************
*
* 函数原型：void Uart1Init(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口1初始化,定时器4作为时钟源，初值自动重装
*
***********************************************************************/
void Uart1Init(void)
{
	EA=0;			      //禁止所有终端  
  	SFRPAGE = TIMER01_PAGE;  

   	CKCON &= 0xEC;	//12分频
    TMOD |= 0x20;  //T1工作方式2：8位自动重载
	TMOD &=~0x40; 	//定时器功能 
	TH1  = 256-(SYSCLK/12/UARTBR9600/2);	//根据波特率设置定时器1重载值	  
   	TL1  = TH1;
	
	SFRPAGE = UART1_PAGE;

    SCON1 |= 0x10;	//方式0：波特率可编程的8 位UART1,并且UART1 接收允许

	SFRPAGE = TIMER01_PAGE;						
   	TCON |= 0x40;   //定时器1允许      
	OpenUart1();
	EA=1;
}

/***********************************************************************
*
* 函数原型：void OpenUart0(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：打开串口1中断
*
***********************************************************************/
static void OpenUart1(void)
{
	SFRPAGE = UART1_PAGE;
	SCON1 &= ~BIT0;
	SCON1 &= ~BIT1;
	EIE2  |=  0x40;									//开启串口1
}							   	

/***********************************************************************
*
* 函数原型：void CloseUart0(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：关闭串口1中断
*
***********************************************************************/
static void CloseUart1(void)
{
	EIE2 &= ~0x40;			 //关闭串口1中断
}


/***********************************************************************
*
* 函数原型：void Uart1Interrupt(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口1中断，发送Uart1SendDataLength长度的Uart1SendData
*
***********************************************************************/
void Uart1Interrupt(void) interrupt UART1INTNUMBER
{
	TEMPAGE = SFRPAGE;		  //120修改
	SFRPAGE = UART1_PAGE;	  //页1	  120修改、

	if((SCON1 & BIT0) == BIT0)
	{
		SCON1 &= ~BIT0;//清除RI1
		//--------------------------------------------------------------------------
		if(WireFlag > 0)
		{
			WireBuff[len] = SBUF1;
			len++;
		}
		//--------------------------------------------------------------------------
		else
		{
			if(Uart1ReceDataLength < MAXLENGTH)
			{
				Uart1ReceData[Uart1ReceDataLength] = SBUF1;
				if(Uart1ReceData[Uart1ReceDataLength] == '\r')
				{
					NumberOfComand++;
				}
				Uart1ReceDataLength++;
			}
		}
	}
	if((SCON1&BIT1)==BIT1)
	{
		SCON1&=~BIT1;//清除TI1

		if((WireFlag == 1) && (WireBuff[0] != '*') && (WireBuff[len - 1] != 'z'))
		{
			Delay(1825);
			SBUF1 = 'z';
		}
		else if((WireFlag == 2) && (WireBuff[1] != '\n'))
		{
			SBUF1 = '\n';
		}
		else if((WireFlag == 3) && (WireBuff[3] != '\n'))
		{
			SBUF1 = Buff[len1++];
		}
		else if((WireFlag == 4) && (len1 <= len2))
		{
			SBUF1 = WireBuff[len1++];
		}
		else
		{
			Uart1SendDataLength++;
			if(Uart1SendDataLength>=Uart1TXMAXLenth)
			{
				Uart1SendDataLength=0;
				Uart1TXMAXLenth = 0;
			}
			else
			{
				SBUF1 = Uart1SendData[Uart1SendDataLength];			
			}
		}
	}
	SFRPAGE = TEMPAGE;  //120修改
}


/***********************************************************************
*
* 函数原型：void Uart1TX(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：串口1发送Uart1SendData中的数据 ，注意如果要循环发送一串数据一
*
* 定要等待Uart1TXMAXLenth	变成0
*
***********************************************************************/
void Uart1TX(void)
{
	OpenUart1();
	Uart1SendDataLength = 0;

	SFRPAGE	= UART1_PAGE;
	if(Uart1TXMAXLenth > 0)							//当前缓冲区有发送数据
	{
		SBUF1 = Uart1SendData[Uart1SendDataLength];	//从Uart1SendDataLength发起
	}
	while(Uart1TXMAXLenth != 0)
		;											//等待发送结束
}


/*以下是有无线切换相关函数*/
void watchwire(void)
{
	int ret;
	char Flag;
	char temp;

	temp = ReadEEPROM(wireaddr);
	if((SWWIPORT == 0) && (temp != '0'))
	{
		WriteEEPROM('0', wireaddr);
		ret = GetG8();
		if(ret == -1)
		{
			InitWire();
			return;
		}
		Flag = GetG8Flag();
		if((Flag & BIT0) != 0)
		{
			Flag &= ~BIT0;
			SetFlag(Flag);
		}
	}
	if((SWWIPORT == BIT4) && (temp != '1'))
	{
		WriteEEPROM('1', wireaddr);
		ret = GetG8();
		if(ret == -1)
		{
			InitWire();
			return;
		}
		Flag = GetG8Flag();
		if((Flag & BIT0) != 1)
		{
			Flag |= BIT0;
			SetFlag(Flag);
		}
	}
	InitWire();
	return;
}

static int GetG8(void)
{
	InitWire();
	WireFlag = 1;
	SFRPAGE	= UART1_PAGE;
	SBUF1 = 'z';
	
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 40) && (WireBuff[len - 1] != '>') && (WireBuff[len - 1] != 'z'))
		;
	CloseTimer0Interrupt();	
	if(OverflowT0 > 38)
	{

		return -1;
	}
	
	for(len; len >= 0; len--)
	{
		WireBuff[len] = '\0';
	}
	
	Delay(60000);
	len = 0;
	WireFlag = 2;
	SBUF1 = '\r';
	while(WireBuff[len - 1] != '>')
		;
	
	for(len; len >= 0; len--)
	{
		WireBuff[len] = '\0';
	}
	Delay(60000);
	len1 = 1;
	len = 0;
	WireFlag = 3;
	Buff[0] = 'G';
	Buff[1] = '8';
	Buff[2] = '\r';
	Buff[3] = '\n';

	SFRPAGE	= UART1_PAGE;
	SBUF1 = 'G';
	while(WireBuff[len - 1] != '>')
		;
	return 0;
}

static char GetG8Flag(void)
{
	char Flag;
	
	//char to int
	if(WireBuff[14] >= '0' && WireBuff[14] <= '9')
	{
		Flag = WireBuff[14] - '0';
	}
	else
	{
		Flag = WireBuff[14] - 'A' + 10;
	}	
	return Flag;
}

static void SetFlag(char TFlag)
{
	char Flag;
	if(TFlag > 9)
	{
		Flag = TFlag - 10 + 'A';
	}
	else
	{
		Flag = TFlag + '0';
	}
	
	len2 = len;
	len = 0;
	len1 = 1;
	WireBuff[14] = Flag;
	WireFlag	 = 4;
	SFRPAGE	= UART1_PAGE;
	SBUF1		 = 'S';
    while(len1 != len2)
		;
	Delay(60000);
	CloseUart1();
	return;
}

static void InitWire(void)
{
	len = 0;
	len1 = 0;
	len2 = 0;
	WireFlag = 0;
	CloseUart1();
	TimeTest(4);

	POWERCLOSE;
	TimeTest(4);
	POWEROPEN;
	Delay(430);

	OpenUart1();

	return;
}
/*
9>G8
:2000001059A100414B4454455354370000000000000000000000000000000000000000008F
:2000201000000000414B445445535437000000000000000000000000000000000000000069
:20004010000000000000000000000000000000000000000000000000000000000000000090
:1E00601000000000000000000000000000000000000000000000000000000000000072
:00000001FF                                                                                                                                                                                                                                                                                                        
*/

