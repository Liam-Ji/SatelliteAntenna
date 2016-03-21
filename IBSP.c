/*
 *           Copyright 2007-2009 Availink, Inc.
 *
 *  This software contains Availink proprietary information and
 *  its use and disclosure are restricted solely to the terms in
 *  the corresponding written license agreement. It shall not be 
 *  disclosed to anyone other than valid licensees without
 *  written permission of Availink, Inc.
 *
 */



#include "IBSP.h"
#include "c8051f120.h"
#include "MyVariable.h"
#include <math.h>



A2108_ErrorCode A2108_IBSP_InitSemaphore( AVL_psemaphore pSemaphore )
{
	pSemaphore = pSemaphore;
	return(A2108_EC_OK);
}

A2108_ErrorCode A2108_IBSP_WaitSemaphore( AVL_psemaphore pSemaphore )
{
	pSemaphore = pSemaphore;
	return(A2108_EC_OK);
}

A2108_ErrorCode A2108_IBSP_ReleaseSemaphore( AVL_psemaphore pSemaphore )
{
	pSemaphore = pSemaphore;
	return(A2108_EC_OK);
}  



void A2108_IBSP_Delay( AVL_uint32 uiMS )
{  	// uiMS/=10;
 	 while(--uiMS){;}		
}
 
	void A2108_IBSP_Delay_ms( AVL_uint32 uiMS )
{  	 uiMS *= 420;
 	 while(--uiMS){;}		
}
   
A2108_ErrorCode A2108_IBSP_I2CRead( AVL_uint16 uiSlaveAddr,  AVL_puchar pucBuff, AVL_uint16 uiSize )
{   int i;
    AVL_uchar ack;

    uiSlaveAddr<<=1;
	uiSlaveAddr+=1;	

	SFRPAGE = 0x0F;	   //120�޸�
	Start_I2C();
    ack=SendByteAck(uiSlaveAddr); //����ѭ��ִ�У����͵�ַ����Ӧ 
	
	A2108_IBSP_Delay(10);  //+++++++++++++bj
	SCLK_0 ;
  	A2108_IBSP_Delay(2);
	SDATA_1;
	A2108_IBSP_Delay(2); //+++++++++++++++++++++==bj
		
 	for(i=0;i<uiSize-1;i++)
	{
	pucBuff[i]=RcvByte();   //���յ��ֽڷ������� ������Ӧ

	SDATA_0;
	A2108_IBSP_Delay(2);
	SCLK_1;
	A2108_IBSP_Delay(2);
	SCLK_0;
	A2108_IBSP_Delay(2);	  //+++++++++++++=bj
	SDATA_1;
	A2108_IBSP_Delay(2);

//	Ack_I2c(0);			 //-----------bj
	
	A2108_IBSP_Delay(10);

	}
	pucBuff[i]=RcvByte();	 //�������һ���ֽڲ���Ӧ
//	Ack_I2c(1);			  //--------------bj
	 A2108_IBSP_Delay(5);
	Stop_I2c();	 
 
	return(ack);
}


