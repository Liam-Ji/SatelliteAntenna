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



//$Revision: 323 $ 
//$Date: 2008-03-14 11:31:11 -0400 (Fri, 14 Mar 2008) $
// 
#include "II2C.h"
#include "IBSP.h"

//#include "CPU020.h"
//#include "Uart1.h" //+++++++++++=bj

AVL_semaphore gI2CSem;

unsigned char pucSendBuff[64];

A2108_ErrorCode A2108_II2C_Initialize(void)
{
	A2108_ErrorCode r = A2108_EC_OK;
	static AVL_uchar gI2CSem_inited = 0;
	if( 0 == gI2CSem_inited )
	{
		gI2CSem_inited = 1;
	//	r = A2108_IBSP_InitSemaphore(&gI2CSem);
	}
	return r;
}


/*Read uiSize bytes from AVL2108 to pucBuff from AVL2108 internal address uiOffset.
Parameters: 
uiSlaveAddr;  The slave address of the AVL2108 chip.   
uiOffset;  The AVL2108 internal address where the function will read data from.   
pucBuff;  Carries back the data read from the AVL2108 chip.����2108оƬ������   
uiSize;  Tells the function read back how many bytes. ���ص��ֽ��� 
 ��2108��uiOffset�ж�ȡuiSize���ֽڷ���pucBuff������*/
A2108_ErrorCode A2108_II2C_Read( AVL_uint16 uiSlaveAddr, AVL_uint32 uiOffset, AVL_puchar pucBuff, AVL_uint16 uiSize)
{
	A2108_ErrorCode r;
	AVL_uchar pucBuffTemp[3];
	AVL_uint16 ui1, ui2;
	AVL_uint16 iSize;

	r = A2108_IBSP_WaitSemaphore(&(gI2CSem));
	if( A2108_EC_OK == r )
	{
		ChunkAddr(uiOffset, pucBuffTemp); // pucBuffTemp�����Ԫ�ش��uioffset�ĸ��ֽ�		   
		ui1 = 3;
		r = A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, ui1);  
		if( A2108_EC_OK == r )
		{
			if( uiSize & 1 )
			{
				iSize = uiSize - 1;
			}
			else
			{
				iSize = uiSize;
			}
			ui2 = 0;
			while( iSize > MAX_II2C_READ_SIZE )	   
			{
				ui1 = MAX_II2C_READ_SIZE;	// #define MAX_II2C_READ_SIZE 64	 #define MAX_II2C_Write_SIZE64 
				r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuff+ui2, ui1);
				ui2 += MAX_II2C_READ_SIZE;
				iSize -= MAX_II2C_READ_SIZE;
			}

			if( 0 != iSize )	 //��Ҫû�н��յ����ݣ���������
			{
				r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuff+ui2, iSize);
			}

			if( uiSize & 1 )   //���uiSize�����������洦�����һ������
			{
				ui1 = 2;
				r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuffTemp, ui1);
				pucBuff[uiSize-1] = pucBuffTemp[0];
			}
		}
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(gI2CSem)); 
	return(r);
}

/*A2108_ErrorCode A2108_II2C_ReadDirect( AVL_uint16 uiSlaveAddr, AVL_puchar pucBuff, AVL_uint16 uiSize)
{
	A2108_ErrorCode r;
	AVL_uchar pucBuffTemp[3];
	AVL_uint16 ui1, ui2;
	AVL_uint16 iSize;
	r = A2108_IBSP_WaitSemaphore(&(gI2CSem));
	if( A2108_EC_OK == r )
	{
		if( uiSize & 1 )
		{
			iSize = uiSize - 1;
		}
		else
		{
			iSize = uiSize;
		}
		ui2 = 0;
		while( iSize > MAX_II2C_READ_SIZE )
		{
			ui1 = MAX_II2C_READ_SIZE;
			r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuff+ui2, ui1);
			ui2 += MAX_II2C_READ_SIZE;
			iSize -= MAX_II2C_READ_SIZE;
		}

		if( 0 != iSize )
		{
			r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuff+ui2, iSize);
		}

		if( uiSize & 1 )
		{
			ui1 = 2;
			r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuffTemp, ui1);
			pucBuff[uiSize-1] = pucBuffTemp[0];
		}
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(gI2CSem)); 
	return(r);
}
	  */
  /*uiSlaveAddr  The slave address of the AVL2108 chip.   
 pucBuff  Holds the data which will be sent to AVL2108 chip.   
 uiSize  Tells the function how many bytes will be sent out. 
Returns:  A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return A2108_EC_I2CFail if there is an I2C problem.  
 ��uiSize���ֽ���(����pucBuff)���͵�2108оƬ*/
