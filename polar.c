/***********************************************************************
* 
* 时    间：2008-10-9
*
* 文 件 名：polar.c
*
* 版    本：AKD10P07(发布版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：极化转动函数
*
***********************************************************************/
#include "c8051f120.h"
#include "station.h"
#include "adc.h"
#include "timer.h"
#include "antenna.h"
#include "polar.h"
#include "rweeprom.h"


UINT16 xdata PolarFlag = 0;		//极化是否可转动标志
UINT16 xdata PloarNormal = 0;	//极化正常标志
UINT8 xdata PloarAngleStage = 0;	 //极化角所处台阶

/***********************************************************************
*
* 函数原型：void GotoPolarAngle(float angle)
*
* 入口参数：目标角度
*
* 出口参数：无
*
* 功能描述：转到目标极化角度
*
* 注    意：PolarDst全局变量!!!
*
***********************************************************************/
void GotoPolarAngle(float angle)
{
	if((PolarFlag == 0) && \
		((ReadEEPROM(PolCompensateAddress) == '+') || \
		(ReadEEPROM(PolCompensateAddress) == '-')))
	{
		PolarFlag = 2;
		if(ReadEEPROM(PolCompensateAddress) == '+')
		{
			AngleCom = ReadEEPROM(PolCompensateAddress + 1);
		}
		if(ReadEEPROM(PolCompensateAddress) == '-')
		{
			AngleCom = 0.0 - ReadEEPROM(PolCompensateAddress + 1);
		}
	}

	PolarFlag = 2; 									 //2012

	if(PolarFlag == 2)
	{
		PolarRightFlag = 0;				//极化角未到位
//*----------------------------------------------------------------
//		if(angle < -90.0)	   				//保护
//		{
//			angle = -90.0;
//		}
//		if(angle > 90.0)
//		{
//			angle = 90.0;
//		}
//---------------------------------------------------------------*/
		PolarDst = angle;
//---------------------------------------------------------------*/

		OpenTimer0Interrupt();
		OpenTimer2Interrupt();
		while(PolarRightFlag == 0 && OverflowT0 < 5000)
		{
			;
		}
		POLAR_STOP;
		CloseTimer2Interrupt();
		CloseTimer0Interrupt();
		if(OverflowT0 > 4999)
		{
			PloarNormal = 0;
		}
		else
		{
			PloarNormal = 1;
		}
	}
}
