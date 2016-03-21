/***********************************************************************
* 
* ʱ    �䣺2008-11-24
*
* �� �� ����rweeprom.c
*
* ��    ����AKD10P02/AKD09P02(���԰�)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
* 
* ����˵����EEPROM��д����
*
* ��    �ģ� ��дEEPROM�����ʼ����ʱ����ǧ�ӵ�����
*
***********************************************************************/
#include "c8051f120.h"
#include "rweeprom.h"
#include "station.h"
#include "timer.h"


/***********************************************************************
*
* ����ԭ�ͣ�static void Send(UINT8 ByteData)//���һ���ֽ�
*
* ��ڲ�����һ���ֽ�����
*
* ���ڲ�������
*
* ������������ģ���ڲ���װ����һ���ֽ����ݵ�93C66;���һ���ֽ�
*
***********************************************************************/
static void Send(char ByteData)
{  
	UINT8 i;
	SFRPAGE = CONFIG_PAGE;	  //ҳF  120�޸�
	SK_93C66_0;						//ʱ��0
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
		SK_93C66_1;					//ʱ��1
		Delay(TIME);
		SK_93C66_0;					//ʱ��0
		Delay(TIME);
	}
}


/***********************************************************************
*
* ����ԭ�ͣ�UINT8 ReadEEPROM(char Address)
*
* ��ڲ�����һ���ֽ����ݱ�ʾ��ַ
*
* ���ڲ�������
*
* ����������������ַ������
*
***********************************************************************/
UINT8 ReadEEPROM(char Address)
{
	char read = 0x0c, I_data = 0;	//�⼸����������ο�93C66оƬ�ĵ�
	int i;

	SFRPAGE = CONFIG_PAGE;	  //ҳF  120�޸�
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
* ����ԭ�ͣ�void WriteEEPROM(UINT8 Data,UINT8 Address)
*
* ��ڲ�����һ���ֽ����ݺ�һ���ֽڵ�ַ����
*
* ���ڲ�������
*
* ����������дһ���ֽڵ�Address
*
***********************************************************************/ 
void WriteEEPROM(UINT8 Data, char Address)
{  
	UINT8 i;
	char EN_D = 0x09, EN_RD = 0x80, ERASE_D = 0x0e, \
		 WRITE_D = 0x0a, EWDS = 0x10;			//�⼸����������ο�93C66оƬ�ĵ�


	SFRPAGE = CONFIG_PAGE;	  //ҳF  120�޸�
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
