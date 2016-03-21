/***********************************************************************
* 
* ʱ    �䣺2008-10-13
*
* �� �� ����uart0.c
*
* ��    ����AKD10P02/AKD09P02(���԰�)
*
* ��    �ߣ��������Ƶ���ϢͨѶ�������޹�˾�������������
* 
* ����˵����C8051F020����0,1����ع��ܺ���
*
*           1������0��Ҫ��GPS����б��ͨ�ţ�����������
*
*           2������1��Ҫ����λ��ͨ�Ź��ܣ��������л�����
*
***********************************************************************/
#include "uart.h"
#include "c8051f120.h"
#include "timer.h"
#include "station.h"
#include "rweeprom.h"


/*ȫ�ֱ���*/
char  xdata Uart0R[MAXLENGTH];				//����0��������
char  xdata Uart0T[MAXLENGTH];				//����0��������
float xdata GradientY;						//��б���������б��Y
float xdata GradientRightR = 0.0;			//��б���������б��У������
int   xdata GradientNormal;					//��б������


int   xdata GPSNormal;						//GPS����
float xdata GPSLong;						//GPS����ľ���
BOOL  xdata GPSEastFlag;					//GPS������־
float xdata GPSLat;							//GPS�����γ��
BOOL  xdata GPSNorthFlag;					//GPS��γ��־


char  xdata Uart1ReceData[MAXLENGTH] = {0};	//����1��������
char  xdata Uart1SendData[MAXLENGTH] = {0};	//����1��������
UINT8 xdata Uart1ReceDataLength 	 = 0;	//��ǰ����1Ҫ���յ����ݻ��峤��
UINT8 xdata Uart1SendDataLength 	 = 0;	//��ǰ����1Ҫ���͵���������
UINT8 xdata Uart1TXMAXLenth 	 	 = 0;	//����1Ҫ���͵���󳤶�
UINT8 xdata NumberOfComand 			 = 0;	//������� 


/*��̬����*/
static int  data Uart0Enflag;
static int  data Uart0RNum;
static int  data Uart0TNum;
static char data Uart0StartF;
static char data Uart0TxFlag;


/*��̬����*/
static void SelectGradient(void);
static void SelectGPS(void);
static void SelectXinBiao(void);
static void OpenUart1(void);
static void CloseUart1(void);
static void SetXinBiaoFreqNum(double Freq);		//2009-2-17
static void PutXinBiaoS(void);


/*�������л����*/
static int  GetG8(void);
static char GetG8Flag(void);
static void SetFlag(char);
static void InitWire(void);


/***********************************************************************
*
* ����ԭ�ͣ�void Uart0Init(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������0��ʼ��,��ʱ��1��Ϊʱ��Դ����ֵ�Զ���װ
*
***********************************************************************/
void Uart0Init(void)
{
	UINT16 Counter;  //����unsigned short int Counter  
	Counter = 65536-(SYSCLK/UARTBR19200/16);	  //ԭ����16��ע��
	EA=0;  			       //����ҳ
   
	SFRPAGE   = TMR4_PAGE; 
	TMR4CF    |= 0x08;	 //ϵͳʱ��
	TMR4CN    &= 0xFC;  //������ʽ��16λ�Զ����أ��Ͷ�ʱ������ 

	RCAP4L |= Counter%256;   // �Ѽ���ֵ�ĵ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4L=RCAP4L;	         //��ֵ����ʱ��4���ֽ� 
	RCAP4H |=Counter/256;
 
	TMR4H=RCAP4H;	

	SFRPAGE   = UART0_PAGE;
	SCON0     |= 0x50;	//��ʽ1��8 λUART���ɱ䲨���� ,	����UART0 �������� 
	SSTA0     |= 0x1F;	//��ʱ��4��Ϊ���պͷ���ʱ��Դ�������ʷ�����������ֹ�����ʼӱ�


	SFRPAGE   = TMR4_PAGE;
	TMR4CN    |= 0x04;	//��ʱ��4����

	ES0	   = 0;		   //������0�жϣ�����ҳ	  
	EA=1;
}


/***********************************************************************
*
* ����ԭ�ͣ�void OpenUart0(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������0��
*
***********************************************************************/
void OpenUart0(void)
{
	ES0	= 1;							//�򿪴���0�ж�
	SFRPAGE   = TMR4_PAGE;
	TMR4CN    &= ~0x80;		//������  
	TMR4CN    |= 0x04;	   //����ʱ��4
}


