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
#include "bspdatadef.h"
#include "avl.h"
#include "avl2108.h"
#include "IBSP.h"
#include "II2C.h"

A2108_ErrorCode InitA2108ChipObject(struct A2108_Chip * pA2108ChipObject, enum A2108_ChipSlaveAddr enumSlaveAddress)
{																	 
	A2108_ErrorCode r;
	pA2108ChipObject->m_SlaveAddr = enumSlaveAddress;  //把	enumSlaveAddress  0x0c付给芯片地址
	pA2108ChipObject->m_StdBuffIndex = 0;
	r = A2108_IBSP_InitSemaphore(&(pA2108ChipObject->m_semRx));
	r |= A2108_IBSP_InitSemaphore(&(pA2108ChipObject->m_semI2CRepeater));
	r |= A2108_IBSP_InitSemaphore(&(pA2108ChipObject->m_semI2CRepeater_r));
	r |= A2108_IBSP_InitSemaphore(&(pA2108ChipObject->m_semDiseqc));
	r |= A2108_II2C_Initialize(); // there is a internal protection to assure it will be initialized only once.
	   //如果A2108-IBSP-InitSemaphore()有一个返回的结果非0，那么r就是非0
	return (r);
}
