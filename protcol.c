//filename:Protcol.c
//author:wangyi,fengjiaji
//date:2008,1,15
//function:define the protcol of the sover and the controller
//功能：定义了监控函数和一系列的协议处理函数，对从串口1收到的一串命令，逐个进行处理，
//并将天线作出的反应以相应的数据格式再发给监控端。
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


static void SendRelease(void);				//返回版本号
static void ReturnSta(void);				//返回站点方位俯仰，如果为搜索状态返回搜索状态信息，如果为跟踪状态返回跟踪状态
static void ReturnGPS(void);				//返回GPS的经纬度信息
static void ReturnStationStatus(void);		//返回天线各个模块状态及天线本身状态
static void ReturnAZEL(void);				//返回方位俯仰
static void ReturnAGCPloar(void);			//返回AGC极化
static void ReturnSatLongPloarVet(void);	//返回目标卫星经度及极化方式
static void ReturnRecFreq(void);			//返回信标频率，或者sharp频率
static void ReturnStaLong(void);
static void ReturnStaLatitude(void);
static void ReturnXinBiaoAGCThreshold(void);
static void ReturnRecKind(void);			//返回接收机类型
static void ReturnCal(void);

static void SetStaLongPlo(void);	//设置目标卫星经度和极化
static void ChangeXinbiaoFreq(void);//改变信标频率
static void SetThreshold(void);		//设置门限
static void SetStationLongLat(void);//设置天线所在地经纬度
static void SetRecKind(void);		//设置接收机的类型


static void SetPloRight(void);		//设置极化较零
static void SetAZRight(void);		//设置方位较零
static void SetELRight(void);		//设置俯仰较零


static void DLRight(void);
static void DRRight(void);
static void DURight(void);
static void DAzaRight(void);
static void DAzbRight(void);
static void DSRight(void);
static void TotReturn(void);


static void AddEL(char c);			//增加俯仰
static void DelEL(char c);			//减少俯仰
static void AddAZ(char c);			//增加方位
static void DelAZ(char c);			//减少方位
static void GotoAZ(void);			//去自定义方位
static void GotoEL(void);			//去自定义俯仰
static void GotoPolar(void);
static void DelPlo(char c);
static void AddPlo(char c);


static void ReturnWarning(void);	//返回告警信息

static void Reset(void);			//复位
static void SetAutoMode(void);		//设置自动模式
static void SetManulMode(void);		//设置手动模式

static void SendSearching(void);	//发送搜索状态信息
static void SendTacking(void);		//发送锁定状态信息
static void SendWarning(void);		//发送各个模块的状态信息


static void ElRange(void);		    //2008-11-27发送俯仰边界


static void RetBaseStarData(void);				//发 参考卫星的参数
static void ReturnEnBaseStar(void);				//发送当前是否用到参考卫星
static void ReturnBitS(float TSymbolRate);		//返回目标符号率
static void StoreData(void);					//保存参考卫星数据
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
函数原型：void watch(void)
入口参数：无
出口参数：无
功能描述：天线控制器不定时的对串口1的接收发送数据进行监控，
		  如果有命令要处理则作出相应动作，此时一般会有数据要发送给人机对话端；