A2108_ErrorCode A2108_II2C_Write( AVL_uint16 uiSlaveAddr, AVL_puchar pucBuff, AVL_uint16 uiSize)
{	 //AVL_uchar pucSendBuff[64];	 //+++++++++++++++==bj
//       int j;               //++++++++++++++++++++++
//	unsigned char Tempch;
//	unsigned char count;	
	A2108_ErrorCode r;
	AVL_uchar pucBuffTemp[5];
	AVL_uint16 ui1, ui2, uTemp,i;
	AVL_uint16 iSize;
	AVL_uint32 uAddr;
	if( uiSize<3 )
	{
		return(A2108_EC_GeneralFail);	 //���������ֽ�
	}

	uiSize -= 3;			//actual data size
	r = A2108_IBSP_WaitSemaphore(&(gI2CSem));
	if( A2108_EC_OK == r )
	{
		//dump address
		uAddr = pucBuff[0];
		uAddr = uAddr<<8;
		uAddr += pucBuff[1];
		uAddr = uAddr<<8;
		uAddr += pucBuff[2];  //uAddrΪ��ַ

		if( uiSize & 1 )   //���uiSize��������iSize=uiSize-1�� ż��iSize=uiSize
		{
			iSize = uiSize -1;
		}
		else
		{
			iSize = uiSize;
		}

		uTemp = (MAX_II2C_Write_SIZE-3) & 0xfffe; //60how many bytes data we can transfer every time

		ui2 = 0;
		while( iSize > uTemp )
		{	
			ui1 = uTemp+3;	    
			//save the data
			for(i=0; i<ui1; i++)
				pucSendBuff[i] = pucBuff[ui2+i];
/*			pucBuffTemp[0] = pucBuff[ui2];
			pucBuffTemp[1] = pucBuff[ui2+1];
			pucBuffTemp[2] = pucBuff[ui2+2];//�Ȱ�pucBuff�е�����Ԫ�ش�����ʱ����temp
*/
			ChunkAddr(uAddr, pucSendBuff);	//��uAddr��Ӧ���ֽڴ������	pucharSendBuff  
		//	for(j=0;j<100;j++)
		//	{ 
			r |= A2108_IBSP_I2CWrite(uiSlaveAddr, pucSendBuff, ui1);
		//	}
		/*	if(r!=0)
			{
				count = 0;
			 for(Uart1TXMAXLenth=0;Uart1TXMAXLenth<64;Uart1TXMAXLenth++)
			  {	
			  	  Tempch = (pucSendBuff[Uart1TXMAXLenth] & 0xf0) >> 4;  //
				  if(Tempch > 9)
				  {
						Uart1SendData[count++]= Tempch - 10 + 'A';				  		
				  }
				  else
				  {
				  		Uart1SendData[count++]= Tempch + '0';				  		
				  }
				  
				  Tempch = pucSendBuff[Uart1TXMAXLenth] & 0xf;	  //
				  if(Tempch > 9)
				  {
						Uart1SendData[count++]= Tempch - 10 + 'A';				  		
				  }
				  else
				  {
				  		Uart1SendData[count++]= Tempch + '0';				  		
				  }				  
			   	  Uart1SendData[count++]= ' '; 
				    
			   }

			   Uart1SendData[count++]='\r';				   
				Uart1SendData[count]='\n';
			  	Uart1TXMAXLenth=count;
			   Uart1TX(); 
			   CloseUart1();
		 	}	 */
		
/*		 	//restore data
			pucBuff[ui2] = pucBuffTemp[0]; //��temp�е�������pucBuff
			pucBuff[ui2+1] = pucBuffTemp[1];
			pucBuff[ui2+2] = pucBuffTemp[2];
*/
			uAddr += uTemp;	   //�Ĵ�����ַ��60
			ui2 += uTemp;
			iSize -= uTemp;			/*�������Ա�֤pucBuff���ֲ���*/
		}
		ui1 = iSize+3;
		for(i=0; i<ui1; i++)
			pucSendBuff[i] = pucBuff[ui2+i];
/*
		//save the data
		pucBuffTemp[0] = pucBuff[ui2];
		pucBuffTemp[1] = pucBuff[ui2+1];
		pucBuffTemp[2] = pucBuff[ui2+2];
*/
		ChunkAddr(uAddr, pucSendBuff);
	

		r |= A2108_IBSP_I2CWrite(uiSlaveAddr, pucSendBuff, ui1); 
		
	/*	
		//restore data
		pucBuff[ui2] = pucBuffTemp[0];
		pucBuff[ui2+1] = pucBuffTemp[1];
		pucBuff[ui2+2] = pucBuffTemp[2];   */
		uAddr += iSize;	   // �Ĵ�����ַ��4
		ui2 += iSize;	  //120+4	 


		if( uiSize & 1 )	  //���Ϊ����
		{
			ChunkAddr(uAddr, pucBuffTemp);	//��ǰ�Ĵ�����ַ����pucTemp[0][1][2]��
			ui1 = 3;
			r |= A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, ui1);//���ͼĴ�����ַ
			ui1 = 2;
			r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuffTemp+3, ui1);//���ؼĴ����е�2������pucTemp[3][4]��
			pucBuffTemp[3] = pucBuff[ui2+3];//���һ���ֽڷ���pucTemp[3]��
			ui1 = 5;
			r |= A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, ui1);  //����pucTemp��[0][1][2][3][4]
		}
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(gI2CSem)); 

	return(r);
}


	A2108_ErrorCode A2108_II2C_Read16( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint16 puiData )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[2];

	r = A2108_II2C_Read(uiSlaveAddr, uiAddr, pBuff, 2);		//��СΪ2��pBuff������16λ����
	if( A2108_EC_OK == r )
	{
		*puiData = DeChunk16(pBuff);
	}
	return(r);
}
 /*Read back a 32 bits unsigned integer from AVL2108 chip. 
 ��2108оƬ�ж�ȡ32λ�޷�������
 Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.  2108оƬ��ַ 
 uiAddr  The AVL2108 internal address where the function will read data from.  2108�ڲ���ַ�����������ж�ȡ���� 
 puiData  Carries back the 32 bits unsigned integer. ����32λ�޷�������*/
