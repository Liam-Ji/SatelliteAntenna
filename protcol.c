//filename:Protcol.c
//author:wangyi,fengjiaji
//date:2008,1,15
//function:define the protcol of the sover and the controller
//���ܣ������˼�غ�����һϵ�е�Э�鴦�������ԴӴ���1�յ���һ�����������д���
//�������������ķ�Ӧ����Ӧ�����ݸ�ʽ�ٷ�����ضˡ�
#include "c8051f120.h"
#include "cpu.h"
#include "public.h"
#include "uart.h"
#include "adc.h"
#include "rweeprom.h"
#include "Protcol.h"
#include "Antenna.h"

#include "sharp.h"
#include "math.h"
#include "timer.h"
#include "station.h"
#include "polar.h"


static void SendRelease(void);				//���ذ汾��
static void ReturnSta(void);				//����վ�㷽λ���������Ϊ����״̬��������״̬��Ϣ�����Ϊ����״̬���ظ���״̬
static void ReturnGPS(void);				//����GPS�ľ�γ����Ϣ
static void ReturnStationStatus(void);		//�������߸���ģ��״̬�����߱���״̬
static void ReturnAZEL(void);				//���ط�λ����
static void ReturnAGCPloar(void);			//����AGC����
static void ReturnSatLongPloarVet(void);	//����Ŀ�����Ǿ��ȼ�������ʽ
static void ReturnRecFreq(void);			//�����ű�Ƶ�ʣ�����sharpƵ��
static void ReturnStaLong(void);
static void ReturnStaLatitude(void);
static void ReturnXinBiaoAGCThreshold(void);
static void ReturnRecKind(void);			//���ؽ��ջ�����
static void ReturnCal(void);

static void SetStaLongPlo(void);	//����Ŀ�����Ǿ��Ⱥͼ���
static void ChangeXinbiaoFreq(void);//�ı��ű�Ƶ��
static void SetThreshold(void);		//��������
static void SetStationLongLat(void);//�����������ڵؾ�γ��
static void SetRecKind(void);		//���ý��ջ�������


static void SetPloRight(void);		//���ü�������
static void SetAZRight(void);		//���÷�λ����
static void SetELRight(void);		//���ø�������


static void DLRight(void);
static void DRRight(void);
static void DURight(void);
static void DAzaRight(void);
static void DAzbRight(void);
static void DSRight(void);
static void TotReturn(void);


static void AddEL(char c);			//���Ӹ���
static void DelEL(char c);			//���ٸ���
static void AddAZ(char c);			//���ӷ�λ
static void DelAZ(char c);			//���ٷ�λ
static void GotoAZ(void);			//ȥ�Զ��巽λ
static void GotoEL(void);			//ȥ�Զ��帩��
static void GotoPolar(void);
static void DelPlo(char c);
static void AddPlo(char c);


static void ReturnWarning(void);	//���ظ澯��Ϣ

static void Reset(void);			//��λ
static void SetAutoMode(void);		//�����Զ�ģʽ
static void SetManulMode(void);		//�����ֶ�ģʽ

static void SendSearching(void);	//��������״̬��Ϣ
static void SendTacking(void);		//��������״̬��Ϣ
static void SendWarning(void);		//���͸���ģ���״̬��Ϣ


static void ElRange(void);		    //2008-11-27���͸����߽�


static void RetBaseStarData(void);				//�� �ο����ǵĲ���
static void ReturnEnBaseStar(void);				//���͵�ǰ�Ƿ��õ��ο�����
static void ReturnBitS(float TSymbolRate);		//����Ŀ�������
static void StoreData(void);					//����ο���������
static void retYorN(void);
static void StorBSarr(UINT8 ch);
static void RetFlag1(void);
static void RetFlag2(void);


static int xdata CKcount1;
static int xdata CKcount2;

static int xdata SelectBaseStar;

char *head=Uart1ReceData;

