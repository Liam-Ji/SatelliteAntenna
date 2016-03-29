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
* ����˵����һЩ��������ص�ȫ�ֱ���
*
***********************************************************************/
#include "station.h"
#include "math.h"
#include "public.h"


float xdata StationAzCal;						//����վ��λ����ֵ
float xdata StationElCal;						//����վ��������ֵ
float xdata StationPolCal;						//����վ��������ֵ
float xdata StationElTemp;						//2009/5/22����


BOOL  xdata StationPloMode;						//����վ������ʽ��0Ϊˮƽ��1Ϊ��ֱ��
float xdata StationLong;						//����վ����
BOOL  xdata StationEastFlag;					//����վ������־
float xdata StationLat;							//����վγ��
BOOL  xdata StationNorthFlag;					//����վ��γ��־
float xdata SatLong;  							//���Ǿ���
float xdata SatLongEastFlag;  					//���Ǿ���


double xdata SharpRecFreq;						//���ս��ջ�Ƶ��,��3 ��Ƶ��
BOOL   xdata SharpRecNormalFlag = 0;			//���ս��ջ�������־
float  xdata SharpThreshold;					//���ս��ջ�����
float  xdata DVBSymbolRate;						//���ս��ջ�������
UINT8  xdata DVBLockFlag;


double xdata XinBiaoRecFreq;					//�ű���ջ�Ƶ��
BOOL   xdata XinBiaoRecNormalFlag = 0;			//�ű���ջ�������־
float  xdata XinBiaoThreshold;					//�ű�����


UINT8  xdata freq_seq[40]={0};					//Ƶ����������


float xdata AGCNoiseNor	= 0.1;					//����������
float xdata AGCNoise;							//����
float xdata AGC;								//AGC����
float xdata Threshold;							//����


UINT8 xdata status;								//���߹���״̬
float xdata AZSearchRange = 30.0;				//��λ������Χ
float xdata ELSearchRange = 5.0;				//����������Χ
float xdata AZlimit		  = 90.0;				//AZ������180�ȵĲ�
float xdata AZ180		  = 180.0;
float xdata ELUPlimit 	  = 75.0;
float xdata ELDOWNlimit	  = ELDOWNlimitDef;



BOOL LockFlag  = 0;		    					//���ٳɹ���־λ
BOOL ResetFlag = 0;								//��λ��־


UINT8  xdata NoMonitorF;						//�޼�ر�־


BOOL BianBanFlag = 0;							//�߰갲װ��ϱ�־
BOOL UninstallBianbanFlag = 0;					//�߰�ж����ϱ�־
BOOL Abnormal;									//�������ϵ��־
BOOL TestFlag = 0;								//�����ղر�־


UINT8  xdata BaseStarLONGEF;					//�ο����Ƕ���������
float  xdata BaseStarLONG;						//��׼���Ǿ���
UINT8  xdata BaseStarPol;						//������ʽ
double xdata BaseStarFreq;						//Ƶ��
float  xdata BaseStarKbS;						//������
UINT8  xdata BaseStarRType;						//���ջ�����
UINT8  xdata EnBaseStar;						//�Ƿ�ʹ�û�׼����


xdata int StoreStatus1 = 0;
//--------------------------------------------------------------
xdata int WireFlag = 0;
xdata char Buff[10] = {'\0'};
xdata int len1 = 0;
xdata char WireBuff[500] = {'\0'};
xdata int len = 0;
xdata int len2 = 0;
//--------------------------------------------------------------



//*********************2009-6-19ʵʱ���ǲ���****************/
BOOL  	xdata StationPloModeS;					//����վ������ʽ��0Ϊˮƽ��1Ϊ��ֱ��
float 	xdata StationLongS;						//����վ����
BOOL  	xdata StationEastFlagS;					//����վ������־
float 	xdata StationLatS;						//����վγ��
BOOL  	xdata StationNorthFlagS;				//����վ��γ��־
float 	xdata SatLongS;  						//���Ǿ���
BOOL 	xdata SatLongEastFlagS;					//���Ƕ�������־
UINT16 	xdata ReceiverKindFlagS;				//���ջ�����жϱ�־

double 	xdata SharpRecFreqS;					//���ս��ջ�Ƶ��,��3 ��Ƶ��
float  	xdata SharpThresholdS;					//���ս��ջ�����

float 	xdata ThresholdS;						//����

double 	xdata XinBiaoRecFreqS;					//�ű���ջ�Ƶ��
float  	xdata XinBiaoThresholdS;				//�ű�����
float 	xdata SrcStarKbS;						//��ǰ������
//*********************2009-6-19ʵʱ���ǲ���****************/


float 	xdata SSrcStarKbS;						//���浽EEPROM�е����ݣ�Ŀ�����Ƿ�����
UINT8 	xdata EnStorSrcPara;					//ʹ�ܱ���λ


UINT8 	xdata BSarr[120];