/***********************************************************************
*
* ����ԭ�ͣ�void CloseUart0(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������0��
*
***********************************************************************/
void CloseUart0(void)
{
	SFRPAGE   = TMR4_PAGE;
	TMR4CN    &= ~0x04;	//��ֹ��ʱ��4
	ES0	= 0;							//�رմ���0�ж�
}


/***********************************************************************
*
* ����ԭ�ͣ�void Uart0Interrupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������0�ж�
*
* ��    �ģ� 2009/9/11����Խ��ռ��������ı���
*
***********************************************************************/
void Uart0Interrupt(void) interrupt UART0INTNUMBER
{
	TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = UART0_PAGE;

	if(TI0)							  	//�����ж���Ӧ
	{
		TI0 = 0;
		if(Uart0TNum > MAXLENGTH - 1)
		{
			Uart0TNum = 0;
		}
		if(Uart0TxFlag == TRUE)
		{
			SBUF0 = Uart0T[Uart0TNum++];	
		}
	}
	
	if(RI0)								//�����ж���Ӧ
	{
		RI0 = 0;
		if(Uart0RNum < MAXLENGTH)		//�������2009.9.11
		{
			if(SBUF0 == Uart0StartF)	//�������0���ռĴ����е��ַ���$
			{
				Uart0Enflag = 1;		//���Ϊ��ʹ�ܡ���־�������ж��Ƿ�ʼ�洢����
			}
			if(Uart0Enflag == 1)
			{
				Uart0R[Uart0RNum++] = SBUF0;
			}
		}
		else
		{
			Uart0RNum = 0;				 //�������ʱ���������±���Ϊ0�����¿�ʼ�洢
		}
	}
	SFRPAGE = TEMPAGE;  //120�޸�
}


/***********************************************************************
*
* ����ԭ�ͣ�void SelectGPS(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������0�л���GPS�����ҽ��������л���4800
*
***********************************************************************/
static void SelectGPS(void)
{ 
	UINT16 Counter;  //����unsigned short int Counter 

	CloseUart0();
		
	SFRPAGE = 0x0F;	  //120�޸�	 ҳF
//	SELECT0H;		  //P6^5=1
//	SELECT1L;		  //P6^6=0	 ѡ��GPS
	SELECT0H;		  //P6^5=1
	SELECT1H;		  //P6^6=1	 ѡ��GPS

    SFRPAGE   = TMR4_PAGE;
	TMR4CN    &= ~0x04;	//��ֹ��ʱ��4
							 
	Counter=65536-(SYSCLK/UARTBR9600/16);	
   	RCAP4L=Counter&0x00ff;   // �Ѽ���ֵ�ĵ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4L=RCAP4L;	         //��ֵ����ʱ��4���ֽ� 
	Counter=Counter>>8;
	RCAP4H=Counter&0x00ff;	 //�Ѽ���ֵ�ĸ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4H=RCAP4H;
							
	TMR4CN    &= ~0x80;		//������  
	TMR4CN    |= 0x04;	   //����ʱ��4 

	OpenUart0();		   //�򿪴����жϣ�׼������GPS��Ϣ
}


/***********************************************************************
*
* ����ԭ�ͣ�void SelectGradient(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������0�л�����б�ǣ����ҽ��������л���9600
*
***********************************************************************/
static void SelectGradient(void)
{
	UINT16 Counter;  //����unsigned short int Counter  
	
	CloseUart0(); 	

	SFRPAGE = 0x0F;	  //120�޸�	 ҳF
	SELECT0L;
	SELECT1L;

    SFRPAGE   = TMR4_PAGE;
	TMR4CN    &=0xFB;	//��ֹ��ʱ��4
							 
	Counter=65536-(SYSCLK/UARTBR9600/16);	
   	RCAP4L=Counter&0x00ff;   // �Ѽ���ֵ�ĵ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4L=RCAP4L;	         //��ֵ����ʱ��4���ֽ� 
	Counter=Counter>>8;
	RCAP4H=Counter&0x00ff;	 //�Ѽ���ֵ�ĸ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4H=RCAP4H;
 
	TMR4CN    |= 0x04;	   //����ʱ��4 

	OpenUart0();
}


