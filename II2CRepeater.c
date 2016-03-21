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


//$Revision: 581 $ 
//$Date: 2008-10-23 13:31:49 -0400 (Thu, 23 Oct 2008) $
// 
#include "II2CRepeater.h"
#include "IBSP.h"
#include "II2C.h"

A2108_ErrorCode A2108_II2CRepeater_SendOP(AVL_puchar pBuff, AVL_uchar ucSize, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	const AVL_uint16 uiTimeDelay = 10;
	const AVL_uint16 uiMaxRetries = 20;
	AVL_uint32 i;

	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semI2CRepeater));

	i = 0;
	while (A2108_EC_OK != A2108_II2CRepeater_GetOPStatus(pAVLChip))	 //如果以前命令没有执行完毕则等待
	{
		if (uiMaxRetries < i++)
		{
			r |= A2108_EC_Running;	//等待时间到，但是还以前命令还没有完毕，状态r为running
			break;		   
		}
		A2108_IBSP_Delay(uiTimeDelay);
	}

	if ( A2108_EC_OK == r )	 //之前的命令执行完毕，则执行该命令
	{
		r = A2108_II2C_Write((AVL_uint16)(pAVLChip->m_SlaveAddr), pBuff, ucSize);  //之前的命令已经执行完毕的话，则执行该命令
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semI2CRepeater));
	return(r);
}


 /*	 
Initialize the I2C repeater. 
Parameters: 
I2CBusClock_kHz  The clock speed of the I2C bus between the tuner and the AVL2108 chip.   
pAVLChip  A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.  
 I2C Repeater初始化 I2CbusClock是 tuner和2108之间的时钟速度
 */
A2108_ErrorCode A2108_II2CRepeater_Initialize( AVL_uint16 I2CBusClock_kHz, struct A2108_Chip * pAVLChip )
{
	AVL_uchar pBuff[5];
	A2108_ErrorCode r;
	r = A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_i2cm_speed_kHz_addr, I2CBusClock_kHz);
	ChunkAddr(i2cm_cmd_addr+I2CM_CMD_LENGTH-2, pBuff);
	pBuff[3] = 0x01;
	pBuff[4] = OP_I2CM_INIT; //初始化命令分配到数组中
	r |= A2108_II2CRepeater_SendOP(pBuff, 5, pAVLChip);
	return r;
}


  /* Check if the last I2C repeater operation is finished. 
   检查最后一个repeater操作是否完成 */
A2108_ErrorCode A2108_II2CRepeater_GetOPStatus( const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[2];
	r = A2108_II2C_Read((AVL_uint16)(pAVLChip->m_SlaveAddr), i2cm_cmd_addr+I2CM_CMD_LENGTH-2, pBuff, 2);
	if ( A2108_EC_OK == r )
	{
		if ( pBuff[1] != 0 )
		{
			r = A2108_EC_Running;
		}
	}
	return(r);
}

/*Reads data back from the tuner via the I2C repeater.  This function is used to read data from a tuner  
which does not require a repeated start condition during its read cycle.  
Parameters: 
ucSlaveAddr  The slave address of the tuner device. Please note AVL2108 only support 7 bits slave address.   
ucBuff  The buffer which carry back the read data.   
uiSize  How many bytes the caller want to read back.   
pAVLChip  A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.  
Remarks: 
This function will trigger a I2C read operation. The read position (or device internal address) could be
 determined by calling A2108_II2CRepeater_SendData.   */
A2108_ErrorCode A2108_II2CRepeater_ReadData( AVL_uchar ucSlaveAddr, AVL_puchar pucBuff, AVL_uint16 uiSize, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[I2CM_RSP_LENGTH];
	AVL_uint16 uiTimeOut;
	const AVL_uint16 uiTimeOutTh = 10;
	const AVL_uint32 uiTimeDelay = 100;  //100 ms 

	if ( uiSize > I2CM_RSP_LENGTH )
	{
		return(A2108_EC_GeneralFail);
	}
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semI2CRepeater_r));

	ChunkAddr(i2cm_cmd_addr+I2CM_CMD_LENGTH-4, pBuff);	 //	从地址
	pBuff[3] = 0x0;
	pBuff[4] = (AVL_uchar)uiSize;
	pBuff[5] = ucSlaveAddr;
	pBuff[6] = OP_I2CM_READ;
	r |= A2108_II2CRepeater_SendOP(pBuff, 7, pAVLChip);
	if ( A2108_EC_OK == r )
	{
		uiTimeOut = 0;
		while ( A2108_EC_OK != A2108_II2CRepeater_GetOPStatus(pAVLChip) )  //发出的命令没有执行完毕的话，继续执行
		{
			if((++uiTimeOut) >= uiTimeOutTh) //如果时间到还没有执行完毕，则失败
			{
				r |= A2108_EC_TimeOut;
				r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semI2CRepeater_r));
				return(r);
			}
			A2108_IBSP_Delay_ms(uiTimeDelay);
		}
		r |= (A2108_II2C_Read((AVL_uint16)(pAVLChip->m_SlaveAddr), i2cm_rsp_addr, pucBuff, uiSize));
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semI2CRepeater_r));
	return(r);
}


/* Write data to the tuner via the I2C repeater.  
Parameters: 
ucSlaveAddr;  The slave address of the tuner device. Please note AVL2108 only support 7 bits slave address.   
ucBuff;  The buffer which hold the send out data.   
uiSize;  How many bytes the caller want to write.   
pAVLChip; A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108    
        chip it is working on.  
Remarks: 
The tuner register address to which data is being written must be  included in the input buffer ucBuff. 
For many tuners, the register address is specified as the first byte to be transmitted, and thus should 
be placed in the first byte entry of ucBuff.  However, the user must refer to the datasheet for the specific
tuner being used to ensure that they place the register address in the correct location within the input buffer. 
 
 经由I2C repeater写数据到tuner，要发送的数组pucBuff大小为uiSize，*/
A2108_ErrorCode A2108_II2CRepeater_SendData(  AVL_uchar ucSlaveAddr, const AVL_puchar pucBuff, AVL_uint16 uiSize, struct A2108_Chip * pAVLChip )
{
	AVL_uchar pBuff[I2CM_CMD_LENGTH+3];	 //	#define I2CM_CMD_LENGTH   0x14
		AVL_uint16 i1, i2;
	AVL_uint16 cmdSize;

	if ( uiSize>I2CM_CMD_LENGTH-3 )
	{
		return(A2108_EC_GeneralFail);
	}

	cmdSize = ((3+uiSize)%2)+3+uiSize;	  /* How many bytes nee send to AVL2108 through i2c interface */
	ChunkAddr(i2cm_cmd_addr+I2CM_CMD_LENGTH-cmdSize, pBuff);

	i1 = 3+((3+uiSize)%2);	  /* skip one byte if the uisize+3 is odd*/

	for ( i2=0; i2<uiSize; i2++ )
	{
		pBuff[i1++] = pucBuff[i2];
	}
	pBuff[i1++] = (AVL_uchar)uiSize;
	pBuff[i1++] = ucSlaveAddr;
	pBuff[i1++] = OP_I2CM_WRITE;

	return(A2108_II2CRepeater_SendOP(pBuff, (AVL_uchar)(cmdSize+3), pAVLChip));
}

