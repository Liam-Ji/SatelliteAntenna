/***********************************************************************
* 
* ʱ�䣺2008-10-15
*
* �ļ�����rweeprom.h
*
* ���ߣ� �������Ƶ���ϢͨѶ�������޹�˾�������������
*
***********************************************************************/
#ifndef __RWEEPROM_H__
#define __RWEEPROM_H__


#include "public.h"


//*--------------------------2009-3-4�Ժ��-------------------------------
#define CS_93C66_0 	(P5 &=~BIT4)
#define CS_93C66_1 	(P5 |= BIT4)		//93c66Ƭѡ�ź�

#define SK_93C66_0 	(P5 &=~BIT3)
#define SK_93C66_1 	(P5 |= BIT3)		//93c66ʱ���ź�

#define DI_93C66_0 	(P5 &=~BIT2)
#define DI_93C66_1 	(P5 |= BIT2)		//93c66�������������ź�

#define D0_93C66_0	(P5 &=~BIT1)
#define D0_93C66_1	(P5 |= BIT1)
#define DO_93C66 	(P5 & BIT1)			//93c66������������ź�
//--------------------------2009-3-4�Ժ��-------------------------------*/


#define TIME 		40					//�ӳ�ʱ�䣬����ܶ�д������ٶ�

//����Э�飺SPI����
/**************93C66�洢����˵��******************************
					�洢��С��256�ֽ�
***************93C66�洢����˵��*****************************/
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

#define AZCompensateAddress			0x14//0x15//��һ���������ţ��ڶ���Ϊ10��ֵ
#define ELCompensateAddress			0x16//0x17//��һ���������ţ��ڶ���Ϊ10��ֵ
#define PolCompensateAddress		0x18//0x19//��һ���������ţ��ڶ���Ϊ10��ֵ

#define StoreElDownAddress			0x1A//�洢�ղص�����Ƕ�
#define StoreELSignAddress			0x1B//����Ƕȷ���

#define wireaddr					0x1C	//
		
#define rightONE					0x1D	
#define rightTWO					0x1E
#define rightTHREE					0x1F	//
#define rightFOUR					0x20	//
#define rightFIVE					0x21
#define rightSIX					0x22//2008-11-14�޸�

#define ReceiverKindAddress			0x23//���ջ����

/*2009-6-9�¼�,���ڱ����׼��������*/
#define BaseStarLONGAddr			0x24//0x25�����׼���Ǿ���
#define BaseStarPolAddr				0x26//���漫����ʽ
#define BaseStarFreqAddr			0x27//0x28//0x29//0x2A����Ƶ��
#define BaseStarKbSAddr				0x2B//0x2C//0x2D���������
#define BaseStarRTypeAddr			0x2E//������ջ�����
#define SSrcStarKbSAddr				0x30//0x31//0x32����Ŀ�����Ƿ�����
#define BaseStarLONGEFAddr			0x33//����ο����Ǿ��ȱ���
#define SatLongEastFlagAddr			0x34//����Ŀ�����Ǿ���


#define BStarNumAddr				0xB0
#define BStarAddr					0xB1//OxB2

#define BStarPolAddr				2	//OxB3	   
#define BStarFreqAddr				3	//0xB4/OxB5/0xB6/OxB7
#define BStarKbSAddr				7	//OxB8/OxB9/OxBA
#define BStarRTypeAddr				10	//OxBB
#define BStarEWFlagAddr				11	//OxBC

#define BStarLen					12

#define AzMidLimitFAddr				0xC0	   //2010-9-28����
#define AZLeftLimitFAddr			0xC1
#define AZRightLimitFAddr			0xC2


extern UINT8 ReadEEPROM(char Address);				//��ȡ93C66һ���ֽ�
extern void WriteEEPROM(char Data,char Address);	//дһ���ֽ�


#endif
