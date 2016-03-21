/***********************************************************************
* 
* ʱ    �䣺2008-10-9
*
* �� �� ����main.c
*
* ��    ����AKD10P07(������)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
*
* ע    �⣺�ϸ��汾�����Ϊ�Ͼ��ʵ��ѧ�з����ر��л��
* 
* ����˵�������߶���������
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
	CPU020Init();   						//MCU��ʼ��
	ADC0Init();								//ADC��ʼ��
	Timer0Init();							//timer0��ʼ��
	Timer2Init();							//timer2��ʼ��
	Timer3Init();							//timer3��ʼ��
	Uart0Init();							//����0��ʼ��
	Uart1Init();							//����1��ʼ��


//	plorF = 1;									//����ת����־��plorF==1ת����plorF==0ֹͣ
	
	status = INIT;						     	//���߿�ʼ״̬Ϊ���߳�ʼ��

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
			AntennaInit();					//���߳�ʼ������
		}
		if(status == SEARCHREADY)
		{
			AntennaReadySearch();			//����׼����������
		}
		if(status == SEARCHING)
		{
			AntennaTimeSearch();			//������������
		}
		if(status == TRACKING)
		{
			AntennaTracking();				//���߸��ٺ���
		}
		if(status == MANUALMODE)
		{
			AntennaManual();				//�����ֶ�����
		}
		if(status == STORESTATUS)
		{
			AntennaStore();					//�����ղغ���
		}
	
	}
	return 0;
}
