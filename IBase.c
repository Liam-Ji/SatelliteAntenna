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


//$Revision: 914 $ 
//$Date: 2009-07-30 09:59:11 -0400 (Thu, 30 Jul 2009) $
// 
#include "bspdatadef.h"
#include "IBase.h"
#include "II2C.h"
#include "IBSP.h"
#include "IDVBSxRx.h"
#include "ITuner.h"
#include "II2CRepeater.h"
//#include "IBlindScan.h"
//#include "demod_patch.h"


   

/// @endcond
   A2108_ErrorCode A2108_IBase_SetPLL(struct A2108_PllConf * pPLLConf, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;

	r = A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (pll_r3), pPLLConf->m_r3); 	
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (pll_r2), pPLLConf->m_r2);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (pll_r1), pPLLConf->m_r1);	
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (pll_r4), pPLLConf->m_r4);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (pll_r5), pPLLConf->m_r5);

	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (pll_softvalue_en),   1);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (reset_register), 0);

	A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), (reset_register), 1);  //this is a reset, do not expect an ACK from the chip.

	pAVLChip->m_DemodFrequency_10kHz = pPLLConf->m_DmodFrequency_10kHz;
	pAVLChip->m_FecFrequency_10kHz =  pPLLConf->m_FecFrequency_10kHz;
	pAVLChip->m_MpegFrequency_10kHz = pPLLConf->m_MpegFrequency_10kHz;

	return(r);
}

A2108_ErrorCode A2108_IBase_DownloadFirmware(AVL_puchar pFirmwareData,  struct A2108_Chip * pAVLChip)
{
	AVL_uint32 uiSize, uiDataSize;
	AVL_uint32 i1=0;
	A2108_ErrorCode r;

	r = A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), core_reset_b_reg, 0);

	uiDataSize = DeChunk32(pFirmwareData);	 //根据ucDemodPatchData数组的前4个元素（0、1、2、3），求出uiDataSize=5516
	i1 = 4;
	while( i1 < uiDataSize )
	{
	/*第一次根据（4、5、6、7），求出uiSize为60 
	    二           */
		uiSize = DeChunk32(pFirmwareData+i1);  // uiSize中存放此次要发送的数据个数
		i1 += 4;  //目的是移动到要发送的元素下标
    	r |= A2108_II2C_Write((AVL_uint16)(pAVLChip->m_SlaveAddr), pFirmwareData+i1+1, (AVL_uint16)(uiSize+3));	//发送量再加三个地址字节
		i1 += 4 + uiSize;
	}
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), 0x00000000, 0x00003ffc);
	r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), core_ready_word_addr, 0x0000);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), error_msg_addr, 0x00000000);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), error_msg_addr+4, 0x00000000);

	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), core_reset_b_reg, 1);
	return r;
}
/*	检查base的初始化是否完成*/

A2108_ErrorCode A2108_IBase_GetStatus( struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[2]; 
	AVL_uint32 uiTemp=0;

	r = A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), core_reset_b_reg, &uiTemp);	// #define core_reset_b_reg		0x600000
	r |= A2108_II2C_Read16((AVL_uint16)(pAVLChip->m_SlaveAddr), core_ready_word_addr, (AVL_puint16)pBuff);
	                                                                             //	#define core_ready_word_addr	0x00000434
	if( (A2108_EC_OK == r) )
	{
		if( (0 == uiTemp) || (pBuff[0]!=0xa5) || (pBuff[1]!=0x5a) )	  //+++++++++++++bj25  +++位置颠倒       如果uiTemp为0或者pBuff[0]不是5a或pBuff[1]不是a5则初始化失败
		{
			r = A2108_EC_GeneralFail;
		}
	}
	return(r);
}



A2108_ErrorCode A2108_IBase_Initialize(struct A2108_PllConf *pPLLConf, AVL_puchar pInitialData,  struct A2108_Chip *pAVLChip )
{	
	A2108_ErrorCode r=0;

	r |= A2108_IBase_SetPLL(pPLLConf, pAVLChip );
	A2108_IBSP_Delay_ms(200);	 // this delay is critical 
	r |= A2108_IBase_DownloadFirmware(pInitialData, pAVLChip);

	return(r);
}


  
 /*	 
This function determines the present functional mode of the AVL2108. The AVL2108 can be in one of 
two functional modes, demodulator mode or blind scan mode. 
Parameters: 
pFunctionalMode; A pointer to an object to which the function writes the AVL2108 functional mode.  
pAVLChip;  A pointer to the A2108_Chip object for which the functional mode is being retrieved. 
功能模式：demodulator或者blind scan模式
把功能模式放入（* pFunctionalMode ）中 */
A2108_ErrorCode A2108_IBase_GetFunctionalMode(enum A2108_FunctionalMode * pFunctionalMode,  const struct A2108_Chip * pAVLChip)
{
	A2108_ErrorCode r;	
	AVL_uint16 uiTemp;
	r =  A2108_II2C_Read16((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_functional_mode_addr, &uiTemp);
	*pFunctionalMode = (enum A2108_FunctionalMode)(uiTemp & 0x1);	
	return r;
}


 /*The above code sends an ucOpCmd command to the AVL2108 chip. 
Parameters: 
 ucOpCmd;  The OP_RX_xxx command code as defined in IBase.h file.   
 pAVLChip;  A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.  
Remarks: 
This function is an 'internal' function. Availink doesn't recommend the user to call it directly. 
发送ucOpCmd命令给2108,     是一个“内部”函数，建议不要直接调用*/
A2108_ErrorCode A2108_IBase_SendRxOP(AVL_uchar ucOpCmd, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uchar pucBuff[2];
	AVL_uint16 uiTemp;
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semRx));	  // A semaphore used to protect the receiver command channel
	r |= A2108_IBase_GetRxOPStatus(pAVLChip);
	if( A2108_EC_OK == r )
	{
		pucBuff[0] = 0;	 //存放高位
		pucBuff[1] = ucOpCmd;//存放低位
		uiTemp = DeChunk16(pucBuff);
		r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rx_cmd_addr, uiTemp);   
	}	//往2108中写命令
	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semRx));
	return(r);
}


 /* The above code will verify that the last OP_Rx command has completed. 
   pAVLChip;  A pointer that points to an A2108_Chip object which used to tell the function which AVL2108 chip
  it is working on. 
Returns: 
A Return A2108_EC_OK if last command is finished; 
Return A2108_EC_Running if AVL2108 is still working on last command;
Return A2108_EC_I2CFail if there is an I2C problem.  
   核实最后一个OP_Rx 命令是否结束 */
A2108_ErrorCode A2108_IBase_GetRxOPStatus(const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uchar pBuff[2];
	r = A2108_II2C_Read((AVL_uint16)(pAVLChip->m_SlaveAddr), rx_cmd_addr, pBuff, 2);
	if( A2108_EC_OK == r )	 //读取的数组的第二个元素为0，说明命令执行完毕
	{
		if( 0 != pBuff[1] )		 
		{
			r = A2108_EC_Running;
		}
	}
	return(r);
}