UINT8	xdata BaseNumber;
struct BaseStarStr bssT[4];		 	//��ǰʹ�õĲ������ǣ�
struct BaseStarStr bsT;		 	
UINT8 	xdata BaseStarFlagThree;
UINT16 xdata AntReadySearchF;		//2009/10/12���ڸ�λʱ�����ٴθ�λ


UINT8 xdata XinBiaoFlagNum;
UINT8 xdata XinBiaoVorX;
/***********************************************************************
*
* ����ԭ�ͣ�void TimeTest(int count)
*
* ��ڲ�����count(16λ)
*
* ���ڲ�������
*
* ���������������ʱ
*
***********************************************************************/
void TimeTest(int count)
{
	int i;
	for(i = 0; i < count * 3; i++)
	{
		Delay(60000);
	}
}


/***********************************************************************
*
* ����ԭ�ͣ�void Delay(UINT16 t)
*
* ��ڲ�����t(16λ)
*
* ���ڲ�������
*
* ���������������ʱ��24MHz, about 0.5us per machine circle or 12MHz, about 1us per machine circle
*
***********************************************************************/
void Delay(UINT16 t)
{
	while(--t)
	{
		;
	}
}								 


/***********************************************************************
*
* ����ԭ�ͣ�void itoa(int i,char *p,int weishu)
*
* ��ڲ�����iΪҪת����ʮ�����з����������ݣ�weishuΪ�޶�λ��
*
* ���ڲ������ַ�����
*
* ������������int��תΪ����,�ֱ�������й̶���ʽ��Э����
*
* ʵ��˵����
*
* 1����λ183.4,�Ŵ�10����Ϊ1834������λ��Ӧ��Ϊ4���ֱ�Ϊ'1''8''3''4'
*
*    ��λ 83.4,�Ŵ�10����Ϊ 834������λ��Ӧ��Ϊ4���ֱ�Ϊ'0''8''3''4'
*
*    ��λ  3.4,�Ŵ�10����Ϊ  34������λ��Ӧ��Ϊ4���ֱ�Ϊ'0''0''3''4'
*
* 2������83.4,�Ŵ�10����Ϊ 834������λ��Ӧ��Ϊ3���ֱ�Ϊ'8''3''4'
*
*    ���� 3.4,�Ŵ�10����Ϊ  34������λ��Ӧ��Ϊ3���ֱ�Ϊ'0''3''4'
*
***********************************************************************/
void itoa(int i, char *p, int weishu)
{
    if(i < 0)
    {
		i = 0 - i;
	}
    while(weishu > 0)
    {
        *(p + weishu - 1) = i % 10 + '0';
        i /= 10;
        weishu--;
    }   
}


/***********************************************************************
*
* ����ԭ�ͣ�void FindAnt(float slong, float slat, float starlong, int plomode)
*
* ��ڲ�����slong    ����վ����
*			
*    		slat     ����վγ��
*
*			starlong ���Ǿ���
*
*			plomode  ������ʽ��0Ϊˮƽ����0Ϊ��ֱ
*
* ���ڲ�������
*
* ����������������ֵ�����ڣ�StationAzCal, StationElCal,StationPolCal
*
***********************************************************************/
void FindAnt(float slong, float slat, float starlong, BOOL plomode)
{
	float delta;

	if(StationEastFlag == WEST)		//�������վ����������
	{
		slong = 360.0 - slong;	
	}

	if(SatLongEastFlagS == WEST)	//�����������������
	{
		starlong = 360.0 - starlong;	
	}

	if(fabs(slong - starlong) > 270.0) //�ڲ�ͬ���ȱ���ʱ
	{
		if(slong < 91.0)
		{
			slong += 360.0;
		}
		else
		{
			starlong += 360.0;
		}
	}
		
	delta = starlong - slong;
	
	delta *= PI / 180.0;
	slat  *= PI / 180.0;
	
	StationAzCal = atan(tan(delta) / sin(slat));
	StationAzCal *= 180.0 / PI;
	StationAzCal = 180.0 - StationAzCal;
				
	StationElCal = atan((cos(delta) * cos(slat) - 0.15127) \
					 / sqrt(1 - pow(cos(delta) * cos(slat), 2)));
	StationElCal *= 180.0 / PI;
	
	StationPolCal = atan(sin(delta) / tan(slat));
	StationPolCal *= 180.0 / PI;
	
	StationPolCal = 45;

	if(plomode == V)
	{
		StationPolCal = 45;
//		if(StationPolCal < 0.0)
//		{
//			StationPolCal += 90.0;
//		}
//		else
//		{
//			StationPolCal -= 90.0;
//		}
	}
	if(plomode == H)
	{
		StationPolCal = 315;
	}

	if(StationNorthFlag == SOUTH)	   //������ϰ���
	{
//		StationPolCal = -StationPolCal;
		StationAzCal = 360.0 - StationAzCal;
	}
}