int	i=0;
/***********************************************************************
����ԭ�ͣ�void watch(void)
��ڲ�������
���ڲ�������
�������������߿���������ʱ�ĶԴ���1�Ľ��շ������ݽ��м�أ�
		  ���������Ҫ������������Ӧ��������ʱһ���������Ҫ���͸��˻��Ի��ˣ�
***********************************************************************/
void watch(void)
{
	//The first char of all commands from the Uart1 is between the char A and C;
	//Judge a command need judgeing the first char ;
	//char *head=Uart1ReceData;//�����ָ����Ϊȫ�ֱ���������ʡʱ��
	int commandlength;//������������
	char temp;
	float Tp;
	Tp = GetT();
	
	if(Tp > 56.0)
	{
		Tempearter = 1;
	}
	if(Tp < 50.0)
	{
		Tempearter = 0;
	}


	temp = ReadEEPROM(wireaddr);

	if((SWWIPORT == 0) && (temp != '0') && (status > SEARCHREADY))
	{
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
		watchwire();
	}
	if((SWWIPORT == BIT4) && (temp != '1') && (status > SEARCHREADY))
	{
		MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
		watchwire();
	}

/*2009/11/3�����ֶ��Զ���λ����*/
	if(MAkit == 0 && status != INIT && status != STORESTATUS)
	{
		
		Delay(20);
		if(MAkit == 0)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			if(status == SEARCHREADY)				//2010/6/30��λʱ�ٴθ�λ��
			{
				AntReadySearchF = 0;		
			}
			OpenTimer0Interrupt();
			while(OverflowT0 < 30 && MAkit == 0)
			{
				if((OverflowT0 & 7) == 0)
				{
					LEDLOCKLOST;
				}
				if((OverflowT0 & 7) == 4)
				{
					LEDREDCOPEN;
				}
			}
			CloseTimer0Interrupt();

			if(OverflowT0 > 29)
			{
				while(MAkit == 0)				//�ȴ������ſ�
				{
					LEDREDCOPEN;
				}
				Reset();						//2009/12/8
			}
			else
			{
				if(status == MANUALMODE)		//����״̬�л�
				{
					status = SEARCHING;
					RetFlag2();
				}
				else
				{
					status = MANUALMODE;
					RetFlag1();
				}				
			}
		}
		LEDLOCKLOST;
	}



	if(ChangeKit == 0 && status != INIT && status != STORESTATUS)
	{
		Delay(20);
		if(ChangeKit == 0)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			if(status == SEARCHREADY)				//2010/6/30��λʱ�ٴθ�λ��
			{
				AntReadySearchF = 0;		
			}
			OpenTimer0Interrupt();
			while(OverflowT0 < 30 && ChangeKit == 0)
			{
				if((OverflowT0 & 7) == 0)
				{
					LEDCLOSE;
				}
				if((OverflowT0 & 7) == 4)
				{
					LEDRED;
				}
			}
			CloseTimer0Interrupt();
			LEDRED;
			if(OverflowT0 > 29)
			{
				status = STORESTATUS;
			}
		}
	}

	if(NumberOfComand>0)
	{
		CKcount1 = 0;
		CKcount2 = 0;
		while(CKcount2 < 20 && \
		 Uart1ReceData[Uart1ReceDataLength-1]!='\r')	 //�ȴ�һ������������
		{
		 	CKcount1++;
			if(CKcount1 > 6000)
			{
				 CKcount1 = 0;
				 CKcount2++;
			}
		}
	}
	//CloseUart1();//Ҫ��Ҫ�ش���1����Ҫ����
	i=0;

	head=Uart1ReceData;
	while(NumberOfComand>0)
	{
		switch (Uart1ReceData[i])
		{
			case 'S':
			{
				NoMonitorF = 0;
				if(Uart1ReceData[i+1]=='t' && Uart1ReceData[i+4]=='\r')//Э��:Sta?<cr>	
					ReturnSta();//����������Ϣ
				else if(Uart1ReceData[i+1]=='e' && Uart1ReceData[i+4]=='\r')//Э��:Set?<cr>
					ReturnSet();//������������
				else if(Uart1ReceData[i+1]=='_' && Uart1ReceData[i+2]=='?')//Э��:S_?<cr>
         			ReturnStationStatus();
				break;
			}
			case 'Q':
			{
				TimeTest(2);  //2008-08-25
				break;
			}
			case 'L':
			{
				if(Uart1ReceData[i+7]=='P' && Uart1ReceData[i+9]=='\r')//Э��:L****P*<cr>
				{
					if(status == SEARCHREADY)
					{
						AntReadySearchF = 0;  //2009/10/12��λ���ٴθ�λ��
					}
					SetStaLongPlo();//����Ŀ�����Ǿ��Ⱥͼ���
				}
				break;
			}
			case 'F':
			{
				if(Uart1ReceData[i+8]=='\r')//Э��:F*******<cr>
					ChangeXinbiaoFreq();
				break;
			}
			case 'H':
			{
				if(Uart1ReceData[i+3]=='\r')//Э��:H**<cr>
					SetThreshold();//��������				
				break;				
			}
			case 'P':
			{
				if(status!=INIT)
				{
					if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='+' && status!=STORESTATUS)//P*+<cr>
						AddEL(Uart1ReceData[1]);//�Ӽ���
					else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='-' && status!=STORESTATUS)//P*-<cr>
						DelEL(Uart1ReceData[1]);//������
					else if(Uart1ReceData[i+10]=='\r' && Uart1ReceData[i+1]=='Z' && Uart1ReceData[i+6]=='V'&& status!=STORESTATUS)//PZ+***V***<cr>����PZ-***V***<cr>
						GotoEL();//��ĳ���ٶ��Զ���Ӽ�����
				}
				break;
			}
			case 'A':
			{
				if(status!=INIT)
				{
					if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='+' && status!=STORESTATUS)//A*+<cr>
						AddAZ(Uart1ReceData[1]);//�Ӽ���
					else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='-'&& status!=STORESTATUS)//A*-<cr>
						DelAZ(Uart1ReceData[i+1]);//������
					else if(Uart1ReceData[i+11]=='\r' && Uart1ReceData[i+1]=='Z' && Uart1ReceData[i+7]=='V' && status!=STORESTATUS)//AZ+****V***<cr>����AZ-****V***<cr>
						GotoAZ();//��ĳ���ٶ��Զ���Ӽ���λ
				}
				break;
				
			}
			case 'C'://C**<cf>
			{
				if(status!=INIT)
				{
					if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='+' && status!=STORESTATUS)
						AddPlo(Uart1ReceData[1]);//�Ӽ��ȼ���
					else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='-'&& status!=STORESTATUS)
						DelPlo(Uart1ReceData[1]);//�����ȼƻ�
					else if(Uart1ReceData[i+1]=='Z' && status!=STORESTATUS)
					//CZ+***V***<cr>����CZ-***V***<cr>
						GotoPolar();
				}
				break;
			}

			case 'D'://Dp****<cf>,Da****<cf>,De****<cf>
			{
				if(Uart1ReceData[i+6]=='\r' && Uart1ReceData[i+1]=='p')
				{
					SetPloRight();//���ü�������
				}
				if(Uart1ReceData[i+6]=='\r' && Uart1ReceData[i+1]=='a')
				{
					SetAZRight();//���÷�λ����
				}
				else if(Uart1ReceData[i+6]=='\r' && Uart1ReceData[i+1]=='e')
				{
					SetELRight();//���ø�������
				}
				else if(Uart1ReceData[i+1]=='L')
				{
					DLRight();
				}
				else if(Uart1ReceData[i+1]=='R')
				{
					DRRight();
				}
				else if(Uart1ReceData[i+1]=='U')
				{
					DURight();
				}
				else if(Uart1ReceData[i+1]=='S')
				{
					DSRight();//2008-11-14
				}
				else if(Uart1ReceData[i+1]=='A' && (Uart1ReceData[i+3]=='a'))
				{
					DAzaRight();
				}
				else if((Uart1ReceData[i+1]=='A') && (Uart1ReceData[i+3]=='b'))
				{
					DAzbRight();	
				}
				break;
			}
			case 'W'://Wa?<cf>
			{
				if(Uart1ReceData[i+3]=='\r')
				{
					ReadGPS(); 
					ReturnWarning();//���ظ澯��Ϣ
				}
				break;
			}
			case 'M':
			{
				if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='R')//M_R<cf>,M_A<cf>,M_M<cf>
					Reset();//��λ
				else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='A')
					SetAutoMode();//�����Զ�ģʽ
				else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='M')
				{
					SetManulMode();//�����ֶ�ģʽ
					if(NumberOfComand > 1)
					{
						Delay(1);
					}
				}
				break; 
			}
			case 'O'://O****T***<cf>
			{
				if(Uart1ReceData[i+13]=='\r' && Uart1ReceData[i+7]=='T')
					SetStationLongLat();//�����������ڵؾ�γ��
				break;
			}
			case 'B': //Ba!<cf>
			{
				if(Uart1ReceData[i+3]=='\r')
				{
					status = STORESTATUS;//�����ʱ״̬�����ղ�״̬�������ó�һ���ղ�״̬
				}
				else
				{
					SrcStarKbS = (Uart1ReceData[i + 1] - '0') * 10000.0 + \
								 (Uart1ReceData[i + 2] - '0') * 1000.0 + \
								 (Uart1ReceData[i + 3] - '0') * 100.0 + \
								 (Uart1ReceData[i + 4] - '0') * 10.0 + \
								  Uart1ReceData[i + 5] - '0';								  
					StoreData();
					SelectBaseStar = 0;
				}				  
				break;
			}
				
			case 'G'://Go!<cf>,�߰갲װ���
			{
				if(Uart1ReceData[i+3]=='\r')
				{
					if(status == INIT)//�����ǰ״̬�ǳ�ʼ��
					{
						BianBanFlag=1;//�߰갲װ��ʼ
					}
					else if(status == STORESTATUS)//�����ǰ״̬���ղ�״̬
					{
						UninstallBianbanFlag=1;//�߰�ж�����
						ReturnStoreSuccess();
					}
					break;
				}	

			}
			case 'g' ://gps?<cf>//����GPS����
			{
			   if(Uart1ReceData[i+4]=='\r')
			   {
					ReadGPS();   		
			   		ReturnGPS();
				}
				break;

			}
			case 'R'://R*<cf>
			{
			   if(Uart1ReceData[i+2]=='\r')
			   		SetRecKind();
				break;

			}

			/*2009/6/11�����µ�Э��*/
			case '$':
			{
				if(Uart1ReceData[i + 1] == '1')
				{	
					TimeTest(2);
				}
				EnStorSrcPara = 1;
				SelectBaseStar = 0;
				if(Uart1ReceData[i + 1] == '2')
				{
					TimeTest(1);
					EnStorSrcPara = 0;
					SelectBaseStar = 1;
					BaseStarFlagThree = 2;
				}
				if(Uart1ReceData[i + 1] == '?')
				{
					RetBaseStarData();	
				}
				break;
			}
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			{
				retYorN();
				break;
			}

			case 'T': //TU,TR,TL�Ȳ�����̨�������ղ�
			{
				if(status > SEARCHREADY && status < STORESTATUS)
				{
					status=MANUALMODE;	
				}
				else
				{
					TotReturn();
					break;					
				}
				if(Uart1ReceData[i + 1] == 'O')
				{
					TotReturn();
					break;	
				}
				TotReturn();

				TestStor(Uart1ReceData[i + 1]);

				break;
			}
			default:
				break;
		}
		NumberOfComand--;//������һ������
		if(NumberOfComand==0)
		{
			Uart1ReceDataLength=0;
			head=Uart1ReceData;
		}
		else
		{		
			while(*(head++)!='\r');//���������Ľ�β
			commandlength=head-(Uart1ReceData+i);
			i=i+commandlength;
		}
	}
	Uart1TX();
}


