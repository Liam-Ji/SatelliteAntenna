/***********************************************************************
* 
* ʱ    �䣺2008-10-9
*
* �� �� ����ADC.c
*
* ��    ����AKD10P07(������)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
* 
* ����˵����C8051F020ADC��س�ʼ��,�빦�ܺ���
*
***********************************************************************/
#include "c8051f120.h"
#include "adc.h"
#include "uart.h"
#include "station.h"

extern INT32 tempmin;
extern INT32 tempmax;
extern UINT8 xdata PloarAngleStage;

/*ȫ�ֱ���*/
UINT16 xdata ReceiverKindFlag;	//���ջ�����жϱ�־
float xdata AngleCom = 0;		//��������ֵ
float xdata StationPol;			//����վ�����Ƕ�


/*��̬����*/
static float GetSharpAGC(void);
static float GetXinbiaoAGC(void);


/***********************************************************************
*
* ����ԭ�ͣ�void ADC0Init(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ����������ADC0��ʼ��ͨ���� AD0BUSY д��1����
*
***********************************************************************/
void ADC0Init(void)
{
	EA      =  0;
	SFRPAGE = ADC0_PAGE;
	ADC0CN  =  0x80;			//10000000B,���ú�����£�ADC0 ʹ�ܣ�
	ADC0GTH =  0x0;				//��AD0BUSY д1 ����ADC0 ת����ADC0H:ADC0L �Ĵ��������Ҷ���
	ADC0GTL =  0x0;
	ADC0LTH =  0x0F;
	ADC0LTL =  0xFF;
	ADC0CF  =  0x50;			//����PGA��ת��ʱ����������Ϊ10������22118400 /��10 + 1�� 
	AMX0CF  =  0x0;				//ȫ��Ϊ�����ĵ�������
   	REF0CN  =  0x02;          	//ʹ�õ�ѹ��׼2.5V
	EIE2   &= ~0x02;			//��ֹADC0 ת�������жϡ�
   	EIE1   &= ~0x04;			//��ֹADC0���ڱȽ��ж�
	EA	    =  1;	
}


/***********************************************************************
*
* ����ԭ�ͣ�float GetAGC(void)
*
* ��ڲ�������
*
* ���ڲ�����UINT16�����ݣ����ص�ǰ��ѹ��4��ֵ
*
* �����������ɴӲ�ͬ���ջ��õ�AGC
*
***********************************************************************/
float GetAGC(void)
{	
	float returnvalue;
	if(ReceiverKindFlagS == XINBIAOREC)					//����ǰѡ��Ϊ�ű���ջ�
	{
//		return GetXinbiaoAGC();
//		return GetKCAGC();
		returnvalue=GetKCAGC();
	}
	else
	{
//		return GetSharpAGC();
		returnvalue=GetSharpAGC();
	}
	return returnvalue;	
}


/***********************************************************************
*
* ����ԭ�ͣ�float GetXinbiaoAGC(void)
*
* ��ڲ�������
*
* ���ڲ�����UINT16�����ݣ����ص�ǰ��ѹ��4��ֵ
*
* ������������һ��ͨ��0�ű���ջ����صĲ���ֵ
*
* ��    �ģ�2008-12-9�����õ�AGC�����ֿ�Ϊ�������������޸��ˣ����
*
***********************************************************************/
/*
static float GetXinbiaoAGC(void)
{
	UINT32 	i, tempAGC = 0;
	float AverAgc;

	AMX0SL = CHXINBIAO;	 								//���ű�

	for(i = 0; i < 5; i++)
	{
		AD0INT  = 0;									//adת����ʼ
		AD0BUSY = 1;
		while(!AD0INT)
			;
		tempAGC += ADC0H * 256 + ADC0L;
	}
	tempAGC /= i;
		
	AverAgc = (float)(tempAGC) * 10.0 / 4096.0;			//��������ֵת��Ϊ��ƽֵ
	
	return AverAgc;
}
*/


