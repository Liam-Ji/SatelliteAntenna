/***********************************************************************
* 
* 时间：2008-10-15
*
* 文件名：station.h
*
* 作者： 北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*
***********************************************************************/
#ifndef __STATION_H__					
#define __STATION_H__


#include "public.h"
#include "c8051f120.h"


#define EAST 	1				//东经
#define WEST 	0				//西经
#define NORTH 	1				//北纬
#define SOUTH 	0				//南纬

#define STOP	0
#define RIGHT	1
#define UP		2
#define LEFT	3
#define DOWN	4


#define H 		0				//水平极化
#define V 		1				//垂直极化


#define INIT			0		//初始化状态
#define SEARCHREADY		1		//预搜索状态
#define	SEARCHING		2		//正在搜索状态
#define TRACKING    	3		//跟踪状态
#define MANUALMODE 		4		//手动工作模式
#define STORESTATUS 	5		//收藏状态

#define ELDOWNlimitDef	32.0     //下降到最底


//#define LEDLOCK 	(P6&=~BIT6)						//锁定灯亮(测试机用)
//#define LEDLOCKLOST (P6|=BIT6)					//锁定灯灭
//#define LEDLOCK (P6&=~BIT5)						//新机器用(测试用15KG25KG)
//#define LEDLOCKLOST (P6|=BIT5)

//2009-3-4-----------------------------------------------------------------------
/*25KG15KG都有的*/
#define LEDRED P0 &= ~BIT6; P0 |= BIT7	  			//LED红色（一键通上面的灯）其实是蓝色
#define LEDGREEN 	 P0 |= BIT6; P0 &= ~BIT7	  	//LED绿色（一键通上面的灯）
#define LEDCLOSE P0 |= BIT6; P0 |= BIT7				//LCED关（一键通上面的灯）


/*25KG独有的*/
//#define LEDGREENOPEN 	(P0 |=  BIT5)	  			//LED绿色亮（天线指示上面的灯）
//#define LEDGREENCLOSE 	(P0 &= ~BIT5)			//LED绿色灭
//#define LEDREDCOPEN 	(P0 |=  BIT4)				//LED红色亮（天线指示上面的灯）
//#define LEDREDCLOSE 	(P0 &= ~BIT4)				//LED红色灭）


/*2009-5-6修改完成*/
#define LEDGREENOPEN 	P0 &= ~BIT5; P0 |=  BIT4	//LED绿色亮（天线指示上面的灯）
#define LEDREDCOPEN 	P0 |=  BIT5; P0 &= ~BIT4	//LED红色亮（天线指示上面的灯）
#define LEDGREENCLOSE 	P0 &= ~BIT5; P0 &= ~BIT4	//LED绿色灭
#define LEDREDCLOSE 	P0 |=  BIT5; P0 |=  BIT4	//LED红色灭



#define LEDLOCK 	LEDGREENOPEN					//新机器用(测试用15KG25KG)
#define LEDLOCKLOST LEDGREENCLOSE


#define POWEROPEN	(P1 |=  BIT5)					//供电
#define POWERCLOSE	(P1 &= ~BIT5)					//不供电
#define SWWIPORT 	(P1 &   BIT4)              		//有无线切换


#define DVBSTART	(P5 &= ~BIT5)					//低电平
#define DVBEND		(P5 |=  BIT5)					//高电平
//2009-3-4-----------------------------------------------------------------------


extern float xdata StationAzCal;					//地球站方位计算值
extern float xdata StationElCal;					//地球站俯仰计算值
extern float xdata StationPolCal;					//地球站极化计算值
extern float xdata StationElTemp;


