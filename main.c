/***********************************************************************
* 
* 时    间：2008-10-9
*
* 文 件 名：main.c
*
* 版    本：AKD10P07(发布版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*
* 注    意：上个版本的软件为南京邮电大学研发，特别感谢！
* 
* 功能说明：天线对星主流程
*
***********************************************************************/
#include "adc.h"
#include "cpu.h"
#include "timer.h"
#include "uart.h"
#include "station.h"
#include "antenna.h"
//#include "XZ_8802.h"
#include "polar.h"
#include "public.h"

//INT32 tempmin;
//INT32 tempmax;

int main(void)
{
//	UINT16 plorF;
	CPU020Init();   						//MCU初始化
	ADC0Init();								//ADC初始化
	Timer0Init();							//timer0初始化
	Timer2Init();							//timer2初始化
	Timer3Init();							//timer3初始化
	Uart0Init();							//串口0初始化
	Uart1Init();							//串口1初始化


//	plorF = 1;									//极化转动标志，plorF==1转动，plorF==0停止
	
	status = INIT;						     	//天线开始状态为天线初始化

//	tempmin = 5000;
//	tempmax = -1;
//	POLAR_LEFT;
	
	while(1)
	{
/*		;
		StationPol = GetComPolA();
//		if(PloarAngleStage == 1)
//		   	plorF++;
		while(!plorF)
		{
			LEDRED;
			Delay(40000);
			LEDCLOSE;
			Delay(40000);
			StationPol = GetComPolA();
		}
		while(!ELMidLimit)
		{
			plorF = 0;
			POLAR_STOP;
			break;
		}
*/

		if(status == INIT)
		{
			AntennaInit();					//天线初始化函数
		}
		if(status == SEARCHREADY)
		{
			AntennaReadySearch();			//天线准备搜索函数
		}
		if(status == SEARCHING)
		{
			AntennaTimeSearch();			//天线搜索函数
		}
		if(status == TRACKING)
		{
			AntennaTracking();				//天线跟踪函数
		}
		if(status == MANUALMODE)
		{
			AntennaManual();				//天线手动函数
		}
		if(status == STORESTATUS)
		{
			AntennaStore();					//天线收藏函数
		}
	
	}
	return 0;
}
