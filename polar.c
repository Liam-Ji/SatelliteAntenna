/***********************************************************************
* 
* ʱ    �䣺2008-10-9
*
* �� �� ����polar.c
*
* ��    ����AKD10P07(������)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
* 
* ����˵��������ת������
*
***********************************************************************/
#include "c8051f120.h"
#include "station.h"
#include "adc.h"
#include "timer.h"
#include "antenna.h"
#include "polar.h"
#include "rweeprom.h"


UINT16 xdata PolarFlag = 0;		//�����Ƿ��ת����־
UINT16 xdata PloarNormal = 0;	//����������־
UINT8 xdata PloarAngleStage = 0;	 //����������̨��

/***********************************************************************
*
* ����ԭ�ͣ�void GotoPolarAngle(float angle)
*
* ��ڲ�����Ŀ��Ƕ�
*
* ���ڲ�������
*
* ����������ת��Ŀ�꼫���Ƕ�
*
* ע    �⣺PolarDstȫ�ֱ���!!!
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
		PolarRightFlag = 0;				//������δ��λ
//*----------------------------------------------------------------
//		if(angle < -90.0)	   				//����
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