/***********************************************************************
*
* ����ԭ�ͣ�void SelectXinBiao(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ����������ѡ���ű���ջ�

*
* ����ʱ�䣺2009-1-4�����
*
***********************************************************************/
static void SelectXinBiao(void)
{
	UINT16 Counter;  //����unsigned short int Counter  
	
	CloseUart0(); 	

	SFRPAGE = 0x0F;	  //120�޸�	 ҳF
	SELECT0L;
	SELECT1H;

    SFRPAGE   = TMR4_PAGE;
	TMR4CN    &=0xFB;	//��ֹ��ʱ��4
							 
	Counter=65536-(SYSCLK/UARTBR19200/16);	
   	RCAP4L=Counter&0x00ff;   // �Ѽ���ֵ�ĵ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4L=RCAP4L;	         //��ֵ����ʱ��4���ֽ� 
	Counter=Counter>>8;
	RCAP4H=Counter&0x00ff;	 //�Ѽ���ֵ�ĸ�8λ������ʱ��4��׽�Ĵ������ֽ� 
	TMR4H=RCAP4H;
 
	TMR4CN    |= 0x04;	   //����ʱ��4 

	OpenUart0();
}


/***********************************************************************
*
* ����ԭ�ͣ�float SelectFreq(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ű���ջ�дƵ���֣��Ͼ����ű���ջ��ã�
*
* ����ʱ�䣺2009-1-4�����
*
* ˵    �����ο�����ű���ջ�˵���ĵ���
*
***********************************************************************/
void SetXinBiaoFreqKC(double freq)
{
	int i = 5;
	while(i--)
	{
		Delay(10000);
		SetXinBiaoFreqNum(freq);
		if(XinBiaoFlagNum == TRUE)
		{
;//			PutXinBiaoS();
		}
		if(XinBiaoRecNormalFlag == TRUE)
		{
			break;
		}	
	}
}


/***********************************************************************
*
* ����ԭ�ͣ�void SetXinBiaoFreqNum(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ű���ջ�дƵ���֣��Ͼ����ű���ջ��ã�
*
* ��    �ģ���2009-3-5��
*
***********************************************************************/
static void SetXinBiaoFreqNum(double Freq)
{
	int FreqT;

	FreqT = (int)Freq;

	Uart0T[0] = '<';
	Uart0T[1] = 'F';
	Uart0T[2] = FreqT / 1000 + '0';
	FreqT %= 1000;
	Uart0T[3] = FreqT / 100 + '0';
	FreqT %= 100;
	Uart0T[4] = FreqT / 10 + '0';
	FreqT %= 10;
	Uart0T[5] = FreqT % 10 + '0';
	Uart0T[6] = '.';

	FreqT = (int)Freq;
	FreqT = (Freq - FreqT) * 1000;

	FreqT %= 1000;
	Uart0T[7] = FreqT / 100 + '0';
	FreqT %= 100;
	Uart0T[8] = FreqT / 10 + '0';
	FreqT %= 10;
	Uart0T[9] = FreqT % 10 + '0';

	Uart0T[10] = 0xd;
	Uart0T[11] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '<';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;
	SelectXinBiao();
	Delay(40);

	SFRPAGE = UART0_PAGE;
	SBUF0 = Uart0T[0];							  	//��ʼ����
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 'T')	  	//�õ��ű���ջ��ķ���ֵ������Uart0R[]��
	{
		if(Uart0TNum > 11)
		{
			 Uart0TxFlag = FALSE;				 	//ֹͣ���ͣ��ط��ͱ�־
			 Uart0Enflag = 0;						//2009/9/8
		}
	}
	CloseUart0();
	CloseTimer0Interrupt();
	TI0 = 0;
	Uart0Enflag = 0;
	Uart0TxFlag = FALSE;
	if(OverflowT0 > 9)
	{
		XinBiaoRecNormalFlag = FALSE;			   	//�ж��ű�Ϊ����
	}
	else
	{
		XinBiaoRecNormalFlag = TRUE;				//�ж��ű�־��Ϊ����		
	}	
}


