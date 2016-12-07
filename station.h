/***********************************************************************
* 
* ʱ�䣺2008-10-15
*
* �ļ�����station.h
*
* ���ߣ� �������Ƶ���ϢͨѶ�������޹�˾�������������
*
***********************************************************************/
#ifndef __STATION_H__					
#define __STATION_H__


#include "public.h"
#include "c8051f120.h"


#define EAST 	1				//����
#define WEST 	0				//����
#define NORTH 	1				//��γ
#define SOUTH 	0				//��γ

#define STOP	0
#define RIGHT	1
#define UP		2
#define LEFT	3
#define DOWN	4


#define H 		0				//ˮƽ����
#define V 		1				//��ֱ����


#define INIT			0		//��ʼ��״̬
#define SEARCHREADY		1		//Ԥ����״̬
#define	SEARCHING		2		//��������״̬
#define TRACKING    	3		//����״̬
#define MANUALMODE 		4		//�ֶ�����ģʽ
#define STORESTATUS 	5		//�ղ�״̬

#define ELDOWNlimitDef	32.0     //�½������


//#define LEDLOCK 	(P6&=~BIT6)						//��������(���Ի���)
//#define LEDLOCKLOST (P6|=BIT6)					//��������
//#define LEDLOCK (P6&=~BIT5)						//�»�����(������15KG25KG)
//#define LEDLOCKLOST (P6|=BIT5)

//2009-3-4-----------------------------------------------------------------------
/*25KG15KG���е�*/
#define LEDRED P0 &= ~BIT6; P0 |= BIT7	  			//LED��ɫ��һ��ͨ����ĵƣ���ʵ����ɫ
#define LEDGREEN 	 P0 |= BIT6; P0 &= ~BIT7	  	//LED��ɫ��һ��ͨ����ĵƣ�
#define LEDCLOSE P0 |= BIT6; P0 |= BIT7				//LCED�أ�һ��ͨ����ĵƣ�


/*25KG���е�*/
//#define LEDGREENOPEN 	(P0 |=  BIT5)	  			//LED��ɫ��������ָʾ����ĵƣ�
//#define LEDGREENCLOSE 	(P0 &= ~BIT5)			//LED��ɫ��
//#define LEDREDCOPEN 	(P0 |=  BIT4)				//LED��ɫ��������ָʾ����ĵƣ�
//#define LEDREDCLOSE 	(P0 &= ~BIT4)				//LED��ɫ��


/*2009-5-6�޸����*/
#define LEDGREENOPEN 	P0 &= ~BIT5; P0 |=  BIT4	//LED��ɫ��������ָʾ����ĵƣ�
#define LEDREDCOPEN 	P0 |=  BIT5; P0 &= ~BIT4	//LED��ɫ��������ָʾ����ĵƣ�
#define LEDGREENCLOSE 	P0 &= ~BIT5; P0 &= ~BIT4	//LED��ɫ��
#define LEDREDCLOSE 	P0 |=  BIT5; P0 |=  BIT4	//LED��ɫ��



#define LEDLOCK 	LEDGREENOPEN					//�»�����(������15KG25KG)
#define LEDLOCKLOST LEDGREENCLOSE


#define POWEROPEN	(P1 |=  BIT5)					//����
#define POWERCLOSE	(P1 &= ~BIT5)					//������
#define SWWIPORT 	(P1 &   BIT4)              		//�������л�


#define DVBSTART	(P5 &= ~BIT5)					//�͵�ƽ
#define DVBEND		(P5 |=  BIT5)					//�ߵ�ƽ
//2009-3-4-----------------------------------------------------------------------


extern float xdata StationAzCal;					//����վ��λ����ֵ
extern float xdata StationElCal;					//����վ��������ֵ
extern float xdata StationPolCal;					//����վ��������ֵ
extern float xdata StationElTemp;


extern BOOL  xdata StationPloMode;					//����վ������ʽ��0Ϊˮƽ��1Ϊ��ֱ��
extern float xdata StationLong;						//����վ����
extern BOOL  xdata  StationEastFlag;				//����վ������־
extern float xdata StationLat;						//����վγ��
extern BOOL  xdata StationNorthFlag;				//����վ��γ��־
extern float xdata SatLong;  						//���Ǿ���
extern float xdata SatLongEastFlag;  				//���Ǿ���

									   		
extern double xdata SharpRecFreq;					//���ս��ջ�Ƶ��,��3 ��Ƶ��
extern BOOL   xdata SharpRecNormalFlag;				//���ս��ջ�������־
extern float  xdata SharpThreshold;					//���ս��ջ�����
extern float  xdata DVBSymbolRate;					//���ս��ջ�������
extern UINT8  xdata DVBLockFlag;


