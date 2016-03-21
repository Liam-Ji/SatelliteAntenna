				  /***********************************************************************
* 
* 时间：2008-10-10
*
* 文件名：pubic.h
*
* 作者： 北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
*
***********************************************************************/
#ifndef __PUBLIC_H__
#define __PUBLIC_H__


typedef signed char 		INT8;
typedef signed short int 	INT16;
typedef signed long 		INT32;
typedef unsigned char 		UINT8;
typedef unsigned short int 	UINT16;
typedef unsigned long 		UINT32;
typedef unsigned char 		BOOL;


#define BIT0 		0x01
#define BIT1 		0x02
#define BIT2 		0x04
#define BIT3 		0x08
#define BIT4 		0x10
#define BIT5 		0x20
#define BIT6 		0x40
#define BIT7 		0x80


#define TRUE 		1
#define FALSE		0
#define PI 			3.1415926L

extern xdata UINT8  TEMPAGE;

#endif