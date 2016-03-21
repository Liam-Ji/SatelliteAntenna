/***********************************************************************
* 
* 时    间：2008-11-24
*
* 文 件 名：rweeprom.c
*
* 版    本：AKD10P02/AKD09P02(测试版)
*
* 作    者：北京爱科迪信息通讯技术有限公司—技术部—杨淳雯
* 
* 功能说明：EEPROM读写功能
*
* 修    改： 读写EEPROM函数最开始的延时从五千加到五万
*
***********************************************************************/
#include "c8051f120.h"
#include "rweeprom.h"
#include "station.h"
#include "timer.h"


/***********************************************************************
*
* 函数原型：static void Send(UINT8 ByteData)//输出一个字节
*
* 入口参数：一个字节数据
*
* 出口参数：无
*
* 功能描述：本模块内部封装，发一个字节数据到93C66;输出一个字节
*
***********************************************************************/
static void Send(char ByteData)
{  
	UINT8 i;
	SFRPAGE = CONFIG_PAGE;	  //页F  120修改
	SK_93C66_0;						//时钟0
	for(i = 0; i < 8; i++)
	{
		if(ByteData & 0x80)
		{
			DI_93C66_1;
		}
		else
		{
			DI_93C66_0;
		}
		Delay(TIME);
		ByteData <<= 1;
		SK_93C66_1;					//时钟1
		Delay(TIME);
		SK_93C66_0;					//时钟0
		Delay(TIME);
	}
}


/***********************************************************************
*
* 函数原型：UINT8 ReadEEPROM(char Address)
*
* 入口参数：一个字节数据表示地址
*
* 出口参数：无
*
* 功能描述：读出地址的数据
*
***********************************************************************/
UINT8 ReadEEPROM(char Address)
{
	char read = 0x0c, I_data = 0;	//这几个命令字请参考93C66芯片文档
	int i;

	SFRPAGE = CONFIG_PAGE;	  //页F  120修改
	Delay(1000);
	CS_93C66_0;
	SK_93C66_0;
	Delay(TIME);
	CS_93C66_1;
	Delay(TIME);
	Send(read);
	Delay(TIME);
	Send(Address);
	Delay(TIME);
	SK_93C66_0;

	for(i = 0; i < 8; i++)
	{
		I_data *= 2;
		Delay(TIME); 
		SK_93C66_1;
		Delay(TIME);
		if(DO_93C66 == BIT1)
		{
			I_data++;
		}
		SK_93C66_0;
	}
	CS_93C66_0;
	return (UINT8)(I_data);
}


/***********************************************************************
*
* 函数原型：void WriteEEPROM(UINT8 Data,UINT8 Address)
*
* 入口参数：一个字节数据和一个字节地址数据
*
* 出口参数：无
*
* 功能描述：写一个字节到Address
*
***********************************************************************/ 
void WriteEEPROM(UINT8 Data, char Address)
{  
	UINT8 i;
	char EN_D = 0x09, EN_RD = 0x80, ERASE_D = 0x0e, \
		 WRITE_D = 0x0a, EWDS = 0x10;			//这几个命令字请参考93C66芯片文档


	SFRPAGE = CONFIG_PAGE;	  //页F  120修改
	Delay(1000);
	CS_93C66_0;									//P5OUT&=~BIT2;
	Delay(TIME);
	CS_93C66_1;									//P5OUT|=BIT2;
	Delay(TIME);

	Send(EN_D);
	Delay(TIME);

	Send(EN_RD);   
	Delay(TIME);
	
	CS_93C66_0;									//P5OUT&=~BIT2; 
	Delay(TIME);
	CS_93C66_1;	
									
	Send(ERASE_D);
	Delay(TIME);

	Send(Address);
	Delay(TIME);

	CS_93C66_0;									//P5OUT&=~BIT2; 
	Delay(TIME);
	CS_93C66_1;									//P5OUT |=BIT2;
	Delay(TIME);
	OpenTimer0Interrupt();

	while(!(DO_93C66 == BIT1) && OverflowT0 < 2)
	{
		;
	}
	CloseTimer0Interrupt();
	if(OverflowT0 > 1)
	{
		return;									//EEPROM is error!!!
	}
	CS_93C66_0;									//P5OUT&=~BIT2;
	Delay(TIME);
	CS_93C66_1;									//P5OUT |=BIT2;
	Delay(TIME);
	  
	Send(WRITE_D);
	Delay(TIME);

	Send(Address); 
	SK_93C66_0;									//P5OUT&=~BIT3;
	for(i = 0; i < 8; i++)
	{ 
		if(Data & 0x80)
		{
			DI_93C66_1;							//P5OUT |=BIT4;
		}
		else
		{
			DI_93C66_0;							//P5OUT&=~BIT4; 
		}
		Delay(TIME);
		Data <<= 1;
		SK_93C66_1;								//P5OUT|=BIT3;
		Delay(TIME);
		SK_93C66_0;								//P5OUT&=~BIT3;
	}
	Delay(TIME);
	CS_93C66_0;									//P5OUT&=~BIT2;
	Delay(TIME);
	CS_93C66_1;									//P5OUT|=BIT2;
	Delay(TIME);
	Send(EWDS);
	CS_93C66_0;									//P5OUT&=~BIT2;
	Delay(TIME);
}