***********************************************************************/
void watch(void)
{
	//The first char of all commands from the Uart1 is between the char A and C;
	//Judge a command need judgeing the first char ;
	//char *head=Uart1ReceData;//把这个指针作为全局变量，可以省时间
	int commandlength;//处理完的命令长度
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

/*2009/11/3加入手动自动复位功能*/
	if(MAkit == 0 && status != INIT && status != STORESTATUS)
	{
		
		Delay(20);
		if(MAkit == 0)
		{
			MotorFun(MOTORSTOP, MOTORSTOP, SpeedInitF);
			if(status == SEARCHREADY)				//2010/6/30复位时再次复位用
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
				while(MAkit == 0)				//等待按键放开
				{
					LEDREDCOPEN;
				}
				Reset();						//2009/12/8
			}
			else
			{
				if(status == MANUALMODE)		//天线状态切换
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
			if(status == SEARCHREADY)				//2010/6/30复位时再次复位用
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
		 Uart1ReceData[Uart1ReceDataLength-1]!='\r')	 //等待一个命令接收完毕
		{
		 	CKcount1++;
			if(CKcount1 > 6000)
			{
				 CKcount1 = 0;
				 CKcount2++;
			}
		}
	}
	//CloseUart1();//要不要关串口1还需要测试
	i=0;

	head=Uart1ReceData;
	while(NumberOfComand>0)
	{
		switch (Uart1ReceData[i])
		{
			case 'S':
			{
				NoMonitorF = 0;
				if(Uart1ReceData[i+1]=='t' && Uart1ReceData[i+4]=='\r')//协议:Sta?<cr>	
					ReturnSta();//返回卫星信息
				else if(Uart1ReceData[i+1]=='e' && Uart1ReceData[i+4]=='\r')//协议:Set?<cr>
					ReturnSet();//返回天线设置
				else if(Uart1ReceData[i+1]=='_' && Uart1ReceData[i+2]=='?')//协议:S_?<cr>
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
				if(Uart1ReceData[i+7]=='P' && Uart1ReceData[i+9]=='\r')//协议:L****P*<cr>
				{
					if(status == SEARCHREADY)
					{
						AntReadySearchF = 0;  //2009/10/12复位后再次复位用
					}
					SetStaLongPlo();//设置目标卫星经度和极化
				}
				break;
			}
			case 'F':
			{
				if(Uart1ReceData[i+8]=='\r')//协议:F*******<cr>
					ChangeXinbiaoFreq();
				break;
			}
			case 'H':
			{
				if(Uart1ReceData[i+3]=='\r')//协议:H**<cr>
					SetThreshold();//设置门限				
				break;				
			}
			case 'P':
			{
				if(status!=INIT)
				{
					if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='+' && status!=STORESTATUS)//P*+<cr>
						AddEL(Uart1ReceData[1]);//加几度
					else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='-' && status!=STORESTATUS)//P*-<cr>
						DelEL(Uart1ReceData[1]);//减几度
					else if(Uart1ReceData[i+10]=='\r' && Uart1ReceData[i+1]=='Z' && Uart1ReceData[i+6]=='V'&& status!=STORESTATUS)//PZ+***V***<cr>或者PZ-***V***<cr>
						GotoEL();//以某个速度自定义加减俯仰
				}
				break;
			}
			case 'A':
			{
				if(status!=INIT)
				{
					if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='+' && status!=STORESTATUS)//A*+<cr>
						AddAZ(Uart1ReceData[1]);//加几度
					else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='-'&& status!=STORESTATUS)//A*-<cr>
						DelAZ(Uart1ReceData[i+1]);//减几度
					else if(Uart1ReceData[i+11]=='\r' && Uart1ReceData[i+1]=='Z' && Uart1ReceData[i+7]=='V' && status!=STORESTATUS)//AZ+****V***<cr>或者AZ-****V***<cr>
						GotoAZ();//以某个速度自定义加减方位
				}
				break;
				
			}
			case 'C'://C**<cf>
			{
				if(status!=INIT)
				{
					if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='+' && status!=STORESTATUS)
						AddPlo(Uart1ReceData[1]);//加几度极化
					else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='-'&& status!=STORESTATUS)
						DelPlo(Uart1ReceData[1]);//减几度计划
					else if(Uart1ReceData[i+1]=='Z' && status!=STORESTATUS)
					//CZ+***V***<cr>或者CZ-***V***<cr>
						GotoPolar();
				}
				break;
			}

			case 'D'://Dp****<cf>,Da****<cf>,De****<cf>
			{
				if(Uart1ReceData[i+6]=='\r' && Uart1ReceData[i+1]=='p')
				{
					SetPloRight();//设置极化较零
				}
				if(Uart1ReceData[i+6]=='\r' && Uart1ReceData[i+1]=='a')
				{
					SetAZRight();//设置方位较零
				}
				else if(Uart1ReceData[i+6]=='\r' && Uart1ReceData[i+1]=='e')
				{
					SetELRight();//设置俯仰较零
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
					ReturnWarning();//返回告警信息
				}
				break;
			}
			case 'M':
			{
				if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='R')//M_R<cf>,M_A<cf>,M_M<cf>
					Reset();//复位
				else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='A')
					SetAutoMode();//设置自动模式
				else if(Uart1ReceData[i+3]=='\r' && Uart1ReceData[i+2]=='M')
				{
					SetManulMode();//设置手动模式
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
					SetStationLongLat();//设置天线所在地经纬度
				break;
			}
			case 'B': //Ba!<cf>
			{
				if(Uart1ReceData[i+3]=='\r')
				{
					status = STORESTATUS;//如果此时状态不是收藏状态，再设置成一次收藏状态
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
				
			case 'G'://Go!<cf>,边瓣安装完毕
			{
				if(Uart1ReceData[i+3]=='\r')
				{
					if(status == INIT)//如果当前状态是初始化
					{
						BianBanFlag=1;//边瓣安装开始
					}
					else if(status == STORESTATUS)//如果当前状态是收藏状态
					{
						UninstallBianbanFlag=1;//边瓣卸载完毕
						ReturnStoreSuccess();
					}
					break;
				}	

			}
			case 'g' ://gps?<cf>//返回GPS读数
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

			/*2009/6/11加入新的协议*/
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

			case 'T': //TU,TR,TL等测试上台，左右收藏
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
		NumberOfComand--;//处理下一个命令
		if(NumberOfComand==0)
		{
			Uart1ReceDataLength=0;
			head=Uart1ReceData;
		}
		else
		{		
			while(*(head++)!='\r');//找这个命令的结尾
			commandlength=head-(Uart1ReceData+i);
			i=i+commandlength;
		}
	}
	Uart1TX();
}


