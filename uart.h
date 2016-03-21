/***********************************************************************
* 
* ʱ�䣺2008-10-13
*
* �ļ�����uart.h
*
* ���ߣ� �������Ƶ���ϢͨѶ�������޹�˾�������������
*
***********************************************************************/
#ifndef __UART0_H__
#define __UART0_H__


#include "public.h"					    


#define UART0INTNUMBER		4			//����0���жϺ�
#define UART1INTNUMBER		20			//����1���жϺ�
#define MAXLENGTH			100			//�������ڽ��շ���������󳤶�


#define SELECT0L (P6 &= ~BIT5)			//
#define SELECT0H (P6 |=  BIT5)			//
#define SELECT1L (P6 &= ~BIT6)			//
#define SELECT1H (P6 |=  BIT6)			//Ϊ���л�����0��GPS�����̣���б�ǣ�����ͨ��


#define SYSCLK 				22118400L	//ϵͳ����Ƶ��
#define UARTBR9600 			9600L		//��ʼ�����ڲ�����		 
#define UARTBR4800			4800L		//��ʼ�����ڲ�����
#define UARTBR19200			19200L		//��ʼ�����ڲ�����


extern char  xdata Uart0R[];
extern float xdata GradientY;			//��б���������б��Y
extern float xdata GradientRightR;		//��б���������б��У������
extern int   xdata GradientNormal;		//��б�ǲ�����
extern int   xdata GPSNormal;			//GPS������
extern float xdata GPSLong;				//GPS����ľ���
extern BOOL  xdata GPSEastFlag;			//GPS������־
extern float xdata GPSLat;				//GPS�����γ��
extern BOOL  xdata GPSNorthFlag;		//GPS��γ��־


extern char  xdata Uart1ReceData[];		//����1��������
extern char  xdata Uart1SendData[];		//����1��������
extern UINT8 xdata Uart1ReceDataLength;	//��ǰ����1Ҫ���յ����ݻ��峤��
extern UINT8 xdata Uart1SendDataLength;	//��ǰ����1Ҫ���͵���������
extern UINT8 xdata Uart1TXMAXLenth;		//����1Ҫ���͵���󳤶�
extern UINT8 xdata NumberOfComand;		//�������


/*uart0 function*/
extern void Uart0Init(void);
extern void ReadGradient(void);
extern void ReadGPS(void);
extern void SetXinBiaoFreqKC(double freq);//2009-2-17
//extern void  SelectFreq(float freq);
extern float GetKCAGC(void);
extern void GetXinBiaoVER(void);
extern void GetXinBiaoVorX(void);


/*uart1 function*/
extern void Uart1Init(void);
extern void Uart1TX(void);


/* uart1 and watchwire */
extern void watchwire(void);


#endif