/***********************************************************************
*
* ����ԭ�ͣ�float GetSharpAGC(void)
*
* ��ڲ�������
*
* ���ڲ�����UINT16�����ݣ����ص�ǰ��ѹ��4��ֵ
*
* ������������һ��ͨ��1���ս��ջ����صĲ���ֵ
*
* ��    �ģ�2008-12-9�����õ�AGC�����ֿ�Ϊ�������������޸��ˣ����
*
***********************************************************************/
static float GetSharpAGC(void)
{
	UINT32 	i, tempAGC = 0;
	float AverAgc;
	
	SFRPAGE = ADC0_PAGE;	
	AMX0SL  = CHSHARP;									//ѡ�����ս��ջ�ͨ��

	for(i = 0; i < 5; i++)
	{
		AD0INT  = 0;									//ADת����ʼ
		AD0BUSY = 1;
		while(!AD0INT)
			;
		tempAGC += ADC0H * 256 + ADC0L;
	}
	tempAGC /= i;
		
	AverAgc = (float)(tempAGC) * 10.0 / 4096.0;			//��������ֵת��Ϊ��ƽֵ
	
	return AverAgc;
}


/***********************************************************************
*
* ����ԭ�ͣ�float   GetPolarAngle(void)
*
* ��ڲ�������
*
* ���ڲ������Ƕ�
*
* �������������ݼ�����ƽ���㼫���Ƕ�.
*
* ע�����ݲ�ͬ�ĵ�����л���,AngleComΪ����ֵ.	
*
***********************************************************************/
float GetPolarAngle(void)
{
	UINT16 	i;
	static tempagc = 0;			   //�洢���ε�ȡ��ֵ
	static tempagcB = 0;		   //�洢��һ�ε�ȡ��ֵ
	float Angle;
	float AngleTemp;

	tempagcB = tempagc;
	SFRPAGE = ADC0_PAGE;		
	AMX0SL  = CHPOLAR;
	tempagcB = tempagc;
	tempagc = 0;
	for(i = 0; i < 5; i++)
	{
		AD0INT = 0;				   //ADC0ת�������жϱ�־����һ��ת���ɹ��Զ���Ϊ1��Ҫ�����������һλ
		AD0BUSY = 1;			   //����ADת��
		while(!AD0INT)			   //�ȴ�ת������
			;
		tempagc += ADC0H * 256 + ADC0L;
	}
	tempagc /= i;				   //�ɼ�5����ƽ��ֵ���˲�

//	if(tempagc > tempmax)
//		tempmax = tempagc;
//	if(tempagc < tempmin)
//		tempmin = tempagc;
	if(tempagc - tempagcB < -2000)					   //�ж�ȡ��ֵ�Ƿ��������䵽��С������һ��̨��
		PloarAngleStage = PloarAngleStage + 1;
	if(tempagc - tempagcB > 2000)					   //�ж�ȡ��ֵ�Ƿ����С���䵽����½�һ��̨��
		PloarAngleStage = PloarAngleStage - 1;
//	Angle = -0.10405 * ((float)(tempagc) - 1295.0) + AngleCom;   //ͨ����25KG
//	Angle = 0.026857654431512981199641897940913 * ((float)(tempagc) - 2093.0) + AngleCom;   //ͨ����25KG
	AngleTemp = 0.1091239769627159745377 * ((float)(tempagc) - 426.0);   //ͨ����25KG
	AngleTemp /= 3;
	Angle = PloarAngleStage * 120 +	AngleTemp - 12;
	return Angle;	
}


/***********************************************************************
*
* ����ԭ�ͣ�float GetT(void)
*
* ��ڲ�������
*
* ���ڲ�����float�����ݣ����ص�ǰ�¶�
*
* �������������ص�ǰ�¶ȣ������Ʒ���
*
***********************************************************************/
float GetT(void)
{
	UINT32 	i, tempAGC = 0;
	float AverAgc;

	SFRPAGE = ADC0_PAGE;
	AMX0SL  = 0xf;	 									//���ű�
	REF0CN = 7;

	for(i = 0; i < 5; i++)
	{
		AD0INT  = 0;									//adת����ʼ
		AD0BUSY = 1;
		while(!AD0INT)
			;
		tempAGC += ADC0H * 256 + ADC0L;
	}
	tempAGC /= i;
	REF0CN = 2;
		
	AverAgc = ((float)tempAGC * 2.5 / 4096.0 - 0.776) / 0.00286;//��������ֵת��Ϊ��ƽֵ
	
	return AverAgc;
}


/***********************************************************************
*
* ����ԭ�ͣ�float   GetComPolA(void)
*
* ��ڲ�������
*
* ���ڲ������Ƕ�
*
* �����������õ�ͨ�õļ����Ƕ�.	
*
***********************************************************************/
float GetComPolA(void)
{
	float angle;

	angle = GetPolarAngle();

	return angle;
}