/***********************************************************************
函数原型：void ReturnSta(void)
入口参数：无
出口参数：无
功能描述：返回天线方位/俯仰/极化角度，agc值
协议格式：Sta?<cr>
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
	if(LockFlag==0)//一旦锁定成功
		SendSearching();//发搜索状态信息
	else 
		SendTacking();//发锁定信息
//	SendRelease();//返回版本号2008-12-4

	ReturnBitS(SSrcStarKbS);
}
/***********************************************************************
函数原型：static void ReturnGPS(void)
入口参数：无
出口参数：无
功能描述：返回GPS的经纬度输出
协议格式：Sta?<cr>
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符

}
/***********************************************************************
函数原型：void ReturnSet(void)
入口参数：无
出口参数：无
功能描述：返回卫星经度，极化方式，信标频率，地球站经纬度,接收机类型
协议格式：Set?<cr>
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
	ElRange();								//2008-11-27发送边界
	ReturnBitS(SSrcStarKbS);
	SendRelease();							//返回版本号2008-12-4
}

/***********************************************************************
函数原型：void SetStaLongPlo(void)
入口参数：无
出口参数：无
功能描述：设置目标卫星经度和极化
协议格式：L****P*<cr>
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
		*以下存储卫星东西经标致,存储方法如下
		*存储0或者1
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
函数原型：void ChangeXinbiaoFreq(void)
入口参数：无
出口参数：无
功能描述：改变信标接收机的频率
协议格式：F*******<cr>
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
函数原型：void SetThreshold(void)
入口参数：无
出口参数：无
功能描述：设置门限
协议格式：H**<cr>
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
函数原型：void SetThreshold(void)
入口参数：无
出口参数：无
功能描述：设置接收机类型
协议格式：R*<cr>
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
函数原型：void AddEL(char c)
入口参数：字符，取值1，2，或3
出口参数：无
功能描述：增加俯仰1.0,0.3或者0.1
协议格式：P*+<cr>
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
函数原型：void DelEL(char c)
入口参数：字符，取值1，2，或3
出口参数：无
功能描述：减少俯仰1.0,0.3或者0.1
协议格式：P*-<cr>
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
函数原型：void AddAZ(char c)
入口参数：字符，取值1，2，或3
出口参数：无
功能描述：增加方位1.0,0.3或者0.1
协议格式：A*+<cr>
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
函数原型：void DelAZ(char c)
入口参数：字符，取值1，2，或3
出口参数：无
功能描述：减少方位1.0,0.3或者0.1
协议格式：A*-<cr>
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
函数原型：void GotoAZ(void)
入口参数：无
出口参数：无
功能描述：以某个速度自定义加减方位
协议格式：AZ+****V***<cr>或者AZ-****V***<cr>
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
函数原型：void GotoEL(void)
入口参数：无
出口参数：无
功能描述：以某个速度自定义加减俯仰
协议格式：PZ+***V***<cr>或者PZ-***V***<cr>
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
函数原型：void SetPloRight(void)
入口参数：无
出口参数：无
功能描述：设置极化较零
协议格式：
***********************************************************************/
static void SetPloRight(void)
{
    float RightPol;					//人为设置的正确极化角
	float StoreDeltaPol;			//最终算出来的要存储的差值
	float ReadDeltaPol = 0.0;		//读出来的差值
	float newpol = 0.0;

	newpol = GetComPolA();
	if(ReadEEPROM(PolCompensateAddress)=='+')		//如果读出来的是正值
	{
		ReadDeltaPol=ReadEEPROM(PolCompensateAddress+1);
	}
	else if(ReadEEPROM(PolCompensateAddress)=='-')	//如果读出来的是负值
	{
		ReadDeltaPol=0.0-ReadEEPROM(PolCompensateAddress+1);
	}

	RightPol=(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1;
   	if(Uart1ReceData[i+2]=='1')
		RightPol=0.0-RightPol;	

	StoreDeltaPol=ReadDeltaPol+(RightPol-newpol);//算出正确的误差值
	AngleCom=StoreDeltaPol;
	if(StoreDeltaPol>0)//如果存储的差值为正
	{
		WriteEEPROM('+',PolCompensateAddress);//存正号
		WriteEEPROM(StoreDeltaPol,PolCompensateAddress+1);
	}
	else
	{
		WriteEEPROM('-',PolCompensateAddress);//存负号
	   	StoreDeltaPol=0-StoreDeltaPol;//变成正的
		WriteEEPROM(StoreDeltaPol,PolCompensateAddress+1);
	}
	PolarFlag = 0;
	ReturnAZEL();
	ReturnAGCPloar();
	Uart1TX();
}


/***********************************************************************
函数原型：void SetAZRight(void)
入口参数：无
出口参数：无
功能描述：设置方位较零
协议格式：
***********************************************************************/
static void SetAZRight(void)
{
	float RightAz;				//人为设置的正确方位
	float StoreDeltaAz;			//最终算出来的要存储的差值
	float ReadDeltaAz = 0.0;	//读出来的差值

	if(ReadEEPROM(AZCompensateAddress)=='+')		//如果读出来的是正值
	{
		ReadDeltaAz=	ReadEEPROM(AZCompensateAddress+1)/10.0;
	}
	else if(ReadEEPROM(AZCompensateAddress)=='-')	//如果读出来的是负值
	{
		ReadDeltaAz=0.0-ReadEEPROM(AZCompensateAddress+1)/10.0;
	}
	RightAz=(Uart1ReceData[i+2]-'0')*100.0+(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1;

	StoreDeltaAz=ReadDeltaAz+(RightAz-StationAz);//算出正确的误差值
	if(StoreDeltaAz>0)//如果存储的差值为正
	{
		WriteEEPROM('+',AZCompensateAddress);//存正号
		WriteEEPROM(10*StoreDeltaAz,AZCompensateAddress+1);
	}
	else
	{
		WriteEEPROM('-',AZCompensateAddress);//存负号
	   	StoreDeltaAz=0-StoreDeltaAz;//变成正的
		WriteEEPROM(10*StoreDeltaAz,AZCompensateAddress+1);
	}
	StationAz=RightAz;
	ReturnAZEL();
	ReturnAGCPloar();
	Uart1TX();	
}
/***********************************************************************
函数原型：void SetELRight(void)
入口参数：无
出口参数：无
功能描述：设置俯仰较零
协议格式：
***********************************************************************/
static void SetELRight(void)
{
	float RightEl;				//人为设置的正确俯仰角
	float StoreDeltaEl;			//最终算出来的要存储的差值
	float ReadDeltaEl = 0.0;	//读出来的差值

	if(ReadEEPROM(ELCompensateAddress)=='+')		//如果读出来的是正值
	{
		ReadDeltaEl=ReadEEPROM(ELCompensateAddress+1)/10.0;
	}
	else if(ReadEEPROM(ELCompensateAddress)=='-')	//如果读出来的是负值
	{
		ReadDeltaEl=0.0-ReadEEPROM(ELCompensateAddress+1)/10.0;
	}
	RightEl=(Uart1ReceData[i+3]-'0')*10.0+(Uart1ReceData[i+4]-'0')*1.0+(Uart1ReceData[i+5]-'0')*0.1;
	StoreDeltaEl=ReadDeltaEl+(RightEl-StationEl);//算出正确的误差值
	GradientRightR = StoreDeltaEl;
	if(StoreDeltaEl>0)//如果存储的差值为正
	{
		WriteEEPROM('+',ELCompensateAddress);//存正号
		WriteEEPROM(10*StoreDeltaEl,ELCompensateAddress+1);
	}
	else
	{
		WriteEEPROM('-',ELCompensateAddress);//存负号
	   	StoreDeltaEl=0-StoreDeltaEl;//变成正的
		WriteEEPROM(10*StoreDeltaEl,ELCompensateAddress+1);
	}
	StationEl=RightEl;
	ReturnAZEL();
	ReturnAGCPloar();
	Uart1TX();
}
/***********************************************************************
函数原型：void ReturnWarning()
入口参数：无
出口参数：无
功能描述：检测倾斜仪，GPS，极化，并返回状态信息
***********************************************************************/
static void ReturnWarning()//返回告警信息
{
//	PloarNormal=0;//先置极化不正常
//	StationPol=GetComPolA();
//	if(StationPol<95.0 && StationPol>-95.0)
//		PloarNormal=1;//直接采样如果极化角正常则极化认为正常
	SendWarning();

}
/***********************************************************************
函数原型：static void ReturnRecKind(void)
入口参数：无
出口参数：无
功能描述：返回接收机类型
***********************************************************************/
static void ReturnRecKind(void)//返回接收机类型
{	
	Uart1SendData[Uart1TXMAXLenth++]='R';
	if(ReceiverKindFlag==XINBIAOREC)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else
		Uart1SendData[Uart1TXMAXLenth++]='1';	
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
}
/***********************************************************************
函数原型：static void Reset()
入口参数：无
出口参数：无
功能描述：准备重新开始对星
***********************************************************************/
static void Reset()//复位
{
	ResetFlag = 1;				//收到一次复位命令后，标志置真
	if(status == SEARCHREADY)
	{
		AntReadySearchF = 0;		//2009/10/12复位时再次复位用
	}
	status = SEARCHREADY;
}
/***********************************************************************
函数原型：void SetAutoMode()
入口参数：无
出口参数：无
功能描述：设置自动模式
***********************************************************************/
static void SetAutoMode()
{
	if(status > SEARCHREADY && status < STORESTATUS)
	{
		status=SEARCHING;	
	}
}
/***********************************************************************
函数原型：void SetManulMode()
入口参数：无
出口参数：无
功能描述：设置手动模式
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
函数原型：void SetStationLongLat()
入口参数：无
出口参数：无
功能描述：天线控制器重新设置地球站经纬度
***********************************************************************/
static void SetStationLongLat()//设置天线所在地经纬度
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
函数原型：static void ReturnAZEL(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前天线的方位和俯仰；
协议格式：A****E***<LF>
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
	ElRange();								//2008-11-27发送边界	
}
/***********************************************************************
函数原型：static void ReturnAGCPloar(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前天线AGC和极化角度；
协议格式：G****R****<LF>
***********************************************************************/
static void ReturnAGCPloar(void)
{
	char temp[4];
	AGC=GetAGC();//这里不应该再采了
	itoa(AGC*100,temp,4);
	Uart1SendData[Uart1TXMAXLenth++]='G';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]=temp[2];
	Uart1SendData[Uart1TXMAXLenth++]=temp[3];
	Uart1SendData[Uart1TXMAXLenth++]='R';
	StationPol=GetComPolA();//这个是否也要等转完了再采
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
}


/***********************************************************************
函数原型：static void ReturnSatLongPloarVet(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前设置的卫星经度和极化方式
协议格式：L****Z*P*<LF>
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符

}

/***********************************************************************
函数原型：static void ReturnRecFreq(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前设置的信标频率或者夏普接收机频率
协议格式：F*******<LF>
***********************************************************************/
static void ReturnRecFreq(void)
{
	char temp[4],temp1[3];
	double FreqInt;//整数
	double FreqDec;//小数 	
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
		Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
	}
	else//sharp接收机带宽较大，所以只精确到小数点第一位
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
		Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
	}
}