/***********************************************************************
*
* ����ԭ�ͣ�void GetXinBiaoVER(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ű���ջ�дƵ���֣��Ͼ����ű���ջ��ã�
*
* ��    �ģ���2009-3-5��
*
***********************************************************************/
void GetXinBiaoVER(void)
{
	char i;
	char j;

	Uart0T[0] = '<';
	Uart0T[1] = 'V';
	Uart0T[2] = 'E';
	Uart0T[3] = 'R';
	Uart0T[4] = '?';
	Uart0T[5] = 0xd;
	Uart0T[6] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '<';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;

	SelectXinBiao();
	Delay(40);

	SFRPAGE = UART0_PAGE;
	SBUF0 = Uart0T[0];							  	//��ʼ����
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 'K')	  	//�õ�gradient��ֵ������Uart0R[]��
	{
		i = SBUF0;
		if(i != j)
		{
			if((i == 'F') && (j == 'R'))
			{
				break;
			}
			j = i;
		}
		if(Uart0TNum > 6)
		{
			 Uart0TxFlag = FALSE;				 	//ֹͣ���ͣ��ط��ͱ�־
			 Uart0Enflag = 0;
		}
	}
	CloseUart0();
	CloseTimer0Interrupt();

	Uart0Enflag = 0;
	TI0 = 0;
	Uart0TxFlag = FALSE;
	if(OverflowT0 < 10)
	{
		XinBiaoFlagNum = TRUE;						//�ж��ű�־��Ϊ����		
	}
	else
	{
		XinBiaoFlagNum = FALSE;
	}	
}


/***********************************************************************
*
* ����ԭ�ͣ�void GetXinBiaoVER(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �������������ű���ջ�дƵ���֣��Ͼ����ű���ջ��ã�
*
* ��    �ģ���2009-3-5��
*
***********************************************************************/
void GetXinBiaoVorX(void)
{
	Uart0T[0] = '<';
	Uart0T[1] = 'C';
	Uart0T[2] = 0xd;
	Uart0T[3] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '>';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;
	SelectXinBiao();
	Delay(40);

	SFRPAGE = UART0_PAGE;
	SBUF0 = Uart0T[0];							  	//��ʼ����
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 'V' && SBUF0 != 'X')	  	//�õ�gradient��ֵ������Uart0R[]��
	{
		if(Uart0TNum > 3)
		{
			 Uart0TxFlag = FALSE;				 	//ֹͣ���ͣ��ط��ͱ�־
			 Uart0Enflag = 0;
		}
	}
	XinBiaoVorX = SBUF0;
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag = 0;
	TI0 = 0;
	Uart0TxFlag = FALSE;				
}


/*�Ժ�����ò���*/
/*
static void PutXinBiaoS(void)
{
	Uart0T[0] = '<';
	Uart0T[1] = 'S';
	Uart0T[2] = 0xd;
	Uart0T[3] = 0xa;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '<';
	Uart0TNum = 1;
	Uart0TxFlag = TRUE;
	SelectXinBiao();
	Delay(40);
	SBUF0 = Uart0T[0];							  	//��ʼ����
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 10) && SBUF0 != 0xa)	  	//�õ�gradient��ֵ������Uart0R[]��
	{
		if(Uart0TNum > 3)
		{
			 Uart0TxFlag = FALSE;				 	//ֹͣ���ͣ��ط��ͱ�־
			 Uart0Enflag = 0;
		}
	}
	CloseUart0();
	CloseTimer0Interrupt();
	TI0 = 0;
	Uart0Enflag = 0;
	Uart0TxFlag = FALSE;				
}
*/
/***********************************************************************
*
* ����ԭ�ͣ�float GetKCAGC(void)
*
* ��ڲ�������
*
* ���ڲ�����float agc���Ӵ��ڵõ�AGC
*
* �����������õ��ű���ջ�AGC
*
***********************************************************************/
float GetKCAGC(void)
{
	float agc;

	Uart0Enflag	= 0;
	Uart0RNum 	= 0;	
	Uart0StartF	= '>';
	
	OpenTimer0Interrupt();
	SelectXinBiao();
	while((OverflowT0 < 10) && Uart0RNum < 7)		//�õ�gradient��ֵ������Uart0R[]��
	{
		;
	}
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag = 0;
	if(OverflowT0 > 9 || Uart0RNum < 7)
	{
		return 0.0;
	}
	if(Uart0R[1] == 'D')
	{
		return 0.5;
	}
	agc = (Uart0R[2] - '0') + 		\
		  (Uart0R[4] - '0') * 0.1 + \
		  (Uart0R[5] - '0') * 0.01;
	if(Uart0R[1] == 'T')
	{
		return AGC;	
	}

	return agc;				
}