extern BOOL  xdata StationPloMode;					//地球站极化方式（0为水平，1为垂直）
extern float xdata StationLong;						//地球站经度
extern BOOL  xdata  StationEastFlag;				//地球站东经标志
extern float xdata StationLat;						//地球站纬度
extern BOOL  xdata StationNorthFlag;				//地球站北纬标志
extern float xdata SatLong;  						//卫星经度
extern float xdata SatLongEastFlag;  				//卫星经度

									   		
extern double xdata SharpRecFreq;					//夏普接收机频率,亚3 的频率
extern BOOL   xdata SharpRecNormalFlag;				//夏普接收机正常标志
extern float  xdata SharpThreshold;					//夏普接收机门限
extern float  xdata DVBSymbolRate;					//夏普接收机符号率
extern UINT8  xdata DVBLockFlag;


extern double xdata XinBiaoRecFreq;					//信标接收机频率
extern BOOL   xdata XinBiaoRecNormalFlag;			//信标接收机正常标志
extern float  xdata XinBiaoThreshold;				//信标门限
											   	

extern UINT8  xdata freq_seq[40];					//频率码字数组


extern float xdata AGCNoiseNor;						//正常的噪声
extern float xdata AGCNoise;						//搜索噪声
extern float xdata AGC;								//搜索用AGC
extern float xdata Threshold;						//搜索用门限


extern UINT8 xdata status;							//天线工作状态
extern float xdata AZSearchRange;					//方位搜索范围,30,
extern float xdata ELSearchRange;					//俯仰搜索范围,5,
extern float xdata AZlimit;							//AZ最大与此180度的差,60,
extern float xdata AZ180;							//180度
extern float xdata ELUPlimit;						//75度
extern float xdata ELDOWNlimit;						//10度


extern BOOL LockFlag;
extern BOOL ResetFlag;


extern UINT8  xdata NoMonitorF;						//无监控标志


extern BOOL xdata BianBanFlag;						//边瓣安装完毕标志
extern BOOL xdata UninstallBianbanFlag;				//边瓣卸载完毕标志
extern BOOL Abnormal;								//非正常断电标志
extern xdata int StoreStatus1;


/*2009-6-9加入，与基准卫星相关部分*/
extern UINT8  xdata BaseStarLONGEF;					//参考卫星东西经标致
extern float  xdata BaseStarLONG;					//基准卫星经度
extern UINT8  xdata BaseStarPol;					//极化方式
extern double xdata BaseStarFreq;					//频率
extern float  xdata BaseStarKbS;					//符号率
extern UINT8  xdata BaseStarRType;					//接收机类型
extern UINT8  xdata EnBaseStar;						//是否使用基准卫星


extern float xdata SrcStarKbS;						//目标卫星符号率


extern BOOL  	xdata StationPloModeS;					//地球站极化方式（0为水平，1为垂直）
extern float 	xdata StationLongS;						//地球站经度
extern BOOL  	xdata StationEastFlagS;					//地球站东经标志
extern float 	xdata StationLatS;						//地球站纬度
extern BOOL  	xdata StationNorthFlagS;				//地球站北纬标志
extern float 	xdata SatLongS;  						//卫星经度
extern BOOL 	xdata SatLongEastFlagS;					//卫经东西经标志
extern UINT16 	xdata ReceiverKindFlagS;				//接收机类别判断标志

extern double 	xdata SharpRecFreqS;					//夏普接收机频率,亚3 的频率
extern float  	xdata SharpThresholdS;					//夏普接收机门限

extern double 	xdata XinBiaoRecFreqS;					//信标接收机频率
extern float  	xdata XinBiaoThresholdS;				//信标门限
extern float 	xdata SSrcStarKbS;						//保存到EEPROM中的数据
extern float 	xdata ThresholdS;

extern UINT8	xdata EnStorSrcPara;
extern UINT8 	xdata BSarr[];

extern UINT8	xdata BaseNumber; //参考卫星个数；
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
extern void Delay(UINT32 t);						//大致延迟
extern void itoa(int i, char *p, int jinzhishu);
extern void FindAnt(float slong, float slat, float starlong, BOOL plomode);
extern void PolarAngleInit(void);

#endif