A2108_ErrorCode A2108_IBSP_I2CWrite(  AVL_uint16 uiSlaveAddr,  AVL_puchar pucBuff ,  AVL_uint16  uiSize )
{	AVL_uchar i,ack;     
    
	  
	uiSlaveAddr<<=1;

//	da=DeChunk32(pucBuff);
    SFRPAGE = 0x0F;	  //120�޸�
	Start_I2C();  			
 																		   
	ack=SendByteAck(uiSlaveAddr); 		   //ͬһ���ֽ�������ʱ��û������	
	 if(ack!=0) return(A2108_EC_I2CFail);  
	 for(i=0;i<uiSize;i++) 											  
	 {
 	 ack|=SendByteAck(pucBuff[i]);
	 if(ack!=0) return(A2108_EC_I2CFail); 
     }	    
    	
	Stop_I2c();	
		
	return(A2108_EC_OK);
  }



	static void Start_I2C(void)		 //��I2C		    
{	//SCLK_0 ;
  	//A2108_IBSP_Delay(30);		//---------bj  20														    
	SDATA_1 ;
  	A2108_IBSP_Delay(3);	// 10	   
  	SCLK_1 ;
  	A2108_IBSP_Delay(2);
  	SDATA_0;
    A2108_IBSP_Delay(3);
  	SCLK_0 ;
  	A2108_IBSP_Delay(2);
}
static void Stop_I2c(void)			 //�ر�I2C		    
{   
   // SCLK_0 ;	   
  	//A2108_IBSP_Delay(20);  //---------����20
  	SDATA_0;
  	A2108_IBSP_Delay(2); // 10	 
  	SCLK_1 ;
  	A2108_IBSP_Delay(2);
  	SDATA_1 ;
  	A2108_IBSP_Delay(3);
//	SCLK_0 ;
 // 	A2108_IBSP_Delay(30);		   //--------����20
}


static AVL_uchar SendByteAck(unsigned char c)	 //����һ���ֽڵı��ز���Ӧ		    
{
  	char BitCnt;
    AVL_uchar a;
  	for(BitCnt = 0;BitCnt<8;BitCnt++)
  	{	 
	   // SCLK_0 ;
		//A2108_IBSP_Delay(30);
    	if((c<<BitCnt)&0x80)
      		SDATA_1 ;
   	 	else 
			SDATA_0;
    	A2108_IBSP_Delay(2);
    	SCLK_1 ;
    	A2108_IBSP_Delay(5);
		SCLK_0 ;	//++++++++++++++=bj
		A2108_IBSP_Delay(3);	//+++++++++++++===bj
  	}

//	SCLK_0 ;			   //�˺�Ӧ��
//	A2108_IBSP_Delay(20);  
	a=AckCheck();		   //Ӧ����
	SCLK_0 ;			   
	A2108_IBSP_Delay(4);
    return(a);
}





static AVL_uchar RcvByte(void)
{
  AVL_uchar retc;
  char BitCnt;
  
  retc=0; 
  //SDATA_1;             //---------bj/*��������Ϊ���뷽ʽ*/
  for(BitCnt=0;BitCnt<8;BitCnt++)
      {
       // A2108_IBSP_Delay(20);           
        //SCLK_0;      //-------bj /*��ʱ����Ϊ�ͣ�׼����������λ*/
        //A2108_IBSP_Delay(50);	   //-------bj
        SCLK_1;       /*��ʱ����Ϊ��ʹ��������������Ч*/
        A2108_IBSP_Delay(2); 
        retc=retc<<1;
        if(SSDA_IN)  retc|= 0x01; /*������λ,���յ�����λ����retc�� */
		else         retc&= 0xFE; 
        A2108_IBSP_Delay(3) ;	   
		SCLK_0;      //++++++++++++bj /*��ʱ����Ϊ�ͣ�׼����������λ*/
        A2108_IBSP_Delay(5); 		
		  }	
		  
	    return(retc);	
  }


	static  AVL_uchar AckCheck(void)   //ACKӦ��   
{
  	AVL_uchar a=1;
	int count=0;
	SDATA_1;	  //+++++++++++++++++++++bj   8λ��������ͷ������ߣ�׼������Ӧ��λ
	A2108_IBSP_Delay(2);
   	//SCLK_0 ;			   //�˺�Ӧ��
	//A2108_IBSP_Delay(50);	//-------bj
  	SCLK_1;
 	A2108_IBSP_Delay(3);
  	a = SSDA_IN;

	while(a!=0&&count<10)	 // +++++++++++++++++bj+++28
	     {
		 a = SSDA_IN;
		 count++;
		 A2108_IBSP_Delay(2);
		 }
  	A2108_IBSP_Delay(2);
  	//SCLK_0 ;	  //-----------bj
  	return(a);
} 
