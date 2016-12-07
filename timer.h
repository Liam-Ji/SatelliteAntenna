/***********************************************************************
* 
* 时间：2008-10-10
*
* 文件名：timer.h
*
* 作者： 北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*											
***********************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__


#include "public.h"
#include "c8051f120.h"


#define MotorLimitFlag 0		//电机限位到标志
sbit	AzMidLimit	 = P3^5;	//中间限位标志
sbit	ELMidLimit	 = P3^4;	//俯仰脉冲(15KG天线俯仰中间限位)	
//sbit	AZRightLimit = P3^7;	//右限位标志
//sbit	AZLeftLimit	 = P3^6;	//左限位标志
sbit	AZRightLimit = P3^6;	//右限位标志
sbit	AZLeftLimit	 = P3^7;	//左限位标志				 //20160117交换左右限位和左右方向


sbit 	ChangeKit	 = P3^0;	//切换开关2008-12-24
sbit	MAkit		 = P3^1;	//手动自动切换开关2009/10/3
#define SpeedInitF 		0xFE50  //天线开机时俯仰用的速度

#define SpeedInitS 		0xFE20	//天线开机时方位用的速度（不正常开机会有）
#define SpeedSearchEl	0xF000	//天线搜索时俯仰用的速度
//#define SpeedSearchEl	0xE000	//天线搜索时俯仰用的速度
#define SElspeed 		0xB00
#define SpeedSearchAzF	0xFE50	//天线搜索时方位用的速度，（从慢到快的中的快速）
#define SpeedSearchAzS	0xFA00	//天线搜索时方位用的速度，（从慢到快的中的快速)
#define SpeedTRACKING	0xFA00	//天线跟踪速度

#define POLAR_LEFT	P2 &= ~BIT5; P2 |=  BIT4	//极化逆时针旋转
#define POLAR_STOP	P2 &= ~BIT5; P2 &= ~BIT4	//极化停止转动
#define POLAR_RIGHT	P2 |=  BIT5; P2 &= ~BIT4	//极化顺时针旋转


#define OFT0NUMBER		1000	//0.1s
#define T0INTNUMBER 	1		//timer0中断号
#define T2INTNUMBER 	5		//timer0中断号
#define T3INTNUMBER		14		//timer3中断号


#define ELUP 			0		//俯仰电机向上
#define ELDOWN 			1		//俯仰电机向下
//#define AZRIGHT 		0		//方位电机向右
//#define AZLEFT			1		//方位电机向左

#define AZRIGHT 		1		//方位电机向右
#define AZLEFT			0		//方位电机向左

#define MOTORSTOP		2		//停电机标志


#define UPDOWN			0		//上下方向(2008-11-3)
#define RIGHTLEFT		1		//左右方向


extern INT16 data OverflowT0;
extern float data StationAz;		//地球站方位角度 
extern float data StationEl;		//地球站俯仰角度
extern float data PolarDst;			//极化目地角度
extern INT16 data PolarRightFlag;	//极化到位标志


extern float data StationAzLimit;
extern UINT8 data AzMidLimitF;				//2010-7-7
extern UINT8 data AZLeftLimitF;				//
extern UINT8 data AZRightLimitF;			//限位故障标致


sbit ELDir	= P2^0;				//俯仰方向
sbit ELPlus = P2^1;				//俯仰脉冲
sbit AZDir	= P2^2;				//方位方向
sbit AZPlus	= P2^3;				//方位脉冲										   	
sbit Tempearter	 = P2^7;										   	

/*定时器相关函数*/
extern void Timer0Init(void);
extern void OpenTimer0Interrupt(void);
extern void CloseTimer0Interrupt(void);
extern void Timer2Init(void);
extern void OpenTimer2Interrupt(void);
extern void CloseTimer2Interrupt(void);
extern void Timer3Init(void);
extern void OpenTimer3Interrupt(UINT16);

/*电机相关函数*/
extern void MotorFun(INT16 AzMode, INT16 ElMode, UINT16 speed);
extern void MotorCtrl(INT16 DirFlag, float Angle, UINT16 speed);
extern void GotoAzMid(void);
extern UINT16 GetElS1(void);
extern UINT16 GetAzS2(void);
extern UINT16 GetAzS1(void);
extern void   TestStor(char flag);
extern void AzLimitTest(void);
extern void GoAM(void);

#endif
