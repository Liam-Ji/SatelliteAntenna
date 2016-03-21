/***********************************************************************
* 
* ʱ    �䣺2008-10-10
*
* �� �� ����adc.h
*
* ��    �ߣ� �������Ƶ���ϢͨѶ�������޹�˾�������������
*
***********************************************************************/
#ifndef __ADC_H__
#define __ADC_H__


#include "public.h"


#define CHXINBIAO		0				//�ű�AGCͨ����
#define CHSHARP			1				//����AGCͨ����
#define CHPOLAR			2				//����ͨ����
#define CHBAK			3				//����ͨ�������ű���


#define XINBIAOREC		0				//��ǰ�����ű���ջ�����
#define SHARPREC		1				//��ǰ����DVB���ջ�����


extern UINT16 xdata ReceiverKindFlag;	//���ջ�����жϱ�־
extern float xdata AngleCom;			//�����ǲ���ֵ
extern float xdata StationPol;			//����վ�����Ƕ�


extern void ADC0Init(void);
extern float GetAGC(void);
extern float GetPolarAngle(void);
extern float GetComPolA(void);
extern float GetT(void);


#endif
