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
pucBuff;  Carries back the data read from the AVL2108 chip.返回2108芯片的数据   
uiSize;  Tells the function read back how many bytes. 读回的字节数 
 从2108的uiOffset中读取uiSize个字节放入pucBuff数组中*/
A2108_ErrorCode A2108_II2C_Read( AVL_uint16 uiSlaveAddr, AVL_uint32 uiOffset, AVL_puchar pucBuff, AVL_uint16 uiSize)
{
	A2108_ErrorCode r;
	AVL_uchar pucBuffTemp[3];
	AVL_uint16 ui1, ui2;
	AVL_uint16 iSize;

	r = A2108_IBSP_WaitSemaphore(&(gI2CSem));
	if( A2108_EC_OK == r )
	{
		ChunkAddr(uiOffset, pucBuffTemp); // pucBuffTemp数组各元素存放uioffset的各字节		   
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

			if( 0 != iSize )	 //还要没有接收的数据，继续接收
			{
				r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuff+ui2, iSize);
			}

			if( uiSize & 1 )   //如果uiSize是奇数，下面处理最后一个数据
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
 把uiSize个字节数(存于pucBuff)发送到2108芯片*/
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
		return(A2108_EC_GeneralFail);	 //至少三个字节
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
		uAddr += pucBuff[2];  //uAddr为地址

		if( uiSize & 1 )   //如果uiSize奇数，则iSize=uiSize-1， 偶数iSize=uiSize
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
			pucBuffTemp[2] = pucBuff[ui2+2];//先把pucBuff中的三个元素存于临时数组temp
*/
			ChunkAddr(uAddr, pucSendBuff);	//把uAddr对应的字节存放数组	pucharSendBuff  
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
			pucBuff[ui2] = pucBuffTemp[0]; //把temp中的数导回pucBuff
			pucBuff[ui2+1] = pucBuffTemp[1];
			pucBuff[ui2+2] = pucBuffTemp[2];
*/
			uAddr += uTemp;	   //寄存器地址加60
			ui2 += uTemp;
			iSize -= uTemp;			/*这样可以保证pucBuff保持不变*/
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
		uAddr += iSize;	   // 寄存器地址加4
		ui2 += iSize;	  //120+4	 


		if( uiSize & 1 )	  //如果为奇数
		{
			ChunkAddr(uAddr, pucBuffTemp);	//当前寄存器地址放入pucTemp[0][1][2]中
			ui1 = 3;
			r |= A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, ui1);//发送寄存器地址
			ui1 = 2;
			r |= A2108_IBSP_I2CRead(uiSlaveAddr, pucBuffTemp+3, ui1);//读回寄存器中的2个数放pucTemp[3][4]中
			pucBuffTemp[3] = pucBuff[ui2+3];//最后一个字节放入pucTemp[3]中
			ui1 = 5;
			r |= A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, ui1);  //发送pucTemp的[0][1][2][3][4]
		}
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(gI2CSem)); 

	return(r);
}


	A2108_ErrorCode A2108_II2C_Read16( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint16 puiData )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[2];

	r = A2108_II2C_Read(uiSlaveAddr, uiAddr, pBuff, 2);		//大小为2的pBuff数组存放16位的数
	if( A2108_EC_OK == r )
	{
		*puiData = DeChunk16(pBuff);
	}
	return(r);
}
 /*Read back a 32 bits unsigned integer from AVL2108 chip. 
 从2108芯片中读取32位无符号整型
 Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.  2108芯片地址 
 uiAddr  The AVL2108 internal address where the function will read data from.  2108内部地址，函数将从中读取数据 
 puiData  Carries back the 32 bits unsigned integer. 返回32位无符号整型*/
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
	AVL_uchar pBuff[5];	   //大小为5的无符号数组

	ChunkAddr(uiAddr, pBuff);  //数组前三个元素存放地址
	Chunk16(uiData, pBuff+3);  //后两个元素    存放16位数据

	r = A2108_II2C_Write(uiSlaveAddr, pBuff, 5);
	return(r);
}


/*Write a 32 bits unsigned integer to AVL2108 chip.    
Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.   
 uiAddr  The AVL2108 internal address where the function will read data from 
 uiData  The data will be write to AVL2108 chip.   
Returns:  A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return  A2108_EC_I2CFail if there is a I2C problem. 
从uiAddr中读取uiData写入2108   */
A2108_ErrorCode A2108_II2C_Write32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint32 uiData )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[7];

	ChunkAddr(uiAddr, pBuff); /*pBuff前三个元素存放地址	pBuff[0] =(AVL_uchar)(uiaddr>>16);
		                                                 pBuff[1] =(AVL_uchar)(uiaddr>>8);
		                                                 pBuff[2] =(AVL_uchar)(uiaddr); */
	Chunk32(uiData, pBuff+3);	//后四个元素存放32位uiData对应的四个字节
	r = A2108_II2C_Write(uiSlaveAddr, pBuff, 7);
	return(r);
}



/*A2108_ErrorCode A2108_II2C_Read16( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint16 puiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[2];
	AVL_uchar pucBuffTemp[3];


	ChunkAddr(uiAddr, pucBuffTemp); // pucBuffTemp数组各元素存放uioffset的各字节++++++++	
	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, 3);//发送寄存器地址+++++++++
	r |= A2108_IBSP_I2CRead(uiSlaveAddr, pBuff, 2);	//读的2个字节存于pBuff++++++++	 
/*	r = A2108_II2C_Read(uiSlaveAddr, uiAddr, pBuff, 2);	*/	
/*	if( A2108_EC_OK == r )
	{
		*puiData = DeChunk16(pBuff);
	}
	return(r);
} 	*/
 /*Read back a 32 bits unsigned integer from AVL2108 chip. 
 从2108芯片中读取32位无符号整型
 Parameters: 
 uiSlaveAddr  The slave address of the AVL2108 chip.  2108芯片地址 
 uiAddr  The AVL2108 internal address where the function will read data from.  2108内部地址，函数将从中读取数据 
 puiData  Carries back the 32 bits unsigned integer. 返回32位无符号整型*/

/*A2108_ErrorCode A2108_II2C_Read32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint32 puiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[4];
	AVL_uchar pucBuffTemp[3];

	ChunkAddr(uiAddr, pucBuffTemp); // pucBuffTemp数组各元素存放uioffset的各字节++++++++	
	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pucBuffTemp, 3);//发送寄存器地址+++++++++
	r |= A2108_IBSP_I2CRead(uiSlaveAddr, pBuff, 4);	//读的4个字节存于pBuff++++++++	 
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
	AVL_uchar pBuff[5];	   //大小为5的无符号数组

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
从uiAddr中读取uiData写入2108   */
/*A2108_ErrorCode A2108_II2C_Write32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint32 uiData )
{
	A2108_ErrorCode r=0;
	AVL_uchar pBuff[7];

	ChunkAddr(uiAddr, pBuff); /*pBuff前三个元素存放地址	pBuff[0] =(AVL_uchar)(uiaddr>>16);
		                                                 pBuff[1] =(AVL_uchar)(uiaddr>>8);
		                                                 pBuff[2] =(AVL_uchar)(uiaddr); */
/*	Chunk32(uiData, pBuff+3);	//后四个元素存放32位uiData对应的四个字节

	r = A2108_IBSP_I2CWrite(uiSlaveAddr, pBuff, 7);	 //+++++++++
	/*r = A2108_II2C_Write(uiSlaveAddr, pBuff, 7);	*/
/*	return(r);
} */
