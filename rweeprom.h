/***********************************************************************
* 
* 时间：2008-10-15
*
* 文件名：rweeprom.h
*
* 作者： 北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*
***********************************************************************/
#ifndef __RWEEPROM_H__
#define __RWEEPROM_H__


#include "public.h"


//*--------------------------2009-3-4以后的-------------------------------
#define CS_93C66_0 	(P5 &=~BIT4)
#define CS_93C66_1 	(P5 |= BIT4)		//93c66片选信号

#define SK_93C66_0 	(P5 &=~BIT3)
#define SK_93C66_1 	(P5 |= BIT3)		//93c66时钟信号

#define DI_93C66_0 	(P5 &=~BIT2)
#define DI_93C66_1 	(P5 |= BIT2)		//93c66串行输入数据信号

#define D0_93C66_0	(P5 &=~BIT1)
#define D0_93C66_1	(P5 |= BIT1)
#define DO_93C66 	(P5 & BIT1)			//93c66串行输出数据信号
//--------------------------2009-3-4以后的-------------------------------*/


#define TIME 		40					//延迟时间，测出能读写的最快速度

//总线协议：SPI总线
/**************93C66存储分配说明******************************
					存储大小：256字节
***************93C66存储分配说明*****************************/
#define AbnormalFlagAddress			0x00

#define StationLongAddress 			0x01//0x02
#define StationLatAddress 			0x03//0x04
#define SatLongAddress				0x05//0x06
#define StationNorthFlagAddress		0x07
#define StationEastFlagAddress		0x08
#define StationPloModeAddress		0x09

#define SatSharpFreqAddress 		0x0A//0x0B,0x0C,0x0D
#define SatXinBiaoFreqAddress 		0x0E//0x0F,0x10,0x11
#define SharpAgcThresholdAddress 	0x12
#define XBThresholdAddress			0x13

#define AZCompensateAddress			0x14//0x15//第一个是正负号，第二个为10倍值
#define ELCompensateAddress			0x16//0x17//第一个是正负号，第二个为10倍值
#define PolCompensateAddress		0x18//0x19//第一个是正负号，第二个为10倍值

#define StoreElDownAddress			0x1A//存储收藏的坡面角度
#define StoreELSignAddress			0x1B//坡面角度符号

#define wireaddr					0x1C	//
		
#define rightONE					0x1D	
#define rightTWO					0x1E
#define rightTHREE					0x1F	//
#define rightFOUR					0x20	//
#define rightFIVE					0x21
#define rightSIX					0x22//2008-11-14修改

#define ReceiverKindAddress			0x23//接收机类别

/*2009-6-9新加,用于保存基准卫星数据*/
#define BaseStarLONGAddr			0x24//0x25保存基准卫星经度
#define BaseStarPolAddr				0x26//保存极化方式
#define BaseStarFreqAddr			0x27//0x28//0x29//0x2A保存频率
#define BaseStarKbSAddr				0x2B//0x2C//0x2D保存符号率
#define BaseStarRTypeAddr			0x2E//保存接收机类型
#define SSrcStarKbSAddr				0x30//0x31//0x32保存目标卫星符号率
#define BaseStarLONGEFAddr			0x33//保存参考卫星经度标致
#define SatLongEastFlagAddr			0x34//保存目标卫星经度


#define BStarNumAddr				0xB0
#define BStarAddr					0xB1//OxB2

#define BStarPolAddr				2	//OxB3	   
#define BStarFreqAddr				3	//0xB4/OxB5/0xB6/OxB7
#define BStarKbSAddr				7	//OxB8/OxB9/OxBA
#define BStarRTypeAddr				10	//OxBB
#define BStarEWFlagAddr				11	//OxBC

#define BStarLen					12

#define AzMidLimitFAddr				0xC0	   //2010-9-28加入
#define AZLeftLimitFAddr			0xC1
#define AZRightLimitFAddr			0xC2


extern UINT8 ReadEEPROM(char Address);				//读取93C66一个字节
extern void WriteEEPROM(char Data,char Address);	//写一个字节


#endif
