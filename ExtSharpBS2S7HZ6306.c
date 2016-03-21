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


//$Revision: 776 $ 
//$Date: 2009-05-18 13:48:32 -0400 (Mon, 18 May 2009) $
// 
#include "SharpBS2S7HZ6306.h"
#include "ExtSharpBS2S7HZ6306.h"
#include "II2CRepeater.h"
#include "IBSP.h"
#include "II2C.h"
#include "IBase.h"

A2108_ErrorCode SharpBS2S7HZ6306Regs_SetLPF( AVL_uint16 uiLPF_10kHz, struct SharpBS2S7HZ6306_Registers * pTunerRegs )
{
	uiLPF_10kHz /=100;
	if( uiLPF_10kHz <10 )
	{
		uiLPF_10kHz = 10;
	}
	if( uiLPF_10kHz>34 )
	{
		uiLPF_10kHz = 34;
	}
	pTunerRegs->m_ucLPF = (AVL_uchar)((uiLPF_10kHz-10)/2+3);  //LPF转换成字
	return(A2108_EC_OK);
}

    /*根据BBGain的值设定pTunerRegs中的m_ucRegData[0]*/
A2108_ErrorCode SharpBS2S7HZ6306_SetBBGain( enum SharpBS2S7HZ6306_BBGain BBGain, struct SharpBS2S7HZ6306_Registers * pTunerRegs )
{
	pTunerRegs->m_ucRegData[0] &=  ~(0x3<<5);
	pTunerRegs->m_ucRegData[0] |= ((AVL_uchar)(BBGain)<<5);	 //5、6位存放BBGain
	return(A2108_EC_OK);
}

 /*根据Current的值设定pTunerRegs中的m_ucRegData[2]*/
A2108_ErrorCode SharpBS2S7HZ6306_SetChargePump( enum SharpBS2S7HZ6306_PumpCurrent Current, struct SharpBS2S7HZ6306_Registers * pTunerRegs )
{
	pTunerRegs->m_ucRegData[2] &= ~(0x3<<5);	//5、6位置0
	pTunerRegs->m_ucRegData[2] |= ((AVL_uchar)(Current)<<5);  //data[2]的5、6位存放current
	return(A2108_EC_OK);
}



 /*根据载波频率uiFrequency，设定pTunerRegs*/
A2108_ErrorCode SharpBS2S7HZ6306_SetFrequency( AVL_uint16 uiFrequency_100kHz, struct SharpBS2S7HZ6306_Registers * pTunerRegs)
{
	AVL_uint16 P, N, A, DIV;	 /*data[3]中存放和BA2、BA1、BA0、PSC、DIV相关*/
								  /*data[0]放高频率字高五位*/
								  /*data[1]放高频率字低三位和低频率字*/

	if( uiFrequency_100kHz<9500 )       //<950Mhz,错误
	{
		return(A2108_EC_GeneralFail);
	}
	else if( uiFrequency_100kHz<9860 )  //[950，986) ,
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);	//把5、6、7为置0
		pTunerRegs->m_ucRegData[3] |= (0x5<<5);	    //5、7位置1
		P = 16;
		DIV = 1;
	}
	else if( uiFrequency_100kHz<10730 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x6<<5);	  //6、7位置1
		P = 16;
		DIV = 1;
	}
	else if( uiFrequency_100kHz<11540 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x7<<5);	 //5、6、7置1
		P = 32;
		DIV = 1;
	}
	else if( uiFrequency_100kHz<12910 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x1<<5);	  //5置1
		P = 32;
		DIV = 0;
	}
	else if( uiFrequency_100kHz<14470 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x2<<5);
		P = 32;
		DIV = 0;
	}
	else if( uiFrequency_100kHz<16150 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x3<<5);
		P = 32;
		DIV = 0;
	}
	else if( uiFrequency_100kHz<17910 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x4<<5);
		P = 32;
		DIV = 0;
	}
	else if( uiFrequency_100kHz<19720 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x5<<5);
		P = 32;
		DIV = 0;
	}
	else if( uiFrequency_100kHz<=21500 )
	{
		pTunerRegs->m_ucRegData[3] &= ~(0x7<<5);
		pTunerRegs->m_ucRegData[3] |= (0x6<<5);//6、7位置1
		P = 32;
		DIV = 0;
	}
	else
	{
		return(A2108_EC_GeneralFail);
	}

	A = (uiFrequency_100kHz/10)%P;	  // A中式频率的低位   0——31
	N = (uiFrequency_100kHz/10)/P;	  // N是频率的高位	  0-255

	pTunerRegs->m_ucRegData[3] &= ~(0x1<<4);   //4位psc为0
	if( P==16 )
	{
		pTunerRegs->m_ucRegData[3] |= (0x1<<4);	   // 如果p=16，4位psc置1 ，
	}		 		   /*上面两句把根据位4  P把PSC设置好*/

	pTunerRegs->m_ucRegData[3] &= ~(0x1<<1);    //第二位置0
	pTunerRegs->m_ucRegData[3] |= (AVL_uchar)(DIV<<1); //设置第二位DIV
	                   	/*上面两句把位2 DIV设置好*/	



	pTunerRegs->m_ucRegData[1] &= ~(0x1f<<0); // 把0、1、2、3、4位置零
	pTunerRegs->m_ucRegData[1] |= (AVL_uchar)(A<<0); 
						 /*把低频率字A用 data1的低五位表示*/	 
	pTunerRegs->m_ucRegData[1] &= ~(0x7<<5);   // 5、6、7位置零
	pTunerRegs->m_ucRegData[1] |= (AVL_uchar)(N<<5);
	 					 /*把高频率字N的低3位用data1的5、6、7位表示*/  
	pTunerRegs->m_ucRegData[0] &= ~(0x1f<<0);	//data0的0、1、2、3、4位置零
	pTunerRegs->m_ucRegData[0] |= (AVL_uchar)((N>>3)<<0);
	                      /*把高频率字的高5位用data0表示*/

	return(A2108_EC_OK);
}

