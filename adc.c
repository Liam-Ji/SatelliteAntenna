/***********************************************************************
* 
* 时    间：2008-10-9
*
* 文 件 名：ADC.c
*
* 版    本：AKD10P07(发布版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：C8051F020ADC相关初始化,与功能函数
*
***********************************************************************/
#include "c8051f120.h"
#include "adc.h"
#include "uart.h"
#include "station.h"



/*全局变量*/
UINT16 xdata ReceiverKindFlag;	//接收机类别判断标志
float xdata AngleCom = 0;		//极化补偿值
float xdata StationPol;			//地球站极化角度


/*静态函数*/
static float GetSharpAGC(void);
static float GetXinbiaoAGC(void);


/***********************************************************************
*
* 函数原型：void ADC0Init(void)
*
* 入口参数：无
*
* 出口参数：无
*
* 功能描述：ADC0初始化通过向 AD0BUSY 写‘1’启
*
***********************************************************************/
void ADC0Init(void)
{
	EA      =  0;
	SFRPAGE = ADC0_PAGE;
	ADC0CN  =  0x80;			//10000000B,配置含义见下，ADC0 使能，
	ADC0GTH =  0x0;				//向AD0BUSY 写1 启动ADC0 转换，ADC0H:ADC0L 寄存器数据右对齐
	ADC0GTL =  0x0;
	ADC0LTH =  0x0F;
	ADC0LTL =  0xFF;
	ADC0CF  =  0x50;			//配置PGA，转换时钟周期配置为10，即：22118400 /（10 + 1） 
	AMX0CF  =  0x0;				//全部为独立的单端输入
   	REF0CN  =  0x02;          	//使用电压基准2.5V
	EIE2   &= ~0x02;			//禁止ADC0 转换结束中断。
   	EIE1   &= ~0x04;			//禁止ADC0窗口比较中断
	EA	    =  1;	
}


/***********************************************************************
*
* 函数原型：float GetAGC(void)
*
* 入口参数：无
*
* 出口参数：UINT16型数据，返回当前电压的4倍值
*
* 功能描述：可从不同接收机得到AGC
*
***********************************************************************/
float GetAGC(void)
{	
	float returnvalue;
	if(ReceiverKindFlagS == XINBIAOREC)					//若当前选择为信标接收机
	{
//		return GetXinbiaoAGC();
//		return GetKCAGC();
		returnvalue=GetKCAGC();
	}
	else
	{
//		return GetSharpAGC();
		returnvalue=GetSharpAGC();
	}
	return returnvalue;	
}


/***********************************************************************
*
* 函数原型：float GetXinbiaoAGC(void)
*
* 入口参数：无
*
* 出口参数：UINT16型数据，返回当前电压的4倍值
*
* 功能描述：采一次通道0信标接收机返回的采样值
*
* 修    改：2008-12-9，将得到AGC函数分开为两个主函数，修改人：杨淳雯
*
***********************************************************************/
/*
static float GetXinbiaoAGC(void)
{
	UINT32 	i, tempAGC = 0;
	float AverAgc;

	AMX0SL = CHXINBIAO;	 								//旧信标

	for(i = 0; i < 5; i++)
	{
		AD0INT  = 0;									//ad转换开始
		AD0BUSY = 1;
		while(!AD0INT)
			;
		tempAGC += ADC0H * 256 + ADC0L;
	}
	tempAGC /= i;
		
	AverAgc = (float)(tempAGC) * 10.0 / 4096.0;			//将采样的值转换为电平值
	
	return AverAgc;
}
*/


/***********************************************************************
*
* 函数原型：float GetSharpAGC(void)
*
* 入口参数：无
*
* 出口参数：UINT16型数据，返回当前电压的4倍值
*
* 功能描述：采一次通道1夏普接收机返回的采样值
*
* 修    改：2008-12-9，将得到AGC函数分开为两个主函数，修改人：杨淳雯
*
***********************************************************************/
static float GetSharpAGC(void)
{
	UINT32 	i, tempAGC = 0;
	float AverAgc;
	
	SFRPAGE = ADC0_PAGE;	
	AMX0SL  = CHSHARP;									//选择夏普接收机通道

	for(i = 0; i < 5; i++)
	{
		AD0INT  = 0;									//AD转换开始
		AD0BUSY = 1;
		while(!AD0INT)
			;
		tempAGC += ADC0H * 256 + ADC0L;
	}
	tempAGC /= i;
		
	AverAgc = (float)(tempAGC) * 10.0 / 4096.0;			//将采样的值转换为电平值
	
	return AverAgc;
}