/***********************************************************************
*
* ����ԭ�ͣ�void ReadGradient(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������򿪴���0�ж�,������б������
*
***********************************************************************/
void ReadGradient(void)
{
	Uart0Enflag		= 0;
	GradientNormal 	= 0;	

	for(Uart0RNum = MAXLENGTH-1; Uart0RNum >= 0; Uart0RNum--)
		Uart0R[Uart0RNum] = '\0';
	Uart0RNum 		= 0;
	Uart0StartF		= '$';							//��б�ǿ�ʼ��־
	OpenTimer0Interrupt();
	SelectGradient();

	while(OverflowT0 < 15 && Uart0RNum < 12)		//�õ�gradient��ֵ������Uart0R[]��
	{
		;
	}
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag	= 0;

/*
	if((Uart0R[3] == '+') || (Uart0R[3] == '-'))
	{
		GradientY = (Uart0R[4] - '0') * 10.0  + \		 
					(Uart0R[5] - '0')		  + \
					(Uart0R[6] - '0') * 0.1   + \
					(Uart0R[7] - '0') * 0.01;
	}
*/
	if((Uart0R[6] == '+') || (Uart0R[6] == '-'))
	{
		GradientY = (Uart0R[7] - '0') * 10.0  + \		 
					(Uart0R[8] - '0')		  + \
					(Uart0R[9] - '0') * 0.1   + \
					(Uart0R[10] - '0') * 0.01;
		if(Uart0R[6] == '-')
			GradientY = -GradientY;
	}
	
	if(Uart0R[0] != '$' 	|| \
		Uart0RNum < 10  	|| \
		OverflowT0 > 14  	|| \
		GradientY > 104.5)
	{
		GradientNormal = 0;
		GradientY = StationEl;
	}
	else
	{
//		if((Uart0R[3] == '+') || (Uart0R[3] == '-'))
//		{
//			if(Uart0R[3] == '-')
//			{
//				GradientY = -GradientY;	
//			}
//		}
//		else if((Uart0R[1] == '+') || (Uart0R[1] == '-'))
//		{
//			if(Uart0R[1] == '-')
//			{
//				GradientY = -GradientY;	
//			}
//		}

		GradientY += 44.5;				//151227  ���ڻ�����б�ǰ�װλ�õĸñ䣬31.6��Ϊ44.5
		GradientY += GradientRightR;
		GradientNormal = 1;
	} 
	return;
}


/***********************************************************************
*
* ����ԭ�ͣ�void ReadGPS(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������򿪴���0�жϣ�����GPS����
*
***********************************************************************/
void ReadGPS(void)
{
	Uart0Enflag = 0;
	Uart0RNum 	= 0;							  //����0���ܵ����ַ������˴���ʼ��Ϊ0
	GPSNormal 	= 0;
	Uart0StartF	= '$';							  //GPS��Ϣͷ��־
	
	OpenTimer0Interrupt();
	SelectGPS();

	SFRPAGE = UART0_PAGE;
	
	while((OverflowT0 < 20) && !(SBUF0 == 0x0a && Uart0RNum > 0))	//��ȡ����0������ʱ��ԼΪ20s��20s��������յ��ַ�0x0a(�س���־)��������Ԫ�ظ�������0��Ҳ��ֱ���˳�ѭ��
	{
		;
	}
	CloseUart0();
	CloseTimer0Interrupt();
	Uart0Enflag	= 0;

	if(Uart0R[42] == 'E')
	{
		GPSEastFlag = EAST;
	}
	if(Uart0R[42 ] == 'W')
	{
		GPSEastFlag = WEST;
	}
	if(Uart0R[29] == 'N')
	{
		GPSNorthFlag = NORTH;
	}
	if(Uart0R[29] == 'S')
	{
		GPSNorthFlag = SOUTH;
	}
	GPSLat = (Uart0R[19] - '0') *   \
		10.0 + (Uart0R[20] - '0') + \
		(Uart0R[21] - '0') * 0.1 +  \
		(Uart0R[22] - '0') * 0.01;
	
	GPSLong = (Uart0R[31] - '0') *   \
		100.0 + (Uart0R[32] - '0') * \
		10.0 + (Uart0R[33] - '0') +  \
		(Uart0R[34] - '0') * 0.1 +   \
		(Uart0R[35] - '0') * 0.01;
	
	if(Uart0R[17] == 'V' && OverflowT0 < 15)
	{
		GPSNormal = 2;
		return;
	}
	if(Uart0R[17] == 'A' && OverflowT0 < 15)
	{
		GPSNormal = 1;
	}
}


