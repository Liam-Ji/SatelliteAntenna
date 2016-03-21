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



///
/// @file 
/// @brief Declare functions for external Sharp BS2S7HZ6306 tuner control
///
//$Revision: 258 $ 
//$Date: 2008-02-06 14:05:26 -0500 (Wed, 06 Feb 2008) $
// 
#ifndef ExtSharpBS2S7HZ6306_h_h
	#define ExtSharpBS2S7HZ6306_h_h

	#include "avl2108.h"
    #include "ITuner.h"


	A2108_ErrorCode ExtSharpBS2S7HZ6306_Initialize(struct AVL_Tuner * pTuner);
	A2108_ErrorCode ExtSharpBS2S7HZ6306_GetLockStatus(struct AVL_Tuner * pTuner );
	A2108_ErrorCode ExtSharpBS2S7HZ6306_Lock(struct AVL_Tuner * pTuner);


#endif