A2108_ErrorCode A2108_II2C_Read32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint32 puiData )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[4];

	r = A2108_II2C_Read(uiSlaveAddr, uiAddr, pBuff, 4);
	if( A2108_EC_OK == r )
	{
		*puiData = DeChunk32(pBuff);  //pBuff[0]<<24+pBuff[1]<<16+pBuff[2]<<8+pBuff[3]
	}
	return(r);
}

A2108_ErrorCode A2108_II2C_Write16(AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint16 uiData )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[5];	   //��СΪ5���޷�������

	ChunkAddr(uiAddr, pBuff);  //����ǰ����Ԫ�ش�ŵ�ַ
	Chunk16(uiData, pBuff+3);  //������Ԫ��    ���16λ����

	r = A2108_II2C_Write(uiSlaveAddr, pBuff, 5);
	return(r);
}


/*Write a 32 bits unsigned integer to AVL2108 chip.    
Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.   
 uiAddr  The AVL2108 internal address where the function will read data from 
 uiData  The data will be write to AVL2108 chip.   
Returns:  A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return  A2108_EC_I2CFail if there is a I2C problem. 
��uiAddr�ж�ȡuiDataд��2108   */
A2108_ErrorCode A2108_II2C_Write32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint32 uiData )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[7];

	ChunkAddr(uiAddr, pBuff); /*pBuffǰ����Ԫ�ش�ŵ�ַ	pBuff[0] =(AVL_uchar)(uiaddr>>16);
		                                                 pBuff[1] =(AVL_uchar)(uiaddr>>8);
		                                                 pBuff[2] =(AVL_uchar)(uiaddr); */
	Chunk32(uiData, pBuff+3);	//���ĸ�Ԫ�ش��32λuiData��Ӧ���ĸ��ֽ�
	r = A2108_II2C_Write(uiSlaveAddr, pBuff, 7);
	return(r);
}



