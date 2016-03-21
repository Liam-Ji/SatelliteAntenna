/***********************************************************************
* 
* 时    间：2008-10-10
*
* 文 件 名：adc.h
*
* 作    者： 北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*
***********************************************************************/
#ifndef __ADC_H__
#define __ADC_H__


#include "public.h"


#define CHXINBIAO		0				//信标AGC通道号
#define CHSHARP			1				//夏普AGC通道号
#define CHPOLAR			2				//极化通道号
#define CHBAK			3				//备用通道号新信标用


#define XINBIAOREC		0				//当前采用信标接收机跟踪
#define SHARPREC		1				//当前采用DVB接收机跟踪


extern UINT16 xdata ReceiverKindFlag;	//接收机类别判断标志
extern float xdata AngleCom;			//极化角补偿值
extern float xdata StationPol;			//地球站极化角度


extern void ADC0Init(void);
extern float GetAGC(void);
extern float GetPolarAngle(void);
extern float GetComPolA(void);
extern float GetT(void);


#endif