A2108_ErrorCode SharpBS2S7HZ6306_CommitSetting(const struct AVL_Tuner * pTuner , struct SharpBS2S7HZ6306_Registers * pTunerRegs )
{
	A2108_ErrorCode r;
	pTunerRegs->m_ucRegData[0] &= 0x7f;	  //data0的7位置0
	pTunerRegs->m_ucRegData[2] |= 0x80;	  //data2的7位置1
//	pTunerRegs->m_ucRegData[2] |= 0x01;	  //data2的0位置1	 +++++++++++++++++++++++++++++++修改
	pTunerRegs->m_ucRegData[2] &= ~(0x7<<2);  //data2的2、3、4位置0
	pTunerRegs->m_ucRegData[3] &= ~(0x3<<2);  //data3的2、3位置0

	r = A2108_II2CRepeater_SendData((AVL_uchar)(pTuner->m_uiSlaveAddress), pTunerRegs->m_ucRegData, 4, pTuner->m_pAVLChip );
	if( r != A2108_EC_OK )
	{
		return(r);
	}
	pTunerRegs->m_ucRegData[2] |= (0x1<<2);	  //TM置1

	r |= A2108_II2CRepeater_SendData((AVL_uchar)(pTuner->m_uiSlaveAddress), (pTunerRegs->m_ucRegData)+2, 1, pTuner->m_pAVLChip );
	if( r != A2108_EC_OK )
	{
		return(r);
	}
	A2108_IBSP_Delay_ms(12);

/*根据LPF设置PD3、4、6、6*/
	r |= SharpBS2S7HZ6306Regs_SetLPF ((AVL_uint16)(pTuner->m_uiLPF_100kHz*10), pTunerRegs);
	pTunerRegs->m_ucRegData[2] |= ((((pTunerRegs->m_ucLPF)>>1)&0x1)<<3); /* PD4 */
	pTunerRegs->m_ucRegData[2] |= ((((pTunerRegs->m_ucLPF)>>0)&0x1)<<4); /* PD5 */
	pTunerRegs->m_ucRegData[3] |= ((((pTunerRegs->m_ucLPF)>>3)&0x1)<<2); /* PD2 */
	pTunerRegs->m_ucRegData[3] |= ((((pTunerRegs->m_ucLPF)>>2)&0x1)<<3); /* PD3 */

	r |= A2108_II2CRepeater_SendData((AVL_uchar)(pTuner->m_uiSlaveAddress), (pTunerRegs->m_ucRegData)+2, 2, pTuner->m_pAVLChip );

	return(r);
}

