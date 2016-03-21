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



//$Revision: 258 $ 
//$Date: 2008-02-06 14:05:26 -0500 (Wed, 06 Feb 2008) $
// 
#include "avl2108.h"
#include "ITuner.h"
#include "II2C.h"
//根据符号率算出LPF
A2108_ErrorCode A2108_ITuner_CalculateLPF(AVL_uint16 uiSymbolRate_10kHz, struct AVL_Tuner * pTuner)
{
	AVL_uint32 lpf = uiSymbolRate_10kHz;
	lpf *= 81;
	lpf /= 1000;
	lpf += 50;
	pTuner->m_uiLPF_100kHz = (AVL_uint16)lpf;
	return(A2108_EC_OK);
}
	 

	 //	下载控制代码到2108
/*A2108_ErrorCode A2108_ITuner_LoadControlData(AVL_puchar pTunerControlData, const struct AVL_Tuner * pTuner)
{
	AVL_uint32 uiSize, uiDataSize;
	AVL_uint32 i1;
	A2108_ErrorCode r = A2108_EC_OK;

	if(0 != pTunerControlData)
	{
		//load data into the chip
		uiDataSize = DeChunk32(pTunerControlData);
		i1 = 4;
		while( i1 < uiDataSize )
		{
			uiSize = DeChunk32(pTunerControlData+i1);
			i1 += 4;
			r |= A2108_II2C_Write((AVL_uint16)(pTuner->m_pAVLChip->m_SlaveAddr), pTunerControlData+i1+1, (AVL_uint16)(uiSize+3));
			i1 += 4 + uiSize;
		}
	}
	return(r);
}
	*/