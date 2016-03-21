/***********************************************************************
* 
* 时间：2008-10-13
*
* 文件名：uart.h
*
* 作者： 北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*
***********************************************************************/
#ifndef __UART0_H__
#define __UART0_H__


#include "public.h"					    


#define UART0INTNUMBER		4			//串口0的中断号
#define UART1INTNUMBER		20			//串口1的中断号
#define MAXLENGTH			100			//两个串口接收发送数据最大长度


#define SELECT0L (P6 &= ~BIT5)			//
#define SELECT0H (P6 |=  BIT5)			//
#define SELECT1L (P6 &= ~BIT6)			//
#define SELECT1H (P6 |=  BIT6)			//为了切换串口0到GPS，罗盘，倾斜仪，极化通信


#define SYSCLK 				22118400L	//系统晶振频率
#define UARTBR9600 			9600L		//初始化串口波特率		 
#define UARTBR4800			4800L		//初始化串口波特率
#define UARTBR19200			19200L		//初始化串口波特率


extern char  xdata Uart0R[];
extern float xdata GradientY;			//倾斜仪输出的倾斜度Y
extern float xdata GradientRightR;		//倾斜仪输出的倾斜度校正度数
extern int   xdata GradientNormal;		//倾斜仪不正常
extern int   xdata GPSNormal;			//GPS不正常
extern float xdata GPSLong;				//GPS输出的经度
extern BOOL  xdata GPSEastFlag;			//GPS东经标志
extern float xdata GPSLat;				//GPS输出的纬度
extern BOOL  xdata GPSNorthFlag;		//GPS北纬标志


extern char  xdata Uart1ReceData[];		//串口1接收数据
extern char  xdata Uart1SendData[];		//串口1发送数据
extern UINT8 xdata Uart1ReceDataLength;	//当前串口1要接收的数据缓冲长度
extern UINT8 xdata Uart1SendDataLength;	//当前串口1要发送的数据序列
extern UINT8 xdata Uart1TXMAXLenth;		//串口1要发送的最大长度
extern UINT8 xdata NumberOfComand;		//命令个数


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