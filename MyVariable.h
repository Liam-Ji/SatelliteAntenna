/**********************************************************************
文件说明：对控制板定义各口的含义

	p0.0 ->  TXD0(TX0)
	p0.1 ->  RXD0(RX0);	串口0分别与GPS,倾斜仪，电子罗盘，极化通信	   
	p0.2 ->  TXD1(RX1)		      
	p0.3 ->  RXD1(TX1);

	P2.0 ->  AZ-S4-GL；	扩展用,现扩展为极化电机
	P2.1 ->  AZ-S3-GL；	扩展用,现扩展为极化电机
	P2.2 ->  AZ-S2-GL；	扩展用
	P2.3 ->  AZ-S1-GL；	扩展用
	P2.4 ->  AZ-CP	；	方位电机脉冲
	P2.5 ->  AZ-DIR；	方位电机方向
	P2.6 ->  EL-CP	；	俯仰电机脉冲
	P2.7 ->  EL-DIR；	俯仰电机方向

	P3.7 <-  AZZEROL;	方位左限位
	P3.6 <-  AZZEROR;	方位右限位
	P3.5 <-  ELZEROU;	俯仰上限位
	P3.4 <-  ELZEROD;	俯仰下限位
	P3.3 <-  SP1；		扩展用
	P3.2 <-  SP2；		扩展用
	P3.1 <-  SP3；		扩展用
	P3.0 <-  SP4；		扩展用

	p5.2 ->  CS_93C66
	p5.3 ->  SK_93C66
	p5.4 ->  DI_93C66
	p5.5 <-  DO_93C66
	P5.6 ->  SR-SCLK;	设置夏普接收机频率用	    
	p5.7 ->  SR-SDATA;	

	P6.0 ->  SDATA
	P6.1 ->  SCLK
	P6.2 ->  CS3296
	P6.3 ->  CS8050；	设置信标接收机频率用

	P6.4 ->  LEDAUTO;
	P6.5 ->  LEDLOCK;
	P6.6 ->  LEDALARM;
	P6.7 <-  LOCK;

	p7.0 ->  SELECTO;
	p7.1 ->  SELECT1;	用来选择串口0分别与GPS,倾斜仪，电子罗盘，极化通信

          剩下的引脚均为GPIO(通用的I/0口)

(注意：配置时一定要遵守优先级，详情请看C8051F020.PDF文档)

***********************************************************************/

#ifndef _MYVARIABLE_H_
#define _MYVARIABLE_H_


#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80


#endif