extern double xdata XinBiaoRecFreq;					//�ű���ջ�Ƶ��
extern BOOL   xdata XinBiaoRecNormalFlag;			//�ű���ջ�������־
extern float  xdata XinBiaoThreshold;				//�ű�����
											   	

extern UINT8  xdata freq_seq[40];					//Ƶ����������


extern float xdata AGCNoiseNor;						//����������
extern float xdata AGCNoise;						//��������
extern float xdata AGC;								//������AGC
extern float xdata Threshold;						//����������


extern UINT8 xdata status;							//���߹���״̬
extern float xdata AZSearchRange;					//��λ������Χ,30,
extern float xdata ELSearchRange;					//����������Χ,5,
extern float xdata AZlimit;							//AZ������180�ȵĲ�,60,
extern float xdata AZ180;							//180��
extern float xdata ELUPlimit;						//75��
extern float xdata ELDOWNlimit;						//10��


extern BOOL LockFlag;
extern BOOL ResetFlag;


extern UINT8  xdata NoMonitorF;						//�޼�ر�־


extern BOOL xdata BianBanFlag;						//�߰갲װ��ϱ�־
extern BOOL xdata UninstallBianbanFlag;				//�߰�ж����ϱ�־
extern BOOL Abnormal;								//�������ϵ��־
extern xdata int StoreStatus1;


/*2009-6-9���룬���׼������ز���*/
extern UINT8  xdata BaseStarLONGEF;					//�ο����Ƕ���������
extern float  xdata BaseStarLONG;					//��׼���Ǿ���
extern UINT8  xdata BaseStarPol;					//������ʽ
extern double xdata BaseStarFreq;					//Ƶ��
extern float  xdata BaseStarKbS;					//������
extern UINT8  xdata BaseStarRType;					//���ջ�����
extern UINT8  xdata EnBaseStar;						//�Ƿ�ʹ�û�׼����


extern float xdata SrcStarKbS;						//Ŀ�����Ƿ�����


extern BOOL  	xdata StationPloModeS;					//����վ������ʽ��0Ϊˮƽ��1Ϊ��ֱ��
extern float 	xdata StationLongS;						//����վ����
extern BOOL  	xdata StationEastFlagS;					//����վ������־
extern float 	xdata StationLatS;						//����վγ��
extern BOOL  	xdata StationNorthFlagS;				//����վ��γ��־
extern float 	xdata SatLongS;  						//���Ǿ���
extern BOOL 	xdata SatLongEastFlagS;					//������������־
extern UINT16 	xdata ReceiverKindFlagS;				//���ջ�����жϱ�־

extern double 	xdata SharpRecFreqS;					//���ս��ջ�Ƶ��,��3 ��Ƶ��
extern float  	xdata SharpThresholdS;					//���ս��ջ�����

extern double 	xdata XinBiaoRecFreqS;					//�ű���ջ�Ƶ��
extern float  	xdata XinBiaoThresholdS;				//�ű�����
extern float 	xdata SSrcStarKbS;						//���浽EEPROM�е�����
extern float 	xdata ThresholdS;

extern UINT8	xdata EnStorSrcPara;
extern UINT8 	xdata BSarr[];

extern UINT8	xdata BaseNumber; //�ο����Ǹ�����
extern struct BaseStarStr bssT[];
extern struct BaseStarStr bsT;

extern UINT8 	xdata BaseStarFlagThree;
extern UINT8 xdata XinBiaoFlagNum;
extern UINT8 xdata XinBiaoVorX;


struct BaseStarStr
{
	float 	Long;
	UINT8 	PolF;
	UINT8 	RecF;
	UINT8 	WEFlag;
	double	Freq;
	float 	KitB;
};

extern UINT16 xdata AntReadySearchF;

//---------------------------------
extern xdata int WireFlag;
extern xdata char WireBuff[];
extern xdata int len;
extern xdata char Buff[];
extern xdata int len1;
extern xdata int len2;
//---------------------------------


extern void TimeTest(int count);
extern void Delay(UINT32 t);						//�����ӳ�
extern void itoa(int i, char *p, int jinzhishu);
extern void FindAnt(float slong, float slat, float starlong, BOOL plomode);
extern void PolarAngleInit(void);

#endif