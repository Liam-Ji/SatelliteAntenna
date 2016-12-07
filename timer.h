/***********************************************************************
* 
* ʱ�䣺2008-10-10
*
* �ļ�����timer.h
*
* ���ߣ� �������Ƶ���ϢͨѶ�������޹�˾�������������
*											
***********************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__


#include "public.h"
#include "c8051f120.h"


#define MotorLimitFlag 0		//�����λ����־
sbit	AzMidLimit	 = P3^5;	//�м���λ��־
sbit	ELMidLimit	 = P3^4;	//��������(15KG���߸����м���λ)	
//sbit	AZRightLimit = P3^7;	//����λ��־
//sbit	AZLeftLimit	 = P3^6;	//����λ��־
sbit	AZRightLimit = P3^6;	//����λ��־
sbit	AZLeftLimit	 = P3^7;	//����λ��־				 //20160117����������λ�����ҷ���


sbit 	ChangeKit	 = P3^0;	//�л�����2008-12-24
sbit	MAkit		 = P3^1;	//�ֶ��Զ��л�����2009/10/3
#define SpeedInitF 		0xFE50  //���߿���ʱ�����õ��ٶ�

#define SpeedInitS 		0xFE20	//���߿���ʱ��λ�õ��ٶȣ��������������У�
#define SpeedSearchEl	0xF000	//��������ʱ�����õ��ٶ�
//#define SpeedSearchEl	0xE000	//��������ʱ�����õ��ٶ�
#define SElspeed 		0xB00
#define SpeedSearchAzF	0xFE50	//��������ʱ��λ�õ��ٶȣ�������������еĿ��٣�
#define SpeedSearchAzS	0xFA00	//��������ʱ��λ�õ��ٶȣ�������������еĿ���)
#define SpeedTRACKING	0xFA00	//���߸����ٶ�

#define POLAR_LEFT	P2 &= ~BIT5; P2 |=  BIT4	//������ʱ����ת
#define POLAR_STOP	P2 &= ~BIT5; P2 &= ~BIT4	//����ֹͣת��
#define POLAR_RIGHT	P2 |=  BIT5; P2 &= ~BIT4	//����˳ʱ����ת


#define OFT0NUMBER		1000	//0.1s
#define T0INTNUMBER 	1		//timer0�жϺ�
#define T2INTNUMBER 	5		//timer0�жϺ�
#define T3INTNUMBER		14		//timer3�жϺ�


#define ELUP 			0		//�����������
#define ELDOWN 			1		//�����������
//#define AZRIGHT 		0		//��λ�������
//#define AZLEFT			1		//��λ�������

#define AZRIGHT 		1		//��λ�������
#define AZLEFT			0		//��λ�������

#define MOTORSTOP		2		//ͣ�����־


#define UPDOWN			0		//���·���(2008-11-3)
#define RIGHTLEFT		1		//���ҷ���


extern INT16 data OverflowT0;
extern float data StationAz;		//����վ��λ�Ƕ� 
extern float data StationEl;		//����վ�����Ƕ�
extern float data PolarDst;			//����Ŀ�ؽǶ�
extern INT16 data PolarRightFlag;	//������λ��־


extern float data StationAzLimit;
extern UINT8 data AzMidLimitF;				//2010-7-7
extern UINT8 data AZLeftLimitF;				//
extern UINT8 data AZRightLimitF;			//��λ���ϱ���


sbit ELDir	= P2^0;				//��������
sbit ELPlus = P2^1;				//��������
sbit AZDir	= P2^2;				//��λ����
sbit AZPlus	= P2^3;				//��λ����										   	
sbit Tempearter	 = P2^7;										   	

/*��ʱ����غ���*/
extern void Timer0Init(void);
extern void OpenTimer0Interrupt(void);
extern void CloseTimer0Interrupt(void);
extern void Timer2Init(void);
extern void OpenTimer2Interrupt(void);
extern void CloseTimer2Interrupt(void);
extern void Timer3Init(void);
extern void OpenTimer3Interrupt(UINT16);

/*�����غ���*/
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
