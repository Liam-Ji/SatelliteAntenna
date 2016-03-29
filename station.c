/***********************************************************************
* 
* 时    间：2008-10-9
*
* 文 件 名：main.c
*
* 版    本：AKD10P07(发布版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：一些与天线相关的全局变量
*
***********************************************************************/
#include "station.h"
#include "math.h"
#include "public.h"


float xdata StationAzCal;						//地球站方位计算值
float xdata StationElCal;						//地球站俯仰计算值
float xdata StationPolCal;						//地球站极化计算值
float xdata StationElTemp;						//2009/5/22加入


BOOL  xdata StationPloMode;						//地球站极化方式（0为水平，1为垂直）
float xdata StationLong;						//地球站经度
BOOL  xdata StationEastFlag;					//地球站东经标志
float xdata StationLat;							//地球站纬度
BOOL  xdata StationNorthFlag;					//地球站北纬标志
float xdata SatLong;  							//卫星经度
float xdata SatLongEastFlag;  					//卫星经度


double xdata SharpRecFreq;						//夏普接收机频率,亚3 的频率
BOOL   xdata SharpRecNormalFlag = 0;			//夏普接收机正常标志
float  xdata SharpThreshold;					//夏普接收机门限
float  xdata DVBSymbolRate;						//夏普接收机符号率
UINT8  xdata DVBLockFlag;


double xdata XinBiaoRecFreq;					//信标接收机频率
BOOL   xdata XinBiaoRecNormalFlag = 0;			//信标接收机正常标志
float  xdata XinBiaoThreshold;					//信标门限


UINT8  xdata freq_seq[40]={0};					//频率码字数组


float xdata AGCNoiseNor	= 0.1;					//正常的噪声
float xdata AGCNoise;							//噪声
float xdata AGC;								//AGC采样
float xdata Threshold;							//门限


UINT8 xdata status;								//天线工作状态
float xdata AZSearchRange = 30.0;				//方位搜索范围
float xdata ELSearchRange = 5.0;				//俯仰搜索范围
float xdata AZlimit		  = 90.0;				//AZ最大与此180度的差
float xdata AZ180		  = 180.0;
float xdata ELUPlimit 	  = 75.0;
float xdata ELDOWNlimit	  = ELDOWNlimitDef;



BOOL LockFlag  = 0;		    					//跟踪成功标志位
BOOL ResetFlag = 0;								//复位标志


UINT8  xdata NoMonitorF;						//无监控标志


BOOL BianBanFlag = 0;							//边瓣安装完毕标志
BOOL UninstallBianbanFlag = 0;					//边瓣卸载完毕标志
BOOL Abnormal;									//非正常断电标志
BOOL TestFlag = 0;								//测试收藏标志


UINT8  xdata BaseStarLONGEF;					//参考卫星东西经标致
float  xdata BaseStarLONG;						//基准卫星经度
UINT8  xdata BaseStarPol;						//极化方式
double xdata BaseStarFreq;						//频率
float  xdata BaseStarKbS;						//符号率
UINT8  xdata BaseStarRType;						//接收机类型
UINT8  xdata EnBaseStar;						//是否使用基准卫星


xdata int StoreStatus1 = 0;
//--------------------------------------------------------------
xdata int WireFlag = 0;
xdata char Buff[10] = {'\0'};
xdata int len1 = 0;
xdata char WireBuff[500] = {'\0'};
xdata int len = 0;
xdata int len2 = 0;
//--------------------------------------------------------------



//*********************2009-6-19实时卫星参数****************/
BOOL  	xdata StationPloModeS;					//地球站极化方式（0为水平，1为垂直）
float 	xdata StationLongS;						//地球站经度
BOOL  	xdata StationEastFlagS;					//地球站东经标志
float 	xdata StationLatS;						//地球站纬度
BOOL  	xdata StationNorthFlagS;				//地球站北纬标志
float 	xdata SatLongS;  						//卫星经度
BOOL 	xdata SatLongEastFlagS;					//卫星东西经标志
UINT16 	xdata ReceiverKindFlagS;				//接收机类别判断标志

double 	xdata SharpRecFreqS;					//夏普接收机频率,亚3 的频率
float  	xdata SharpThresholdS;					//夏普接收机门限

float 	xdata ThresholdS;						//门限

double 	xdata XinBiaoRecFreqS;					//信标接收机频率
float  	xdata XinBiaoThresholdS;				//信标门限
float 	xdata SrcStarKbS;						//当前符号率
//*********************2009-6-19实时卫星参数****************/


float 	xdata SSrcStarKbS;						//保存到EEPROM中的数据，目标卫星符号率
UINT8 	xdata EnStorSrcPara;					//使能保存位


UINT8 	xdata BSarr[120];

UINT8	xdata BaseNumber;
struct BaseStarStr bssT[4];		 	//当前使用的参数卫星，
struct BaseStarStr bsT;		 	
UINT8 	xdata BaseStarFlagThree;
UINT16 xdata AntReadySearchF;		//2009/10/12用于复位时可以再次复位


UINT8 xdata XinBiaoFlagNum;
UINT8 xdata XinBiaoVorX;
/***********************************************************************
*
* 函数原型：void TimeTest(int count)
*
* 入口参数：count(16位)
*
* 出口参数：无
*
* 功能描述：大概延时
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
* 函数原型：void Delay(UINT16 t)
*
* 入口参数：t(16位)
*
* 出口参数：无
*
* 功能描述：大概延时。24MHz, about 0.5us per machine circle or 12MHz, about 1us per machine circle
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
* 函数原型：void itoa(int i,char *p,int weishu)
*
* 入口参数：i为要转换的十进制有符号整形数据，weishu为限定位数
*
* 出口参数：字符数组
*
* 功能描述：将int型转为符号,分别填入具有固定格式的协议内
*
* 实例说明：
*
* 1，方位183.4,放大10倍后为1834，数组位数应该为4，分别为'1''8''3''4'
*
*    方位 83.4,放大10倍后为 834，数组位数应该为4，分别为'0''8''3''4'
*
*    方位  3.4,放大10倍后为  34，数组位数应该为4，分别为'0''0''3''4'
*
* 2，俯仰83.4,放大10倍后为 834，数组位数应该为3，分别为'8''3''4'
*
*    俯仰 3.4,放大10倍后为  34，数组位数应该为3，分别为'0''3''4'
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
* 函数原型：void FindAnt(float slong, float slat, float starlong, int plomode)
*
* 入口参数：slong    地球站经度
*			
*    		slat     地球站纬度
*
*			starlong 卫星经度
*
*			plomode  极化方式，0为水平，非0为垂直
*
* 出口参数：无
*
* 功能描述：计算后的值保存在：StationAzCal, StationElCal,StationPolCal
*
***********************************************************************/
void FindAnt(float slong, float slat, float starlong, BOOL plomode)
{
	float delta;

	if(StationEastFlag == WEST)		//如果地球站是在西半球
	{
		slong = 360.0 - slong;	
	}

	if(SatLongEastFlagS == WEST)	//如果卫星是在西半球
	{
		starlong = 360.0 - starlong;	
	}

	if(fabs(slong - starlong) > 270.0) //在不同经度标致时
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

	if(StationNorthFlag == SOUTH)	   //如果是南半球
	{
//		StationPolCal = -StationPolCal;
		StationAzCal = 360.0 - StationAzCal;
	}
}