/***********************************************************************
*
* ����ԭ�ͣ�void Uart1Init(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������1��ʼ��,��ʱ��4��Ϊʱ��Դ����ֵ�Զ���װ
*
***********************************************************************/
void Uart1Init(void)
{
	EA=0;			      //��ֹ�����ն�  
  	SFRPAGE = TIMER01_PAGE;  

   	CKCON &= 0xEC;	//12��Ƶ
    TMOD |= 0x20;  //T1������ʽ2��8λ�Զ�����
	TMOD &=~0x40; 	//��ʱ������ 
	TH1  = 256-(SYSCLK/12/UARTBR9600/2);	//���ݲ��������ö�ʱ��1����ֵ	  
   	TL1  = TH1;
	
	SFRPAGE = UART1_PAGE;

    SCON1 |= 0x10;	//��ʽ0�������ʿɱ�̵�8 λUART1,����UART1 ��������

	SFRPAGE = TIMER01_PAGE;						
   	TCON |= 0x40;   //��ʱ��1����      
	OpenUart1();
	EA=1;
}

/***********************************************************************
*
* ����ԭ�ͣ�void OpenUart0(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������򿪴���1�ж�
*
***********************************************************************/
static void OpenUart1(void)
{
	SFRPAGE = UART1_PAGE;
	SCON1 &= ~BIT0;
	SCON1 &= ~BIT1;
	EIE2  |=  0x40;									//��������1
}							   	

/***********************************************************************
*
* ����ԭ�ͣ�void CloseUart0(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* �����������رմ���1�ж�
*
***********************************************************************/
static void CloseUart1(void)
{
	EIE2 &= ~0x40;			 //�رմ���1�ж�
}


/***********************************************************************
*
* ����ԭ�ͣ�void Uart1Interrupt(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������1�жϣ�����Uart1SendDataLength���ȵ�Uart1SendData
*
***********************************************************************/
void Uart1Interrupt(void) interrupt UART1INTNUMBER
{
	TEMPAGE = SFRPAGE;		  //120�޸�
	SFRPAGE = UART1_PAGE;	  //ҳ1	  120�޸ġ�

	if((SCON1 & BIT0) == BIT0)
	{
		SCON1 &= ~BIT0;//���RI1
		//--------------------------------------------------------------------------
		if(WireFlag > 0)
		{
			WireBuff[len] = SBUF1;
			len++;
		}
		//--------------------------------------------------------------------------
		else
		{
			if(Uart1ReceDataLength < MAXLENGTH)
			{
				Uart1ReceData[Uart1ReceDataLength] = SBUF1;
				if(Uart1ReceData[Uart1ReceDataLength] == '\r')
				{
					NumberOfComand++;
				}
				Uart1ReceDataLength++;
			}
		}
	}
	if((SCON1&BIT1)==BIT1)
	{
		SCON1&=~BIT1;//���TI1

		if((WireFlag == 1) && (WireBuff[0] != '*') && (WireBuff[len - 1] != 'z'))
		{
			Delay(1825);
			SBUF1 = 'z';
		}
		else if((WireFlag == 2) && (WireBuff[1] != '\n'))
		{
			SBUF1 = '\n';
		}
		else if((WireFlag == 3) && (WireBuff[3] != '\n'))
		{
			SBUF1 = Buff[len1++];
		}
		else if((WireFlag == 4) && (len1 <= len2))
		{
			SBUF1 = WireBuff[len1++];
		}
		else
		{
			Uart1SendDataLength++;
			if(Uart1SendDataLength>=Uart1TXMAXLenth)
			{
				Uart1SendDataLength=0;
				Uart1TXMAXLenth = 0;
			}
			else
			{
				SBUF1 = Uart1SendData[Uart1SendDataLength];			
			}
		}
	}
	SFRPAGE = TEMPAGE;  //120�޸�
}


