#ifndef __SHARPRECIVER_H__
#define __SHARPRECIVER_H__

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

#include "MyVariable.h"	
#include "avl2108.h" 	 
#include "bspdatadef.h"

extern struct A2108_Chip  A2108Chip;	//芯片
extern struct A2108_Chip * pA2108Chip ;
extern  struct AVL_Tuner Tuner;			//调谐器
extern struct AVL_Tuner * pTuner;

extern AVL_uint16  uiLockStatus;

#define true 1
#define false 0	  		


/*以下为tuner设置的一些参数*/
#define tuner_slave_address     0xC0            //The Tuner slave address. It is the write address of the tuner device. i.e. it is 8 bits address and the last bit is the R/W flag which is zero. Although this data structure support 10 bits I2C address. The AVL2108 chip not supports it. 
#define tuner_I2Cbus_clock      200             //The clock speed of the tuner dedicated I2C bus, in a unit of kHz.
#define Diseqc_Tone_Frequency   22              // The Diseqc bus speed in the unit of kHz. Normally, it should be 22kHz.
   
struct Signal_Level
{
	AVL_uint16 SignalLevel;
	AVL_int16 SignalDBM;
};


extern A2108_ErrorCode Initialize(struct A2108_Chip *pA2108Chip,struct AVL_Tuner * pTuner);
extern char SetSharpFreq (float tuner_freq,float  signal_symbolrate);
extern char SetSharpFreq1 (float tuner_freq,float  signal_symbolrate);
extern char SetSharpFreq_sub (float tuner_freq,float  signal_symbolrate);
extern char SetSharpFreq1_sub(float tuner_freq,float  signal_symbolrate);
extern A2108_ErrorCode CPU_Halt(struct A2108_Chip * pA2108Chip);


#endif