/***********************************************************************
����ԭ�ͣ�void ReturnSta(void)
��ڲ�������
���ڲ�������
�����������������߷�λ/����/�����Ƕȣ�agcֵ
Э���ʽ��Sta?<cr>
***********************************************************************/
static void ReturnSta(void)
{
	SetAutoMode();
	ReturnRecKind();
	ReturnAZEL();
	ReturnAGCPloar();

	ReturnSatLongPloarVet();
	ReturnRecFreq();	
	ReturnStaLong();

	ReturnStaLatitude();

	ReturnXinBiaoAGCThreshold();
	if(LockFlag==0)//һ�������ɹ�
		SendSearching();//������״̬��Ϣ
	else 
		SendTacking();//��������Ϣ
//	SendRelease();//���ذ汾��2008-12-4

	ReturnBitS(SSrcStarKbS);
}
/***********************************************************************
����ԭ�ͣ�static void ReturnGPS(void)
��ڲ�������
���ڲ�������
��������������GPS�ľ�γ�����
Э���ʽ��Sta?<cr>
***********************************************************************/
static void ReturnGPS(void)
{
	char temp[4];
	itoa(GPSLong*10,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='J';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='Z';
	if(GPSEastFlag==EAST)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else
		Uart1SendData[Uart1TXMAXLenth++]='2';	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	itoa(GPSLat*10,temp,3);
	Uart1SendData[Uart1TXMAXLenth++]='W';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]='Z';
	if(GPSNorthFlag==NORTH)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else
		Uart1SendData[Uart1TXMAXLenth++]='2';	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������

}
/***********************************************************************
����ԭ�ͣ�void ReturnSet(void)
��ڲ�������
���ڲ�������
�����������������Ǿ��ȣ�������ʽ���ű�Ƶ�ʣ�����վ��γ��,���ջ�����
Э���ʽ��Set?<cr>
***********************************************************************/
void ReturnSet(void)
{
	ReturnEnBaseStar();
	ReturnSatLongPloarVet();
	ReturnRecFreq();
	ReturnStaLong();
	ReturnStaLatitude();
	ReturnXinBiaoAGCThreshold();
	ReturnRecKind();
	ElRange();								//2008-11-27���ͱ߽�
	ReturnBitS(SSrcStarKbS);
	SendRelease();							//���ذ汾��2008-12-4
}