/***********************************************************************
*
* ����ԭ�ͣ�void Uart1TX(void)
*
* ��ڲ�������
*
* ���ڲ�������
*
* ��������������1����Uart1SendData�е����� ��ע�����Ҫѭ������һ������һ
*
* ��Ҫ�ȴ�Uart1TXMAXLenth	���0
*
***********************************************************************/
void Uart1TX(void)
{
	OpenUart1();
	Uart1SendDataLength = 0;

	SFRPAGE	= UART1_PAGE;
	if(Uart1TXMAXLenth > 0)							//��ǰ�������з�������
	{
		SBUF1 = Uart1SendData[Uart1SendDataLength];	//��Uart1SendDataLength����
	}
	while(Uart1TXMAXLenth != 0)
		;											//�ȴ����ͽ���
}


/*�������������л���غ���*/
void watchwire(void)
{
	int ret;
	char Flag;
	char temp;

	temp = ReadEEPROM(wireaddr);
	if((SWWIPORT == 0) && (temp != '0'))
	{
		WriteEEPROM('0', wireaddr);
		ret = GetG8();
		if(ret == -1)
		{
			InitWire();
			return;
		}
		Flag = GetG8Flag();
		if((Flag & BIT0) != 0)
		{
			Flag &= ~BIT0;
			SetFlag(Flag);
		}
	}
	if((SWWIPORT == BIT4) && (temp != '1'))
	{
		WriteEEPROM('1', wireaddr);
		ret = GetG8();
		if(ret == -1)
		{
			InitWire();
			return;
		}
		Flag = GetG8Flag();
		if((Flag & BIT0) != 1)
		{
			Flag |= BIT0;
			SetFlag(Flag);
		}
	}
	InitWire();
	return;
}

static int GetG8(void)
{
	InitWire();
	WireFlag = 1;
	SFRPAGE	= UART1_PAGE;
	SBUF1 = 'z';
	
	
	OpenTimer0Interrupt();
	while((OverflowT0 < 40) && (WireBuff[len - 1] != '>') && (WireBuff[len - 1] != 'z'))
		;
	CloseTimer0Interrupt();	
	if(OverflowT0 > 38)
	{

		return -1;
	}
	
	for(len; len >= 0; len--)
	{
		WireBuff[len] = '\0';
	}
	
	Delay(60000);
	len = 0;
	WireFlag = 2;
	SBUF1 = '\r';
	while(WireBuff[len - 1] != '>')
		;
	
	for(len; len >= 0; len--)
	{
		WireBuff[len] = '\0';
	}
	Delay(60000);
	len1 = 1;
	len = 0;
	WireFlag = 3;
	Buff[0] = 'G';
	Buff[1] = '8';
	Buff[2] = '\r';
	Buff[3] = '\n';

	SFRPAGE	= UART1_PAGE;
	SBUF1 = 'G';
	while(WireBuff[len - 1] != '>')
		;
	return 0;
}

static char GetG8Flag(void)
{
	char Flag;
	
	//char to int
	if(WireBuff[14] >= '0' && WireBuff[14] <= '9')
	{
		Flag = WireBuff[14] - '0';
	}
	else
	{
		Flag = WireBuff[14] - 'A' + 10;
	}	
	return Flag;
}

static void SetFlag(char TFlag)
{
	char Flag;
	if(TFlag > 9)
	{
		Flag = TFlag - 10 + 'A';
	}
	else
	{
		Flag = TFlag + '0';
	}
	
	len2 = len;
	len = 0;
	len1 = 1;
	WireBuff[14] = Flag;
	WireFlag	 = 4;
	SFRPAGE	= UART1_PAGE;
	SBUF1		 = 'S';
    while(len1 != len2)
		;
	Delay(60000);
	CloseUart1();
	return;
}

static void InitWire(void)
{
	len = 0;
	len1 = 0;
	len2 = 0;
	WireFlag = 0;
	CloseUart1();
	TimeTest(4);

	POWERCLOSE;
	TimeTest(4);
	POWEROPEN;
	Delay(430);

	OpenUart1();

	return;
}
/*
9>G8
:2000001059A100414B4454455354370000000000000000000000000000000000000000008F
:2000201000000000414B445445535437000000000000000000000000000000000000000069
:20004010000000000000000000000000000000000000000000000000000000000000000090
:1E00601000000000000000000000000000000000000000000000000000000000000072
:00000001FF                                                                                                                                                                                                                                                                                                        
*/