/*A2108_ErrorCode A2108_II2C_Read16( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint16 puiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[2];
	AVL_uchar pucBuffTemp[3];


	ChunkAddr(uiAddr, pucBuffTemp); // pucBuffTemp�����Ԫ�ش��uioffset�ĸ��ֽ�++++++++	
	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, 3);//���ͼĴ�����ַ+++++++++
	r |= A2108_IBSP_I2CRead(uiSlaveAddr, pBuff, 2);	//����2���ֽڴ���pBuff++++++++	 
/*	r = A2108_II2C_Read(uiSlaveAddr, uiAddr, pBuff, 2);	*/	
/*	if( A2108_EC_OK == r )
	{
		*puiData = DeChunk16(pBuff);
	}
	return(r);
} 	*/
 /*Read back a 32 bits unsigned integer from AVL2108 chip. 
 ��2108оƬ�ж�ȡ32λ�޷�������
 Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.  2108оƬ��ַ 
 uiAddr  The AVL2108 internal address where the function will read data from.  2108�ڲ���ַ�����������ж�ȡ���� 
 puiData  Carries back the 32 bits unsigned integer. ����32λ�޷�������*/

/*A2108_ErrorCode A2108_II2C_Read32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint32 puiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[4];
	AVL_uchar pucBuffTemp[3];

	ChunkAddr(uiAddr, pucBuffTemp); // pucBuffTemp�����Ԫ�ش��uioffset�ĸ��ֽ�++++++++	
	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, 3);//���ͼĴ�����ַ+++++++++
	r |= A2108_IBSP_I2CRead(uiSlaveAddr, pBuff, 4);	//����4���ֽڴ���pBuff++++++++	 
	/*r = A2108_II2C_Read(uiSlaveAddr, uiAddr, pBuff, 4);*/
/*	if( A2108_EC_OK == r )
	{
		*puiData = DeChunk32(pBuff);  //pBuff[0]<<24+pBuff[1]<<16+pBuff[2]<<8+pBuff[3]
	}
	return(r);
}

A2108_ErrorCode A2108_II2C_Write16(AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint16 uiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[5];	   //��СΪ5���޷�������

	ChunkAddr(uiAddr, pBuff);
	Chunk16(uiData, pBuff+3); 

	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pBuff, 5);	 //+++++++++
/*	r = A2108_II2C_Write(uiSlaveAddr, pBuff, 5);   */
/*	return(r);
}


/*Write a 32 bits unsigned integer to AVL2108 chip.    
Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.   
 uiAddr  The AVL2108 internal address where the function will read data from 
 uiData  The data will be write to AVL2108 chip.   
Returns:  A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return  A2108_EC_I2CFail if there is a I2C problem. 
��uiAddr�ж�ȡuiDataд��2108   */
/*A2108_ErrorCode A2108_II2C_Write32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint32 uiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[7];

	ChunkAddr(uiAddr, pBuff); /*pBuffǰ����Ԫ�ش�ŵ�ַ	pBuff[0] =(AVL_uchar)(uiaddr>>16);
		                                                 pBuff[1] =(AVL_uchar)(uiaddr>>8);
		                                                 pBuff[2] =(AVL_uchar)(uiaddr); */
/*	Chunk32(uiData, pBuff+3);	//���ĸ�Ԫ�ش��32λuiData��Ӧ���ĸ��ֽ�

	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pBuff, 7);	 //+++++++++
	/*r = A2108_II2C_Write(uiSlaveAddr, pBuff, 7);	*/
/*	return(r);
} */