/***********************************************************************
����ԭ�ͣ�void SetStaLongPlo(void)
��ڲ�������
���ڲ�������
��������������Ŀ�����Ǿ��Ⱥͼ���
Э���ʽ��L****P*<cr>
***********************************************************************/
static void SetStaLongPlo(void)
{
	float t;
	t=(Uart1ReceData[i+1]-'0')*100+(Uart1ReceData[i+2]-'0')*10+(Uart1ReceData[i+3]-'0')+(Uart1ReceData[i+4]-'0')*0.1; 
	if(SelectBaseStar == 1)
	{
		BaseStarLONG = t;
		if(Uart1ReceData[i+8]=='H')
			BaseStarPol=H;
		else
			BaseStarPol=V;
		if(Uart1ReceData[i+6]=='1')
		{
			BaseStarLONGEF = 1;
		}
		else
		{
			BaseStarLONGEF = 0;
		}
		/****************************************************		
		*���´洢���Ƕ���������,�洢��������
		*�洢0����1
		****************************************************/	
		WriteEEPROM(BaseStarLONGEF, BaseStarLONGEFAddr);
	}
	else
	{
		SatLong = t;
		if(Uart1ReceData[i+8]=='H')
			StationPloMode=H;
		else
			StationPloMode=V;
		if(Uart1ReceData[i+6]=='1')
		{
			SatLongEastFlag = 1;
		}
		else
		{
			SatLongEastFlag = 0;
		}	
	}	

	//status=SearchReady;
}
/***********************************************************************
����ԭ�ͣ�void ChangeXinbiaoFreq(void)
��ڲ�������
���ڲ�������
�����������ı��ű���ջ���Ƶ��
Э���ʽ��F*******<cr>
***********************************************************************/
static void ChangeXinbiaoFreq(void)
{
	double t;
	t = (Uart1ReceData[i+1]-'0')*1000.0+(Uart1ReceData[i+2]-'0')*100.0+(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1+(Uart1ReceData[i+6]-'0')*0.01+(Uart1ReceData[i+7]-'0')*0.001; 
	if(SelectBaseStar == 1)
	{
		BaseStarFreq = t;	
	}
	else
	{
		if(ReceiverKindFlag==XINBIAOREC)
		{
			XinBiaoRecFreq = t;
		}
   		else
		{
			SharpRecFreq = t;
		}
	//status=SearchReady;
	}
}
/***********************************************************************
����ԭ�ͣ�void SetThreshold(void)
��ڲ�������
���ڲ�������
������������������
Э���ʽ��H**<cr>
***********************************************************************/
static void SetThreshold(void)
{
	if(ReceiverKindFlag==XINBIAOREC)
		XinBiaoThreshold=(Uart1ReceData[i+1]-'0')+(Uart1ReceData[i+2]-'0')*0.1;
	else
		SharpThreshold=	(Uart1ReceData[i+1]-'0')+(Uart1ReceData[i+2]-'0')*0.1;
	status=SEARCHREADY;	
}
/***********************************************************************
����ԭ�ͣ�void SetThreshold(void)
��ڲ�������
���ڲ�������
�������������ý��ջ�����
Э���ʽ��R*<cr>
***********************************************************************/
static void SetRecKind(void)
{
	if(SelectBaseStar == 1)
	{
		BaseStarRType = Uart1ReceData[i+1] - '0';	
	}
	else
	{
		if(Uart1ReceData[i+1]=='0')
		{
			ReceiverKindFlag=XINBIAOREC;
		}
		else
		{
			ReceiverKindFlag=SHARPREC;		
		}
	}

   //status=SearchReady;
}
/***********************************************************************
����ԭ�ͣ�void AddEL(char c)
��ڲ������ַ���ȡֵ1��2����3
���ڲ�������
�������������Ӹ���1.0,0.3����0.1
Э���ʽ��P*+<cr>
***********************************************************************/
static void AddEL(char c)
{
	float i;
	switch(c)
	{
		case '1':
			i = 1.0;
			break;
		case '2':
			i = 0.3;
			break;
		case '3':
			i = 0.1;
			break;
		default:
			i = 0.0;
			break;		
	}
	MotorCtrl(UPDOWN, StationEl + i, SpeedSearchEl);
	ReturnAGCPloar();
	ReturnAZEL();
	Uart1TX();
}
/***********************************************************************
����ԭ�ͣ�void DelEL(char c)
��ڲ������ַ���ȡֵ1��2����3
���ڲ�������
�������������ٸ���1.0,0.3����0.1
Э���ʽ��P*-<cr>
***********************************************************************/
static void DelEL(char c)
{
	float i;
	switch(c)
	{
		case '1':
			i = 1.0;
			break;
		case '2':
			i = 0.3;
			break;
		case '3':
			i = 0.1;
			break;
		default:
			i = 0.0;
			break;		
	}

	MotorCtrl(UPDOWN, StationEl - i, SpeedSearchEl);
	ReturnAGCPloar();
	ReturnAZEL();
	Uart1TX();
}
/***********************************************************************
����ԭ�ͣ�void AddAZ(char c)
��ڲ������ַ���ȡֵ1��2����3
���ڲ�������
�������������ӷ�λ1.0,0.3����0.1
Э���ʽ��A*+<cr>
***********************************************************************/
static void AddAZ(char c)
{
	float i;
	switch(c)
	{
		case '1':
			i = 1.0;
			break;
		case '2':
			i = 0.3;
			break;
		case '3':
			i = 0.1;
			break;
		default:
			i = 0.0;
			break;		
	}

	MotorCtrl(RIGHTLEFT, StationAz + i, SpeedSearchAzF);
	ReturnAGCPloar();
	ReturnAZEL();
	Uart1TX();
}
/***********************************************************************
����ԭ�ͣ�void DelAZ(char c)
��ڲ������ַ���ȡֵ1��2����3
���ڲ�������
�������������ٷ�λ1.0,0.3����0.1
Э���ʽ��A*-<cr>
***********************************************************************/
static void DelAZ(char c)
{
	float i;
	switch(c)
	{
		case '1':
			i = 1.0;
			break;
		case '2':
			i = 0.3;
			break;
		case '3':
			i = 0.1;
			break;
		default:
			i = 0.0;
			break;		
	}

	MotorCtrl(RIGHTLEFT, StationAz - i, SpeedSearchAzF);
	ReturnAGCPloar();
	ReturnAZEL();
	Uart1TX();
}

/***********************************************************************
����ԭ�ͣ�void GotoAZ(void)
��ڲ�������
���ڲ�������
������������ĳ���ٶ��Զ���Ӽ���λ
Э���ʽ��AZ+****V***<cr>����AZ-****V***<cr>
***********************************************************************/
static void GotoAZ(void)
{
	float az;
	int speed;
	az=(Uart1ReceData[i+3]-'0')*100+(Uart1ReceData[i+4]-'0')*10+(Uart1ReceData[i+5]-'0')+(Uart1ReceData[i+6]-'0')*0.1;
	speed=(Uart1ReceData[i+8]-'0')*100+(Uart1ReceData[i+9]-'0')*10+(Uart1ReceData[i+10]-'0');

	if(Uart1ReceData[i+2]=='+')
	{ 
		az += StationAz;
	}
	else 
	{ 
		az = StationAz - az;
	}
	MotorCtrl(RIGHTLEFT, az, SpeedInitF - (1000 - speed));
	ReturnAGCPloar();
	ReturnAZEL();
	Uart1TX();
}
/***********************************************************************
����ԭ�ͣ�void GotoEL(void)
��ڲ�������
���ڲ�������
������������ĳ���ٶ��Զ���Ӽ�����
Э���ʽ��PZ+***V***<cr>����PZ-***V***<cr>
***********************************************************************/
static void GotoEL(void)
{
	float el;
	int speed;
	el=(Uart1ReceData[i+3]-'0')*10+(Uart1ReceData[i+4]-'0')+(Uart1ReceData[i+5]-'0')*0.1;
	speed=(Uart1ReceData[i+7]-'0')*100+(Uart1ReceData[i+8]-'0')*10+(Uart1ReceData[i+9]-'0');
	
	if(Uart1ReceData[i+2]=='+')
	{ 
		el += StationEl;
	}
	else 
	{ 
		el = StationEl - el;
	}

	MotorCtrl(UPDOWN, el, SpeedInitS - (1000 - speed));
	ReturnAGCPloar();
	ReturnAZEL();
	Uart1TX();
}


 /***********************************************************************
����ԭ�ͣ�void SetPloRight(void)
��ڲ�������
���ڲ�������
�������������ü�������
Э���ʽ��
***********************************************************************/
static void SetPloRight(void)
{
    float RightPol;					//��Ϊ���õ���ȷ������
	float StoreDeltaPol;			//�����������Ҫ�洢�Ĳ�ֵ
	float ReadDeltaPol = 0.0;		//�������Ĳ�ֵ
	float newpol = 0.0;

	newpol = GetComPolA();
	if(ReadEEPROM(PolCompensateAddress)=='+')		//���������������ֵ
	{
		ReadDeltaPol=ReadEEPROM(PolCompensateAddress+1);
	}
	else if(ReadEEPROM(PolCompensateAddress)=='-')	//������������Ǹ�ֵ
	{
		ReadDeltaPol=0.0-ReadEEPROM(PolCompensateAddress+1);
	}

	RightPol=(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1;
   	if(Uart1ReceData[i+2]=='1')
		RightPol=0.0-RightPol;	

	StoreDeltaPol=ReadDeltaPol+(RightPol-newpol);//�����ȷ�����ֵ
	AngleCom=StoreDeltaPol;
	if(StoreDeltaPol>0)//����洢�Ĳ�ֵΪ��
	{
		WriteEEPROM('+',PolCompensateAddress);//������
		WriteEEPROM(StoreDeltaPol,PolCompensateAddress+1);
	}
	else
	{
		WriteEEPROM('-',PolCompensateAddress);//�渺��
	   	StoreDeltaPol=0-StoreDeltaPol;//�������
		WriteEEPROM(StoreDeltaPol,PolCompensateAddress+1);
	}
	PolarFlag = 0;
	ReturnAZEL();
	ReturnAGCPloar();
	Uart1TX();
}


/***********************************************************************
����ԭ�ͣ�void SetAZRight(void)
��ڲ�������
���ڲ�������
�������������÷�λ����
Э���ʽ��
***********************************************************************/
static void SetAZRight(void)
{
	float RightAz;				//��Ϊ���õ���ȷ��λ
	float StoreDeltaAz;			//�����������Ҫ�洢�Ĳ�ֵ
	float ReadDeltaAz = 0.0;	//�������Ĳ�ֵ

	if(ReadEEPROM(AZCompensateAddress)=='+')		//���������������ֵ
	{
		ReadDeltaAz=	ReadEEPROM(AZCompensateAddress+1)/10.0;
	}
	else if(ReadEEPROM(AZCompensateAddress)=='-')	//������������Ǹ�ֵ
	{
		ReadDeltaAz=0.0-ReadEEPROM(AZCompensateAddress+1)/10.0;
	}
	RightAz=(Uart1ReceData[i+2]-'0')*100.0+(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1;

	StoreDeltaAz=ReadDeltaAz+(RightAz-StationAz);//�����ȷ�����ֵ
	if(StoreDeltaAz>0)//����洢�Ĳ�ֵΪ��
	{
		WriteEEPROM('+',AZCompensateAddress);//������
		WriteEEPROM(10*StoreDeltaAz,AZCompensateAddress+1);
	}
	else
	{
		WriteEEPROM('-',AZCompensateAddress);//�渺��
	   	StoreDeltaAz=0-StoreDeltaAz;//�������
		WriteEEPROM(10*StoreDeltaAz,AZCompensateAddress+1);
	}
	StationAz=RightAz;
	ReturnAZEL();
	ReturnAGCPloar();
	Uart1TX();	
}
/***********************************************************************
����ԭ�ͣ�void SetELRight(void)
��ڲ�������
���ڲ�������
�������������ø�������
Э���ʽ��
***********************************************************************/
static void SetELRight(void)
{
	float RightEl;				//��Ϊ���õ���ȷ������
	float StoreDeltaEl;			//�����������Ҫ�洢�Ĳ�ֵ
	float ReadDeltaEl = 0.0;	//�������Ĳ�ֵ

	if(ReadEEPROM(ELCompensateAddress)=='+')		//���������������ֵ
	{
		ReadDeltaEl=ReadEEPROM(ELCompensateAddress+1)/10.0;
	}
	else if(ReadEEPROM(ELCompensateAddress)=='-')	//������������Ǹ�ֵ
	{
		ReadDeltaEl=0.0-ReadEEPROM(ELCompensateAddress+1)/10.0;
	}
	RightEl=(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1;
	StoreDeltaEl=ReadDeltaEl+(RightEl-StationEl);//�����ȷ�����ֵ
	GradientRightR = StoreDeltaEl;
	if(StoreDeltaEl>0)//����洢�Ĳ�ֵΪ��
	{
		WriteEEPROM('+',ELCompensateAddress);//������
		WriteEEPROM(10*StoreDeltaEl,ELCompensateAddress+1);
	}
	else
	{
		WriteEEPROM('-',ELCompensateAddress);//�渺��
	   	StoreDeltaEl=0-StoreDeltaEl;//�������
		WriteEEPROM(10*StoreDeltaEl,ELCompensateAddress+1);
	}
	StationEl=RightEl;
	ReturnAZEL();
	ReturnAGCPloar();
	Uart1TX();
}
/***********************************************************************
����ԭ�ͣ�void ReturnWarning()
��ڲ�������
���ڲ�������
���������������б�ǣ�GPS��������������״̬��Ϣ
***********************************************************************/
static void ReturnWarning()//���ظ澯��Ϣ
{
//	PloarNormal=0;//���ü���������
//	StationPol=GetComPolA();
//	if(StationPol<95.0 && StationPol>-95.0)
//		PloarNormal=1;//ֱ�Ӳ�����������������򼫻���Ϊ����
	SendWarning();

}
/***********************************************************************
����ԭ�ͣ�static void ReturnRecKind(void)
��ڲ�������
���ڲ�������
�������������ؽ��ջ�����
***********************************************************************/
static void ReturnRecKind(void)//���ؽ��ջ�����
{	
	Uart1SendData[Uart1TXMAXLenth++]='R';
	if(ReceiverKindFlag==XINBIAOREC)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else
		Uart1SendData[Uart1TXMAXLenth++]='1';	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
}
/***********************************************************************
����ԭ�ͣ�static void Reset()
��ڲ�������
���ڲ�������
����������׼�����¿�ʼ����
***********************************************************************/
static void Reset()//��λ
{
	ResetFlag = 1;				//�յ�һ�θ�λ����󣬱�־����
	if(status == SEARCHREADY)
	{
		AntReadySearchF = 0;		//2009/10/12��λʱ�ٴθ�λ��
	}
	status = SEARCHREADY;
}
/***********************************************************************
����ԭ�ͣ�void SetAutoMode()
��ڲ�������
���ڲ�������
���������������Զ�ģʽ
***********************************************************************/
static void SetAutoMode()
{
	if(status > SEARCHREADY && status < STORESTATUS)
	{
		status=SEARCHING;	
	}
}
/***********************************************************************
����ԭ�ͣ�void SetManulMode()
��ڲ�������
���ڲ�������
���������������ֶ�ģʽ
***********************************************************************/
static void SetManulMode()
{
	ReturnAZEL();
	ReturnAGCPloar();
	ReturnCal();
	SendRelease();
	if(status > INIT && status < STORESTATUS)
	{
		status=MANUALMODE;	
	}
}

/***********************************************************************
����ԭ�ͣ�void SetStationLongLat()
��ڲ�������
���ڲ�������
�������������߿������������õ���վ��γ��
***********************************************************************/
static void SetStationLongLat()//�����������ڵؾ�γ��
{
	StationLong=(Uart1ReceData[i+1]-'0')*100.0+(Uart1ReceData[i+2]-'0')*10.0+(Uart1ReceData[i+3]-'0')*1.0+(Uart1ReceData[i+4]-'0')*0.1;
	StationLat=(Uart1ReceData[i+8]-'0')*10.0+(Uart1ReceData[i+9]-'0')*1.0+(Uart1ReceData[i+10]-'0')*0.1;
	if(Uart1ReceData[i+6] == '1')
	{
		StationEastFlag = 1;
	}
	else
	{
		StationEastFlag = 0;
	}
	if(Uart1ReceData[i+12] == '1')
	{
		StationNorthFlag = 1;
	}
	else
	{
		StationNorthFlag = 0;
	}
	ReturnAGCPloar();
	ReturnAZEL();
	status=SEARCHREADY;
}
/***********************************************************************
����ԭ�ͣ�static void ReturnAZEL(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰ���ߵķ�λ�͸�����
Э���ʽ��A****E***<LF>
***********************************************************************/
static void ReturnAZEL(void)
{
	char temp[4];
	itoa(StationAz*10,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='A';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='E';
	itoa(StationEl*10,temp,3);
	if(StationEl>0.0)
		Uart1SendData[Uart1TXMAXLenth++]='+';
	else
		Uart1SendData[Uart1TXMAXLenth++]='-';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];	
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	ElRange();								//2008-11-27���ͱ߽�	
}
/***********************************************************************
����ԭ�ͣ�static void ReturnAGCPloar(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰ����AGC�ͼ����Ƕȣ�
Э���ʽ��G****R****<LF>
***********************************************************************/
static void ReturnAGCPloar(void)
{
	char temp[4];
	AGC=GetAGC();//���ﲻӦ���ٲ���
	itoa(AGC*100,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='G';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='R';
	StationPol=GetComPolA();//����Ƿ�ҲҪ��ת�����ٲ�
	if(StationPol < 90.0)
	{
		Uart1SendData[Uart1TXMAXLenth++]='-';
	}
	else
	{ 
		Uart1SendData[Uart1TXMAXLenth++]='+';
		StationPol -= 360;
	}
	itoa(fabs(StationPol) * 10, temp, 3);
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
}


/***********************************************************************
����ԭ�ͣ�static void ReturnSatLongPloarVet(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰ���õ����Ǿ��Ⱥͼ�����ʽ
Э���ʽ��L****Z*P*<LF>
***********************************************************************/
static void ReturnSatLongPloarVet(void)
{
	char temp[4];
	itoa(SatLong*10,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='L';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='Z';
	if(SatLongEastFlag == 1)
	{
		Uart1SendData[Uart1TXMAXLenth++]='1';	
	}
	else
	{
		Uart1SendData[Uart1TXMAXLenth++]='2';
	}
	Uart1SendData[Uart1TXMAXLenth++]='P';
	if(StationPloMode==H)
		Uart1SendData[Uart1TXMAXLenth++]='H';
	else
		Uart1SendData[Uart1TXMAXLenth++]='V';		
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������

}

/***********************************************************************
����ԭ�ͣ�static void ReturnRecFreq(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰ���õ��ű�Ƶ�ʻ������ս��ջ�Ƶ��
Э���ʽ��F*******<LF>
***********************************************************************/
static void ReturnRecFreq(void)
{
	char temp[4],temp1[3];
	double FreqInt;//����
	double FreqDec;//С�� 	
	if(ReceiverKindFlag==XINBIAOREC)
	{
		FreqDec=modf(XinBiaoRecFreq,&FreqInt);
		itoa(FreqInt,temp,4);
		itoa(FreqDec*1000,temp1,3);
		Uart1SendData[Uart1TXMAXLenth++]='F';
		Uart1SendData[Uart1TXMAXLenth++]=temp[0];
		Uart1SendData[Uart1TXMAXLenth++]=temp[1];
		Uart1SendData[Uart1TXMAXLenth++]=temp[2];
		Uart1SendData[Uart1TXMAXLenth++]=temp[3];
		Uart1SendData[Uart1TXMAXLenth++]=temp1[0];
		Uart1SendData[Uart1TXMAXLenth++]=temp1[1];
		Uart1SendData[Uart1TXMAXLenth++]=temp1[2];
		Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	}
	else//sharp���ջ�����ϴ�����ֻ��ȷ��С�����һλ
	{
		//itoa((long int)(SharpRecFreq*10),temp,4);
		FreqDec=modf(SharpRecFreq,&FreqInt);
		itoa(FreqInt,temp,4);
		itoa(FreqDec*1000,temp1,3);
		Uart1SendData[Uart1TXMAXLenth++]='F';
		Uart1SendData[Uart1TXMAXLenth++]=temp[0];
		Uart1SendData[Uart1TXMAXLenth++]=temp[1];
		Uart1SendData[Uart1TXMAXLenth++]=temp[2];
		Uart1SendData[Uart1TXMAXLenth++]=temp[3];
		Uart1SendData[Uart1TXMAXLenth++]=temp1[0];
		Uart1SendData[Uart1TXMAXLenth++]=temp1[1];
		Uart1SendData[Uart1TXMAXLenth++]=temp1[2];
		Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	}
}

/***********************************************************************
����ԭ�ͣ�static void ReturnStaLong(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰվ��ľ���(������������Ϣ)
Э���ʽ��J****Z*<LF>
***********************************************************************/
static void ReturnStaLong(void)
{
	char temp[4];
	itoa(StationLong*10,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='J';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='Z';
	if(StationEastFlag==EAST)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else
		Uart1SendData[Uart1TXMAXLenth++]='2';	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������

}
/***********************************************************************
����ԭ�ͣ�static void ReturnStaLatitude(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰվ���γ�ȣ������ϱ�γ��Ϣ��
Э���ʽ��W****Z*<LF>
***********************************************************************/
static void ReturnStaLatitude(void)
{
	char temp[4];
	itoa(StationLat*10,temp,3);
	Uart1SendData[Uart1TXMAXLenth++]='W';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]='Z';
	if(StationNorthFlag==NORTH)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else
		Uart1SendData[Uart1TXMAXLenth++]='2';	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������

}
/***********************************************************************
����ԭ�ͣ�static void ReturnAGCThreshold(void)
��ڲ�������
���ڲ�������
�������������߿��������ط���������Ŀǰ�ű���ջ����õ�AGC��������ֵ
Э���ʽ��H**<LF>
***********************************************************************/
static void ReturnXinBiaoAGCThreshold(void)
{
	char temp[2];
	itoa(Threshold*10,temp,2);
	Uart1SendData[Uart1TXMAXLenth++]='H';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������

}


/***********************************************************************
����ԭ�ͣ�static void SendWarning(void)
��ڲ�������
���ڲ�������
�������������߿��������ط������������߸澯��Ϣ
Э���ʽ��j*q*g*l*p*<LF>
***********************************************************************/
static void SendWarning(void)
{
	
	Uart1SendData[Uart1TXMAXLenth++]='j';
	if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';//�ű���ջ������ս��ջ�������
	else if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';//�ű���ջ����ϣ����ս��ջ�����
	else if(XinBiaoRecNormalFlag==1 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='2';//�ű���ջ����������ս��ջ�����
	else
		Uart1SendData[Uart1TXMAXLenth++]='3';//�ű���ջ����������ս��ջ�����

	Uart1SendData[Uart1TXMAXLenth++]='q';
	if(GradientNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GradientNormal==1) 
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//��б��Ŀǰ������״̬

	Uart1SendData[Uart1TXMAXLenth++]='g';
	if(GPSNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GPSNormal==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//GPSĿǰ������״̬

	Uart1SendData[Uart1TXMAXLenth++]='l';
	Uart1SendData[Uart1TXMAXLenth++]='0';

	Uart1SendData[Uart1TXMAXLenth++]='p';
	if(PloarNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='1';//����Ŀǰֻ������״̬


	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	Uart1TX();
}

/***********************************************************************
����ԭ�ͣ�static void SendSearching(void)
��ڲ�������
���ڲ�������
�������������߿��������ط�������������״̬����
Э���ʽ��X!
***********************************************************************/
static void SendSearching(void)
{

	Uart1SendData[Uart1TXMAXLenth++]='X';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
}

/***********************************************************************
����ԭ�ͣ�static void SendTacking(void)
��ڲ�������
���ڲ�������
�������������߿��������ط�������������״̬����
Э���ʽ��Y!
***********************************************************************/
static void SendTacking(void)
{

	Uart1SendData[Uart1TXMAXLenth++]='Y';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
}

/***********************************************************************
����ԭ�ͣ�static void ReturnStationStatus(void)
��ڲ�������
���ڲ�������
�������������߿��������ط��������͸澯��״̬��Ϣ
Э���ʽ��j*q*g*l*p*S_*<LF>
***********************************************************************/
void ReturnStationStatus(void)
{
	
	Uart1SendData[Uart1TXMAXLenth++]='j';
	if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';//�ű���ջ������ս��ջ�������
	else if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';//�ű���ջ����ϣ����ս��ջ�����
	else if(XinBiaoRecNormalFlag==1 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='2';//�ű���ջ����������ս��ջ�����
	else
		Uart1SendData[Uart1TXMAXLenth++]='3';//�ű���ջ����������ս��ջ�����

	Uart1SendData[Uart1TXMAXLenth++]='q';
	if(GradientNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GradientNormal==1) 
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//��б��Ŀǰ������״̬

	Uart1SendData[Uart1TXMAXLenth++]='g';
	if(GPSNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GPSNormal==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//GPSĿǰ������״̬

	Uart1SendData[Uart1TXMAXLenth++]='l';
	Uart1SendData[Uart1TXMAXLenth++]='0';

	Uart1SendData[Uart1TXMAXLenth++]='p';
	if(PloarNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='1';//����Ŀǰֻ������״̬

	Uart1SendData[Uart1TXMAXLenth++]='S';
	Uart1SendData[Uart1TXMAXLenth++]='_';
	if(status==INIT)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(status==MANUALMODE)
		Uart1SendData[Uart1TXMAXLenth++]='2';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='1';
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	SendRelease();//���ذ汾��2008-12-4
	Delay(1000);//2008-09-02
	Uart1TX();
} 


/*----------------------eric.Yang  ---------------------------------*/
static void DLRight(void)
{
	unsigned char temp;
	temp = (Uart1ReceData[i+2]-'0') * 10 + (Uart1ReceData[i + 3] - '0');
	WriteEEPROM(temp, rightONE);
	return;	
}

static void DRRight(void)
{
	unsigned char temp;
	temp = (Uart1ReceData[i+2]-'0') * 10 + (Uart1ReceData[i + 3] - '0');
	WriteEEPROM(temp, rightTWO);
	return;
}

static void DURight(void)
{
	unsigned char temp;
	temp = (Uart1ReceData[i + 3]-'0') * 100 + (Uart1ReceData[i + 4] - '0') * 10 + (Uart1ReceData[i + 5] - '0');
	WriteEEPROM(temp, rightTHREE);
	return;
}

static void DAzaRight(void)
{
	unsigned char temp;
	temp = (Uart1ReceData[i+5]-'0') * 100 + (Uart1ReceData[i + 6] - '0') * 10 + (Uart1ReceData[i + 7] - '0');
	WriteEEPROM(temp, rightFOUR);
	return;
}

static void DAzbRight(void)
{
	unsigned char temp;
	temp = (Uart1ReceData[i+5]-'0') * 100 + (Uart1ReceData[i + 6] - '0') * 10 + (Uart1ReceData[i + 7] - '0');
	WriteEEPROM(temp, rightFIVE);
	return;
}


/*2008-11-14��������У��*/
static void DSRight(void)
{
	unsigned char temp;
	temp = (Uart1ReceData[i+2]-'0') * 100 + (Uart1ReceData[i + 3] - '0') * 10 + (Uart1ReceData[i + 4] - '0');
	WriteEEPROM(temp, rightSIX);
	return;	
}


static void TotReturn(void)
{
	char temp[4];
	int itemp;

	Uart1SendData[Uart1TXMAXLenth++]='T';
 	Uart1SendData[Uart1TXMAXLenth++]='O';

	itemp = (int)ReadEEPROM(rightONE);
	if(itemp > 200)
	{
		itemp = 20;	
	}
	itoa(itemp, temp,2);
	Uart1SendData[Uart1TXMAXLenth++]='D';
 	Uart1SendData[Uart1TXMAXLenth++]='L';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];

	itemp = (int)ReadEEPROM(rightTWO);
	if(itemp > 200)
	{
		itemp = 20;	
	}
	itoa(itemp, temp,2);
	Uart1SendData[Uart1TXMAXLenth++]='D';
 	Uart1SendData[Uart1TXMAXLenth++]='R';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];

	itemp = (int)ReadEEPROM(rightTHREE);
	if(itemp > 200)
	{
		itemp = 100;	
	}
	itoa(itemp, temp,3);
	Uart1SendData[Uart1TXMAXLenth++]='D';
 	Uart1SendData[Uart1TXMAXLenth++]='U';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];

	itemp = (int)ReadEEPROM(rightSIX);
	if(itemp > 200)
	{
		itemp = 60;	
	}
	itoa(itemp, temp,3);
	Uart1SendData[Uart1TXMAXLenth++]='D';
 	Uart1SendData[Uart1TXMAXLenth++]='S';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];

	itemp = (int)ReadEEPROM(rightFOUR);
	if(itemp > 200)
	{
		itemp = 100;	
	}
	itoa(itemp, temp,3);
	Uart1SendData[Uart1TXMAXLenth++]='D';
 	Uart1SendData[Uart1TXMAXLenth++]='A';
 	Uart1SendData[Uart1TXMAXLenth++]='z';
	Uart1SendData[Uart1TXMAXLenth++]='a';
	Uart1SendData[Uart1TXMAXLenth++]='0';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];

	itemp = (int)ReadEEPROM(rightFIVE);
	if(itemp > 200)
	{
		itemp = 100;	
	}
	itoa(itemp, temp,3);
	Uart1SendData[Uart1TXMAXLenth++]='D';
 	Uart1SendData[Uart1TXMAXLenth++]='A';
 	Uart1SendData[Uart1TXMAXLenth++]='z';
	Uart1SendData[Uart1TXMAXLenth++]='b';
	Uart1SendData[Uart1TXMAXLenth++]='0';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
			
	Uart1SendData[Uart1TXMAXLenth++]='\n';//������
	Uart1TX();
	return;
}


/***********************************************************************
����ԭ�ͣ�void ReturnCal(void)
��ڲ�������
���ڲ�������
�������������߷������ۼ���ֵ
***********************************************************************/
static void ReturnCal(void)
{
	UINT8 TempF;
	char temp[4];
	
	TempF = SatLongEastFlagS;
	SatLongEastFlagS = SatLongEastFlag;
	FindAnt(StationLong, StationLat, SatLong, StationPloMode);
  	SatLongEastFlagS = TempF;

	itoa(StationAzCal*10,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='a';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='e';
	itoa(StationElCal*10,temp,3);
	if(StationEl>0.0)
		Uart1SendData[Uart1TXMAXLenth++]='+';
	else
		Uart1SendData[Uart1TXMAXLenth++]='-';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];	
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];	
	Uart1SendData[Uart1TXMAXLenth++]='\n';
	Uart1SendData[Uart1TXMAXLenth++]='r';

	if(StationPol < 90.0)
		Uart1SendData[Uart1TXMAXLenth++]='-';
	else{ 
		Uart1SendData[Uart1TXMAXLenth++]='+';
		StationPol -= 360;
	}

	itoa((StationPolCal - 90.0)*10,temp,3);
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]='\n';
}


/*����EL��Χ*/
static void ElRange(void)
{
	char temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='P';
	Uart1SendData[Uart1TXMAXLenth++]='i';

	itoa(ELDOWNlimit * 10, temp, 3);
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];

	itoa(ELUPlimit * 10, temp, 3);
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]='\n';
	return;
}

/***********************************************************************
����ԭ�ͣ�static void ReturnResetSuccess(void)
��ڲ�������
���ڲ�������
�������������߿��������ط������������߸�λ���
Э���ʽ��Rs!
***********************************************************************/
void ReturnResetSuccess(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='R';
	Uart1SendData[Uart1TXMAXLenth++]='s';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1SendData[Uart1TXMAXLenth++]='\n';
	Uart1TX();
}

/***********************************************************************
����ԭ�ͣ�void SendSJ(void)
��ڲ�������
���ڲ�������
�������������߿��������ط��������͹ػ�ж�ر߰���Ϣ
Э���ʽ��SJ!
***********************************************************************/
void SendSJ(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='S';
	Uart1SendData[Uart1TXMAXLenth++]='J';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1TX();
}

/*�汾�� AKD10K01*/
static void SendRelease(void)
{
    UINT8 temp;
	Uart1SendData[Uart1TXMAXLenth++]='A';
	Uart1SendData[Uart1TXMAXLenth++]='K';
	Uart1SendData[Uart1TXMAXLenth++]='D';
	temp = 0;
	if(AZLeftLimitF == 1)
	{
		temp += 1; 
	}
	if(AzMidLimitF == 1)
	{
		temp += 2; 
	}
	if(AZRightLimitF == 1)
	{
		temp += 4; 
	}

	Uart1SendData[Uart1TXMAXLenth++]='0' + temp;
	Uart1SendData[Uart1TXMAXLenth++]='0' + XinBiaoFlagNum;//beacon version:1:KuanChao;0:KenLi--kitty add on 2011/01/18 for getting beacon version;
	Uart1SendData[Uart1TXMAXLenth++]='P';
	Uart1SendData[Uart1TXMAXLenth++]='0';
	Uart1SendData[Uart1TXMAXLenth++]='2';
	Uart1SendData[Uart1TXMAXLenth++]='\n';
}



/***********************************************************************
����ԭ�ͣ�void AddPlo(char c)
��ڲ������ַ�'1','2','3'
���ڲ�������
�������������Ӽ�����1��
Э���ʽ��
***********************************************************************/
static void AddPlo(char c)
{
	StationPol = GetComPolA();
	StationPol += (float)(c - '0');
	GotoPolarAngle(StationPol);
	ReturnAGCPloar();
	Uart1TX();
}
/***********************************************************************
����ԭ�ͣ�void AddPlo(char c)
��ڲ������ַ�'1','2','3'
���ڲ�������
�������������ټ�����1��
Э���ʽ��
***********************************************************************/
static void DelPlo(char c)
{
	StationPol = GetComPolA();
	StationPol -= (float)(c - '0');
	GotoPolarAngle(StationPol);
	ReturnAGCPloar();
	Uart1TX();
}
 /***********************************************************************
����ԭ�ͣ�void GotoPolar(void)
��ڲ�������
���ڲ�������
�����������ߵ�Ŀ�꼫����
Э���ʽ��
***********************************************************************/
static void GotoPolar(void)
{
	StationPol = GetComPolA();
	if(Uart1ReceData[i+2]=='+')
	{
		StationPol += (Uart1ReceData[i+3]-'0')*10.0 + \
					  (Uart1ReceData[i+4]-'0') + \
					  (Uart1ReceData[i+5]-'0') * 0.1;		
	}
	else 
	{
		StationPol -= (Uart1ReceData[i+3]-'0')*10.0 + \
					  (Uart1ReceData[i+4]-'0') + \
					  (Uart1ReceData[i+5]-'0') * 0.1;				
	}
	GotoPolarAngle(StationPol);
	ReturnAGCPloar();
	Uart1TX();
}

/***********************************************************************
����ԭ�ͣ�void SendKJ(void)
��ڲ�������
���ڲ�������
�������������߿��������ط��������Ϳ���װ�ر߰���Ϣ
Э���ʽ��KJ!
***********************************************************************/
void SendKJ(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='K';
	Uart1SendData[Uart1TXMAXLenth++]='J';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1TX();
}

void ReturnStoreSuccess(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='P';
	Uart1SendData[Uart1TXMAXLenth++]='o';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1TX();
}

/*2009/6/12�������º���*/
static void RetBaseStarData(void)
{
	char temp[4],temp1[3];
	double FreqInt;//����
	double FreqDec;
	float TSatLong;
	UINT8 TStationPloMode, TEFlag;
		
	Uart1SendData[Uart1TXMAXLenth++]='$';	  //
	Uart1SendData[Uart1TXMAXLenth++]='2';

	Uart1SendData[Uart1TXMAXLenth++]='\n';
	TSatLong = SatLong;
	SatLong = BaseStarLONG;					 //
	TStationPloMode = StationPloMode;
	StationPloMode = BaseStarPol;
	TEFlag = SatLongEastFlag;
	SatLongEastFlag = BaseStarLONGEF;
	ReturnSatLongPloarVet();
	SatLongEastFlag = TEFlag;				 //
	StationPloMode = TStationPloMode;
	SatLong = TSatLong;

	FreqDec=modf(BaseStarFreq,&FreqInt);
	itoa(FreqInt,temp,4);
	itoa(FreqDec*1000,temp1,3);
	Uart1SendData[Uart1TXMAXLenth++]='F';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]=temp1[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp1[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp1[2];
	Uart1SendData[Uart1TXMAXLenth++]='\n';

	Uart1SendData[Uart1TXMAXLenth++] = 'R';
	if(BaseStarRType == XINBIAOREC)
	{
		Uart1SendData[Uart1TXMAXLenth++]= '0';	
	}
	else
	{
		Uart1SendData[Uart1TXMAXLenth++]= '1';
	}
	Uart1SendData[Uart1TXMAXLenth++]='\n';

	ReturnBitS(BaseStarKbS);
	return;
}


static void ReturnEnBaseStar(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='$';
	if(EnBaseStar == 1)
	{
		Uart1SendData[Uart1TXMAXLenth++] = '1';
	}
	else
	{
		Uart1SendData[Uart1TXMAXLenth++] = '0';
	}
	Uart1SendData[Uart1TXMAXLenth++]='\n';
		
}

static void ReturnBitS(float TSymbolRate)
{
	UINT16 TempKbS;
	if(TSymbolRate > 50000.0)
	{
		TSymbolRate = 50000.0;
	}
	TempKbS = (UINT16)TSymbolRate;
	Uart1SendData[Uart1TXMAXLenth++] = 'B';
	Uart1SendData[Uart1TXMAXLenth++] = TempKbS / 10000 + '0';
	TempKbS %= 10000;
	Uart1SendData[Uart1TXMAXLenth++] = TempKbS / 1000 + '0';
	TempKbS %= 1000;
	Uart1SendData[Uart1TXMAXLenth++] = TempKbS / 100 + '0';
	TempKbS %= 100;
	Uart1SendData[Uart1TXMAXLenth++] = TempKbS / 10 + '0';
	TempKbS %= 10;
	Uart1SendData[Uart1TXMAXLenth++] = TempKbS + '0';
	Uart1SendData[Uart1TXMAXLenth++] = '\n';
	Delay(200);
	Uart1TX();		
}

static void StoreData(void)
{
	UINT16 TempSrcStarKbS;
	UINT8  TempAddr;
	int a, b, c, d, i, j;

	if(SrcStarKbS - 50000.0 > 1.0)
	{
		SrcStarKbS = 50000.0;	
	}
	if(SelectBaseStar == 1)
	{
		BaseStarKbS = SrcStarKbS;
		TempAddr = BaseStarKbSAddr;
	}
	else
	{
		SSrcStarKbS = SrcStarKbS;
		TempAddr = SSrcStarKbSAddr;
	}

	TempSrcStarKbS = (UINT16)SrcStarKbS;
	WriteEEPROM((UINT8)(TempSrcStarKbS / 10000), TempAddr);
	TempSrcStarKbS %= 10000;
	WriteEEPROM((UINT8)(TempSrcStarKbS / 100), TempAddr + 1);
	TempSrcStarKbS %= 100;
	WriteEEPROM((UINT8)TempSrcStarKbS, TempAddr + 2);

	if(SelectBaseStar == 1)
	{
		a = (int)(BaseStarLONG);
		b = (BaseStarLONG - (float)a) * 100.0;
		WriteEEPROM(a, BaseStarLONGAddr);	
		WriteEEPROM(b, BaseStarLONGAddr + 1);
		
		WriteEEPROM(BaseStarRType, BaseStarRTypeAddr);
		WriteEEPROM(BaseStarPol, BaseStarPolAddr);						
			
		i = (int)(BaseStarFreq);
		j = (BaseStarFreq - i)*1000;
		a = i / 100;								//a=14
		b = i % 100;								//b=49
		c = j / 10;									//c=50
		d = ((int)j) % 10;							//d=0
		WriteEEPROM(a, BaseStarFreqAddr);//
		WriteEEPROM(b, BaseStarFreqAddr + 1);//
		WriteEEPROM(c, BaseStarFreqAddr + 2);//
		WriteEEPROM(d, BaseStarFreqAddr + 3);//
		Delay(2000);
	}
	return;
}


/*�õ����ݲ����棬Ϊ���������Ĳο���������*/
static void retYorN(void)
{
	UINT8 Num, count, t, Flag;
	TimeTest(1);

	Uart1SendData[Uart1TXMAXLenth++] = '$';

	Num = Uart1ReceData[i] - '0';
	Flag = 0;
	t = 0;
	for(count = 1; count < Num; count++)
	{
		if(Uart1ReceData[i + count] > '9' 			|| \
		   Uart1ReceData[i + count] < '0'           || \
		   Uart1ReceData[i + count + t] > '1'		|| \
		   Uart1ReceData[i + count + t + 4] > '1' 	|| \
		   Uart1ReceData[i + count + t + 5] > '1' 	|| \
		   Uart1ReceData[i + count + t + 6] > '1' 	|| \
		   Uart1ReceData[i + count + t + 7] > '2' 	|| \
		   Uart1ReceData[i + count + t + 12] > '1')
		{
			Flag = 1;	
		}
		t += 19;
	}
	
	if(Flag == 0)
	{
		for(count = 0; count < Num * 19; count++)
		{
			BSarr[count] = Uart1ReceData[i + count + 1];
		}
		Uart1SendData[Uart1TXMAXLenth++] = '3';
		StorBSarr(Uart1ReceData[i]);
	}
	else
	{
		Uart1SendData[Uart1TXMAXLenth++] = '4';
	}

	Uart1SendData[Uart1TXMAXLenth++] = '\n';
	Uart1TX();		
}

/*��������*/
static void StorBSarr(char ch)
{
	int a, b, c, d, x, y;
	int t, count;
	UINT16 Ttt;
	float Tf;
	double freqt;

	t = ch - '0';
	WriteEEPROM(t, BStarNumAddr);
	BaseNumber = t;	//�õ��ο����Ǹ�����

	for(count = 0; count < t; count++)
	{
	  	Tf = (BSarr[count * 19] - '0') * 100.0 + 		\
			 (BSarr[count * 19 + 1] - '0') * 10.0 +  	\
			 (BSarr[count * 19 + 2] - '0') * 1.0 + 		\
			 (BSarr[count * 19 + 3] - '0') * 0.1;
		bssT[count].Long = Tf;	///////////////////д���Ա����

		a = (int)(Tf);
		b = (Tf - (float)a) * 100.0;
		WriteEEPROM(a, BStarAddr + count * BStarLen);	
		WriteEEPROM(b, BStarAddr + 1 + count * BStarLen);

		bssT[count].PolF = BSarr[count * 19 + 4] - '0';///////
		WriteEEPROM(BSarr[count * 19 + 4] - '0', \
					BStarAddr + count * BStarLen + BStarPolAddr);
		
		bssT[count].WEFlag = BSarr[count * 19 + 5] - '0';///////
		WriteEEPROM(BSarr[count * 19 + 5] - '0', \
					BStarAddr + count * BStarLen + BStarEWFlagAddr);

		bssT[count].RecF = BSarr[count * 19 + 6] - '0';///////
		WriteEEPROM(BSarr[count * 19 + 6] - '0', \
					BStarAddr + count * BStarLen + BStarRTypeAddr);

		Tf = (BSarr[count * 19 + 7] - '0') * 10000.0 +	\
			 (BSarr[count * 19 + 8] - '0') * 1000.0 +	\
			 (BSarr[count * 19 + 9] - '0') * 100.0 +	\
			 (BSarr[count * 19 + 10] - '0') * 10.0 +	\
			 (BSarr[count * 19 + 11] - '0')* 1.0;
		bssT[count].KitB = Tf;///////////////////

		Ttt = (UINT16)Tf;
		WriteEEPROM((UINT8)(Ttt / 10000), \
					BStarAddr + count * BStarLen + BStarKbSAddr);
		Ttt %= 10000;
		WriteEEPROM((UINT8)(Ttt / 100), \
					BStarAddr + count * BStarLen + BStarKbSAddr + 1);
		Ttt %= 100;
		WriteEEPROM((UINT8)Ttt, \
					BStarAddr + count * BStarLen + BStarKbSAddr + 2);

		freqt = (BSarr[count * 19 + 12] - '0') * 1000.0 +	\
			 	(BSarr[count * 19 + 13] - '0') * 100.0 +	\
			 	(BSarr[count * 19 + 14] - '0') * 10.0 +	\
				(BSarr[count * 19 + 15] - '0') * 1.0 +	\
			 	(BSarr[count * 19 + 16] - '0')* 0.1+ \
				(BSarr[count * 19 + 17] - '0')* 0.001+ \
				(BSarr[count * 19 + 18] - '0')* 0.0001;
		bssT[count].Freq = freqt;  //////////////////////////////

		x = (int)(freqt);
		y = (freqt - x)*1000;
		a = x / 100;								//a=14
		b = x % 100;								//b=49
		c = y / 10;									//c=50
		d = ((int)y) % 10;							//d=0
		WriteEEPROM(a, \
					BStarAddr + count * BStarLen + BStarFreqAddr);
							//�洢�ű���ջ���Ƶ����������ǧλ��λֵ
		WriteEEPROM(b, \
					BStarAddr + count * BStarLen + BStarFreqAddr + 1);
						//�洢�ű���ջ���Ƶ����������ʮλ��λֵ
		WriteEEPROM(c, \
					BStarAddr + count * BStarLen + BStarFreqAddr + 2);
		WriteEEPROM(d, \
					BStarAddr + count * BStarLen + BStarFreqAddr + 3);											 
	}		
}


void RetFlag1(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='*';
	Uart1TX();	
}

void RetFlag2(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='%';
	Uart1TX();	
}