/***********************************************************************
*
* 函数原型：float   GetPolarAngle(void)
*
* 入口参数：无
*
* 出口参数：角度
*
* 功能描述：根据极化电平计算极化角度.
*
* 注：根据不同的电机进行换算,AngleCom为补尝值.	
*
***********************************************************************/
float GetPolarAngle(void)
{
	UINT16 	i;
	static tempagc = 0;			   //存储本次的取样值
	static tempagcA = 0;
//	static tempagcB = 0;		   //存储上一次的取样值
//	static tempagcC = 0;
//	static tempagcD = 0;
//	static tempagcE = 0;
	float Angle;
	float AngleTemp;

	tempagcA = tempagc;
//	tempagcB = tempagcC;
//	tempagcC = tempagcD;
//	tempagcD = tempagcE;
//	tempagcE = tempagc;
	SFRPAGE = ADC0_PAGE;		
	AMX0SL  = CHPOLAR;

	tempagc = 0;
	for(i = 0; i < 5; i++)
	{
		AD0INT = 0;				   //ADC0转换结束中断标志，上一次转换成功自动变为1，要用软件清零这一位
		AD0BUSY = 1;			   //启动AD转换
		while(!AD0INT)			   //等待转换结束
			;
		tempagc += ADC0H * 256 + ADC0L;
	}
	tempagc /= i;				   //采集5次求平均值，滤波

	if(tempagc > tempmax)
		tempmax = tempagc;
	if(tempagc < tempmin)
		tempmin = tempagc;


//	if(tempagc > tempmax - 100)
//		maxnumber = maxnumber + 1;
//	if(tempagc < tempmin + 100)
//		minnumber = minnumber + 1;
//	if(maxnumber > 10)
//		maxflag = 1;
//	if(minnumber > 10)
//		minflag = 1;
//	if(maxflag == 1) {
//		if(minflag == 1) {
//			
//		}
//	}

	if(tempagc - tempagcA < -2200)					   //判断取样值是否从最大跳变到最小，上升一个台阶
		PloarAngleStage = PloarAngleStage + 1;
	if(tempagc - tempagcA > 2200)					   //判断取样值是否从最小跳变到最大，下降一个台阶
		PloarAngleStage = PloarAngleStage - 1;
//	Angle = -0.10405 * ((float)(tempagc) - 1295.0) + AngleCom;   //通用型25KG
//	Angle = 0.026857654431512981199641897940913 * ((float)(tempagc) - 2093.0) + AngleCom;   //通用型25KG
	AngleTemp = 120.0/(float)(3737-427) * ((float)(tempagc) - 427);   //通用型25KG
	if(PloarAngleStage < 0 || PloarAngleStage > 4)
		PloarAngleStage = 0;
//	if(PloarAngleStage == 0)
//		Angle = AngleTemp;		 //第一个阶梯使用原值
//	else		
		Angle = PloarAngleStage * 120 +	AngleTemp - 55;	   //后两个阶梯要加上每个阶梯的度数120，减去初始化角度，因为第一个阶梯没有整个走过
	return Angle;	
}


/***********************************************************************
*
* 函数原型：float GetT(void)
*
* 入口参数：无
*
* 出口参数：float型数据，返回当前温度
*
* 功能描述：返回当前温度，来控制风扇
*
***********************************************************************/
float GetT(void)
{
	UINT32 	i, tempAGC = 0;
	float AverAgc;

	SFRPAGE = ADC0_PAGE;
	AMX0SL  = 0xf;	 									//旧信标
	REF0CN = 7;

	for(i = 0; i < 5; i++)
	{
		AD0INT  = 0;									//ad转换开始
		AD0BUSY = 1;
		while(!AD0INT)
			;
		tempAGC += ADC0H * 256 + ADC0L;
	}
	tempAGC /= i;
	REF0CN = 2;
		
	AverAgc = ((float)tempAGC * 2.5 / 4096.0 - 0.776) / 0.00286;//将采样的值转换为电平值
	
	return AverAgc;
}


/***********************************************************************
*
* 函数原型：float   GetComPolA(void)
*
* 入口参数：无
*
* 出口参数：角度
*
* 功能描述：得到通用的极化角度.	
*
***********************************************************************/
float GetComPolA(void)
{
	float angle;

	angle = GetPolarAngle();

	return angle;
}