//*******************************************************************************************

A2108_ErrorCode ExtSharpBS2S7HZ6306_Initialize(struct AVL_Tuner * pTuner)
{
	A2108_ErrorCode r;
	r = A2108_II2C_Write16((AVL_uint16)(pTuner->m_pAVLChip->m_SlaveAddr), rc_tuner_slave_addr_addr, pTuner->m_uiSlaveAddress); //把tuner地址写入
	r |= A2108_II2C_Write16((AVL_uint16)(pTuner->m_pAVLChip->m_SlaveAddr), rc_tuner_use_internal_control_addr, 0);
	r |= A2108_II2C_Write16((AVL_uint16)(pTuner->m_pAVLChip->m_SlaveAddr), rc_tuner_LPF_margin_100kHz_addr, 0);	//为盲扫清除空余，对于外部驱动必须设置为0 clean up the LPF margin for blind scan. for external driver, this must be zero.
	r |= A2108_II2C_Write16((AVL_uint16)(pTuner->m_pAVLChip->m_SlaveAddr), rc_tuner_max_LPF_100kHz_addr, 320);	//为盲扫管理步长设置正确的LPF  set up the right LPF for blind scan to regulate the freq_step. This field should corresponding the flat response part of the LPF.
	r |= A2108_II2CRepeater_Initialize(pTuner->m_uiI2CBusClock_kHz, pTuner->m_pAVLChip); //I2C repeater初始化

	return(r);
}


A2108_ErrorCode ExtSharpBS2S7HZ6306_GetLockStatus(struct AVL_Tuner * pTuner )
{
	A2108_ErrorCode r;
	AVL_uchar ucTemp;	  //+++++++++++++++++修改 原来是16无符号型
	r = A2108_II2CRepeater_ReadData((AVL_uchar)(pTuner->m_uiSlaveAddress), (AVL_puchar)(&ucTemp), 1, pTuner->m_pAVLChip );
	if( A2108_EC_OK == r )
	{

		if( 0 == (ucTemp & 0x40) )
		{
			r = A2108_EC_Running ;
		}
	}
	return(r);
}

A2108_ErrorCode ExtSharpBS2S7HZ6306_Lock(struct AVL_Tuner * pTuner)
{
	A2108_ErrorCode r;
	struct SharpBS2S7HZ6306_Registers TunerRegs;
	struct SharpBS2S7HZ6306_TunerPara * pPara;

	TunerRegs.m_ucRegData[0] = 0;
	TunerRegs.m_ucRegData[1] = 0;
	TunerRegs.m_ucRegData[2] = 0;
	TunerRegs.m_ucRegData[3] = 0; 
   /*根据载波频率m_uiFrequency，设定TunerRegs*/
	r = SharpBS2S7HZ6306_SetFrequency(pTuner->m_uiFrequency_100kHz, &TunerRegs );
	if( 0 == pTuner->m_pParameters )	//use default values
	{	
	/*设定pTunerRegs中的m_ucRegData[2]的5、6位存放PC_360_694_Sharp*/
		r |= SharpBS2S7HZ6306_SetChargePump(PC_360_694_Sharp, &TunerRegs);//2, PC_360_694_Sharp是枚举常量	 < = 2 min +/- 360 uA; typical +/- 555 uA; Max +/- 694 uA
	/*设定pTunerRegs中的m_ucRegData[0]的5、6位存放Bbg-4-sharp*/
		r |= SharpBS2S7HZ6306_SetBBGain(Bbg_4_Sharp, &TunerRegs);		  //3,Bbg_4_Sharp为枚举常量
	}
	else		//use custom value
	{
		pPara = (struct SharpBS2S7HZ6306_TunerPara *)(pTuner->m_pParameters);
		r |= SharpBS2S7HZ6306_SetChargePump(pPara->m_ChargPump, &TunerRegs);
		r |= SharpBS2S7HZ6306_SetBBGain(pPara->m_BBGain, &TunerRegs);
	}
	r = SharpBS2S7HZ6306_CommitSetting(pTuner, &TunerRegs);
	return(r);
}