/***********************************************************************
函数原型：static void ReturnStaLong(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前站点的经度(包括东西经信息)
协议格式：J****Z*<LF>
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符

}
/***********************************************************************
函数原型：static void ReturnStaLatitude(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前站点的纬度（包括南北纬信息）
协议格式：W****Z*<LF>
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
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符

}
/***********************************************************************
函数原型：static void ReturnAGCThreshold(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回目前信标接收机设置的AGC跟踪门限值
协议格式：H**<LF>
***********************************************************************/
static void ReturnXinBiaoAGCThreshold(void)
{
	char temp[2];
	itoa(Threshold*10,temp,2);
	Uart1SendData[Uart1TXMAXLenth++]='H';
	Uart1SendData[Uart1TXMAXLenth++]=temp[0];
	Uart1SendData[Uart1TXMAXLenth++]=temp[1];
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符

}


/***********************************************************************
函数原型：static void SendWarning(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回天线告警信息
协议格式：j*q*g*l*p*<LF>
***********************************************************************/
static void SendWarning(void)
{
	
	Uart1SendData[Uart1TXMAXLenth++]='j';
	if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';//信标接收机与夏普接收机均故障
	else if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';//信标接收机故障，夏普接收机正常
	else if(XinBiaoRecNormalFlag==1 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='2';//信标接收机正常，夏普接收机故障
	else
		Uart1SendData[Uart1TXMAXLenth++]='3';//信标接收机正常，夏普接收机正常

	Uart1SendData[Uart1TXMAXLenth++]='q';
	if(GradientNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GradientNormal==1) 
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//倾斜仪目前有三种状态

	Uart1SendData[Uart1TXMAXLenth++]='g';
	if(GPSNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GPSNormal==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//GPS目前有三种状态

	Uart1SendData[Uart1TXMAXLenth++]='l';
	Uart1SendData[Uart1TXMAXLenth++]='0';

	Uart1SendData[Uart1TXMAXLenth++]='p';
	if(PloarNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='1';//极化目前只有两种状态


	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
	Uart1TX();
}

/***********************************************************************
函数原型：static void SendSearching(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器发送搜索状态命令
协议格式：X!
***********************************************************************/
static void SendSearching(void)
{

	Uart1SendData[Uart1TXMAXLenth++]='X';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
}

/***********************************************************************
函数原型：static void SendTacking(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器发送锁定状态命令
协议格式：Y!
***********************************************************************/
static void SendTacking(void)
{

	Uart1SendData[Uart1TXMAXLenth++]='Y';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
}

/***********************************************************************
函数原型：static void ReturnStationStatus(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器发送告警和状态信息
协议格式：j*q*g*l*p*S_*<LF>
***********************************************************************/
void ReturnStationStatus(void)
{
	
	Uart1SendData[Uart1TXMAXLenth++]='j';
	if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';//信标接收机与夏普接收机均故障
	else if(XinBiaoRecNormalFlag==0 && SharpRecNormalFlag==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';//信标接收机故障，夏普接收机正常
	else if(XinBiaoRecNormalFlag==1 && SharpRecNormalFlag==0)
		Uart1SendData[Uart1TXMAXLenth++]='2';//信标接收机正常，夏普接收机故障
	else
		Uart1SendData[Uart1TXMAXLenth++]='3';//信标接收机正常，夏普接收机正常

	Uart1SendData[Uart1TXMAXLenth++]='q';
	if(GradientNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GradientNormal==1) 
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//倾斜仪目前有三种状态

	Uart1SendData[Uart1TXMAXLenth++]='g';
	if(GPSNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(GPSNormal==1)
		Uart1SendData[Uart1TXMAXLenth++]='1';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='2';//GPS目前有三种状态

	Uart1SendData[Uart1TXMAXLenth++]='l';
	Uart1SendData[Uart1TXMAXLenth++]='0';

	Uart1SendData[Uart1TXMAXLenth++]='p';
	if(PloarNormal==0)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='1';//极化目前只有两种状态

	Uart1SendData[Uart1TXMAXLenth++]='S';
	Uart1SendData[Uart1TXMAXLenth++]='_';
	if(status==INIT)
		Uart1SendData[Uart1TXMAXLenth++]='0';
	else if(status==MANUALMODE)
		Uart1SendData[Uart1TXMAXLenth++]='2';
	else 
		Uart1SendData[Uart1TXMAXLenth++]='1';
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
	SendRelease();//返回版本号2008-12-4
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


/*2008-11-14加入以下校正*/
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
			
	Uart1SendData[Uart1TXMAXLenth++]='\n';//结束符
	Uart1TX();
	return;
}


/***********************************************************************
函数原型：void ReturnCal(void)
入口参数：无
出口参数：无
功能描述：天线反回理论计算值
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


/*返回EL范围*/
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
函数原型：static void ReturnResetSuccess(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器返回天线复位完毕
协议格式：Rs!
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
函数原型：void SendSJ(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器发送关机卸载边瓣信息
协议格式：SJ!
***********************************************************************/
void SendSJ(void)
{
	Uart1SendData[Uart1TXMAXLenth++]='S';
	Uart1SendData[Uart1TXMAXLenth++]='J';
	Uart1SendData[Uart1TXMAXLenth++]='!';
	Uart1TX();
}

/*版本号 AKD10K01*/
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
函数原型：void AddPlo(char c)
入口参数：字符'1','2','3'
出口参数：无
功能描述：增加极化角1度
协议格式：
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
函数原型：void AddPlo(char c)
入口参数：字符'1','2','3'
出口参数：无
功能描述：减少极化角1度
协议格式：
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
函数原型：void GotoPolar(void)
入口参数：无
出口参数：无
功能描述：走到目标极化角
协议格式：
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
函数原型：void SendKJ(void)
入口参数：无
出口参数：无
功能描述：天线控制器向监控服务器发送开机装载边瓣信息
协议格式：KJ!
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

/*2009/6/12加入以下函数*/
static void RetBaseStarData(void)
{
	char temp[4],temp1[3];
	double FreqInt;//整数
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


/*得到数据并保存，为主控软件里的参考卫星数据*/
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

/*保存数据*/
static void StorBSarr(char ch)
{
	int a, b, c, d, x, y;
	int t, count;
	UINT16 Ttt;
	float Tf;
	double freqt;

	t = ch - '0';
	WriteEEPROM(t, BStarNumAddr);
	BaseNumber = t;	//得到参考卫星个数；

	for(count = 0; count < t; count++)
	{
	  	Tf = (BSarr[count * 19] - '0') * 100.0 + 		\
			 (BSarr[count * 19 + 1] - '0') * 10.0 +  	\
			 (BSarr[count * 19 + 2] - '0') * 1.0 + 		\
			 (BSarr[count * 19 + 3] - '0') * 0.1;
		bssT[count].Long = Tf;	///////////////////写入成员经度

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
							//存储信标接收机的频率整数部分千位百位值
		WriteEEPROM(b, \
					BStarAddr + count * BStarLen + BStarFreqAddr + 1);
						//存储信标接收机的频率整数部分十位个位值
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