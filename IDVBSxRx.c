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
#include "IDVBSxRx.h"
#include "IBase.h"
#include "II2C.h"
#include "IBSP.h"

/*Initialize the demodulator.   
Parameters: 
pAVLChip;  A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.   
Returns:  A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return A2108_EC_I2CFail if there is an I2C problem.  
Remarks:  This function must be called first before all other functions declared in  this interface. 
            这个函数必须在这一层声明的其他函数之前被调用  */
A2108_ErrorCode A2108_IDVBSxRx_Initialize(const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r=0;
	// Set clk to match the PLL 
	r = A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_int_dmd_clk_MHz_addr,  pAVLChip->m_DemodFrequency_10kHz);
	r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_int_fec_clk_MHz_addr, pAVLChip->m_FecFrequency_10kHz);
	r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_int_mpeg_clk_MHz_addr, pAVLChip->m_MpegFrequency_10kHz);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_format_addr, 1);

	return(r);
}

A2108_ErrorCode A2108_IDVBSxRx_SetFreqSweepRange(AVL_uint16 uiFreqSweepRange_10kHz, const struct A2108_Chip * pAVLChip )
{
	if(uiFreqSweepRange_10kHz > 500)
	{
		uiFreqSweepRange_10kHz = 500;
	}
	return A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_int_carrier_freq_half_range_MHz_addr, uiFreqSweepRange_10kHz);
}

/*A2108_ErrorCode A2108_IDVBSxRx_SetChannelLockMode( struct A2108_Channel * psChannel, enum A2108_LockMode enumChannelLockMode )
{
	if(enumChannelLockMode == A2108_LOCK_MODE_FIXED)
	{
		psChannel->m_Flags &= ~CI_FLAG_LOCK_MODE_BIT_MASK;
	}
	else
	{
		psChannel->m_Flags |= CI_FLAG_LOCK_MODE_BIT_MASK;
	}

	return(A2108_EC_OK);
}
	  */

 /* This function locks to a channel using the parameters specified in the psChannel input parameter.  This function may only be called when 
the AVL2108 is in the demodulator functional mode.只有在demodulator模式才调用此函数，否则提示错误*/
A2108_ErrorCode A2108_IDVBSxRx_LockChannel( struct A2108_Channel * sChannel, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 IQ;
	AVL_uint32 autoIQ_Detect;
	AVL_uint16 Standard;
	enum A2108_LockMode LockMode;	//fixed或者adaptive模式
	enum A2108_FunctionalMode enumFunctionalMode;

	r = A2108_EC_OK;

	r |= A2108_IBase_GetFunctionalMode(&enumFunctionalMode, pAVLChip);
	if(enumFunctionalMode == A2108_FunctMode_Demod)		  //demodulator模式
	{
		if( (sChannel->m_uiSymbolRate_Hz > 800000) && (sChannel->m_uiSymbolRate_Hz < 50000000) ) //800K<波特率<50M
		{
			LockMode = (enum A2108_LockMode)((sChannel->m_Flags & CI_FLAG_LOCK_MODE_BIT_MASK) >> CI_FLAG_LOCK_MODE_BIT);

			if(LockMode == A2108_LOCK_MODE_ADAPTIVE)  //lockmode=1，adaptive模式
			{
				r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_lock_mode_addr, 1);
				if(sChannel->m_uiSymbolRate_Hz < 3000000) //符号率小于3M，率扫描范围3MHz
				{
					r |= A2108_IDVBSxRx_SetFreqSweepRange(300, pAVLChip);
				}
				else
				{
					r = A2108_IDVBSxRx_SetFreqSweepRange(500, pAVLChip); //符号率大于或等于3M，扫描范围5MHz	 
				}
			}
			else   //fixed模式
			{
				r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_lock_mode_addr, 0);
			}

			IQ = ((sChannel->m_Flags) & CI_FLAG_IQ_BIT_MASK)>>CI_FLAG_IQ_BIT; //是否IQ交换

			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_specinv_addr, IQ);
			Standard = (AVL_uint16)(((sChannel->m_Flags) & CI_FLAG_DVBS2_BIT_MASK)>>CI_FLAG_DVBS2_BIT);//符号标准
			autoIQ_Detect = (((sChannel->m_Flags) & CI_FLAG_IQ_AUTO_BIT_MASK)>>CI_FLAG_IQ_AUTO_BIT); //手动还是自动配置IQ交换

			if((Standard == CI_FLAG_DVBS2_UNDEF) || (autoIQ_Detect == 1))  //不知道信号标准 或者 自动IQ交换
			{
				Standard = 0x14;
			}
			r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_decode_mode_addr, Standard);
			r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_iq_swap_mode_addr, (AVL_uint16)autoIQ_Detect);
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_int_sym_rate_Hz_addr, sChannel->m_uiSymbolRate_Hz);
			r |= A2108_IBase_SendRxOP(OP_RX_INIT_GO, pAVLChip );  //发送OP_RX_INIT_GO命令给2108
		}
		else
		{
			r = A2108_EC_GeneralFail;	//符号率应该在800K和3M之间
		}
	}
	else
	{
		r = A2108_EC_GeneralFail;
	}

	return(r);
}

 /* Check the current AVL2108 lock status.  
Parameters: 
 puiLockStatus;  Carries back the lock status. If the AVL2108 has locked the signal,
puiLockStatus will be 1, otherwise, it will be 0. pAVLChip;  A pointer that points
 to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.  
 Returns: 
A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return A2108_EC_I2CFail if there is an I2C problem.  
 Remarks: 
This function should be called after the A2108_IDVBSxRx_LockChannel has been called. 
 puiLockStatus用于返回状态信息，为1表示锁定信号，否则为0
 这个函数应该在LockChannel函数之后被调用 */
A2108_ErrorCode A2108_IDVBSxRx_GetLockStatus( AVL_puint16 puiLockStatus, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	r = A2108_II2C_Read16((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_fec_lock_addr, puiLockStatus);
	return(r);//从2108的内部地址rs-fec-lock-addr中读取数据放到puiLockStatus指针指向的空间
}


/*This function resets the AVL2108 internal BER/PER statistics accumulation associated with the GetBER 
and GetPER functions. This function does not reset the BER statistics accumulation that is associated with
 the GetDVBSBER function.   This function may only be called while the AVL2108 is in the demodulator functional
 mode.  If the AVL2108 is in the blind scan functional mode, the function returns the error code 
A2108_EC_GeneralFail. 
 这个函数重设和GetBER and GetPER函数有关的数据集 
 不重设和GetDVBSBER函数有关的数据集
 只有在demodulator模式此函数才能被调用*/
/*A2108_ErrorCode A2108_IDVBSxRx_ResetErrorStat( struct A2108_Chip * pAVLChip )
{
	enum A2108_FunctionalMode enumFunctionalMode;
	A2108_ErrorCode r;

	r = A2108_IBase_GetFunctionalMode(&enumFunctionalMode, pAVLChip);  //把功能模式值发那个如enmFunctionMode

	if(enumFunctionalMode == A2108_FunctMode_Demod)	//如果是demodulator模式
	{
		r |= A2108_IBase_SendRxOP( OP_RX_RESET_BERPER, pAVLChip );//发送OP_RX_RESET_BERPER命令
	}
	else
	{
		r = A2108_EC_GeneralFail;
	}

	return(r);   
}
   */
 /*Set the polarity of the RF AGC. This value should be set according to  the specific tuner voltage/gain characteristics.
设置RF AGC的极性	，值的的设定必须依据具体的调谐器电压/增益描述
Parameters:  enumAGCPola  The polarity of the RF AGC. Refer to A2108_RFAGCPola. 
pAVLChip  A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.   
 Returns:A2108_ErrorCode, Return A2108_EC_OK if everything is OK; Return  A2108_EC_I2CFail if there is a I2C problem.  */
A2108_ErrorCode A2108_IDVBSxRx_SetRFAGCPola( enum A2108_RfagcPola enumAGCPola, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r=0;
	AVL_uint32 uiTemp;
	uiTemp = (AVL_uint32)enumAGCPola;
	r = A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_rfagc_pol_addr, uiTemp);	 //#define rc_rfagc_pol_addr	0x0000043c
	return(r);
}

A2108_ErrorCode A2108_IDVBSxRx_SetMpegMode( const struct A2108_MpegInfo * pMpegMode, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r=0;
	AVL_uint32 uiTemp1;
	AVL_uint16 uiTemp2;
	uiTemp1 = (AVL_uint32)(pMpegMode->m_mpf);	 //图像数据传输格式
	r = A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_mpeg_mode_addr, uiTemp1);
	uiTemp2 = (AVL_uint16)(pMpegMode->m_mpm);	 //图像传输模式
	r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_mpeg_serial_addr, uiTemp2);
	uiTemp2 = (AVL_uint16)(pMpegMode->m_mpcm);	  //时钟模式
	r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_mpeg_posedge_addr, uiTemp2);
	return(r);
}


 /*This function gets the RF signal level. 
Parameters: 
puiRFSignalLevel;  Carries back the signal level as a value ranging from 0 to 65535 with 0 corresponding
 to the weakest signal level and 65535 corresponding to the strongest signal level. 
pAVLChip;  A pointer to the A2108_Chip object for which the signal level is being read.  
 Remarks: 
This function returns a value in the range 0 to 65535, with 0 corresponding to the weakest signal and 65535 
corresponding to the strongest signal. The respective signal level in units of dbm varies with 
the tuner configuration and board design.   */
/*A2108_ErrorCode A2108_IDVBSxRx_GetSignalLevel(AVL_puint16 puiRFSignalLevel , const struct A2108_Chip * pAVLChip )
{
	AVL_uint32 uiData;
	AVL_uint32 uiPolarity;
	AVL_uint16 uiSignalLevel;
	A2108_ErrorCode r;

	r = A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), (rx_aagc_gain), &uiData);
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_rfagc_pol_addr, &uiPolarity);

	if( A2108_EC_OK == r )
	{
		uiData += 0x800000;
		uiData &= 0xffffff;	
		uiSignalLevel = (AVL_uint16)(uiData>>8);
		*puiRFSignalLevel = uiSignalLevel;
	}
	return(r);
}
  */
/*A2108_ErrorCode A2108_IDVBSxRx_GetScatterData( AVL_puchar ucpData, AVL_puint16 puiSize, const struct A2108_Chip * pAVLChip )
{
	AVL_uint16 ucTemp1, ucTemp2;
	A2108_ErrorCode r;
	r = A2108_II2C_Read16((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_ScatterData_rdy_addr, &ucTemp1);
	r |= A2108_II2C_Read16((AVL_uint16)(pAVLChip->m_SlaveAddr), scatter_data_addr, &ucTemp2);
	if( (A2108_EC_OK != r) || (0 == ucTemp1) )
	{
		return(A2108_EC_Running);
	}
	if( ucTemp2>(*puiSize) )
	{
		ucTemp2 = (*puiSize);
	}
	else
	{
		(*puiSize) = ucTemp2;
	}

	r = A2108_II2C_Read((AVL_uint16)(pAVLChip->m_SlaveAddr), scatter_data_addr+2, ucpData, (AVL_uint16)(ucTemp2<<1)); //both i and q
	ucTemp1 = 0;
	r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_ScatterData_rdy_addr, ucTemp1);
	return(r);
}
	*/

 /*Reads back the current SNR estimate. 
 Parameters: 
puiSNR_db;  Carries back the current estimated SNR value. The value is scaled by 100. For example, 
reading back decimal 2578 means the SNR estimate is 25.78 db. pAVLChip;  A pointer to the A2108_Chip 
object for which the SNR is being read.  
Remarks: 
It may take a few seconds for the chip to calculate a stable SNR value after FEC lock. This function 
returns an SNR value of 0 before a stable SNR value is calculated.   */
/*A2108_ErrorCode A2108_IDVBSxRx_GetSNR( AVL_puint32 puiSNR_db, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	r = A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_int_SNR_dB_addr, puiSNR_db);
	if( (*puiSNR_db) > 10000 )
	{
		// Not get stable SNR value yet.
		*puiSNR_db = 0;
	}
	return(r);
} */

 /*	Read back the current PER calculation result. 
 puiPER;  Carries back the current PER value. The value is scaled by 1e-9. For example, reading back 
 decimal 123456 means the PER value is 0.000123456.  pAVLChip;  A pointer that points to an A2108_Chip 
 object which is used to tell the function which AVL2108 chip it is working on.  
 读取当前的PER计算结果，puiPER存放当前PER值 */
/*A2108_ErrorCode A2108_IDVBSxRx_GetPER( AVL_puint32 puiPER, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	r = A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_uint_PER_addr, puiPER);
	return(r);
}
   */

/*	  This function reports an estimated BER when a DVBS input signal is applied to the receiver. 
 The DVBS bit error rate estimate is performed after Viterbi decoding but before Reed Solomon decoding. 
  The function does not require the input of a known test pattern to the 
receiver, and thus can be used to estimate BER for on-air DVBS 
signals.   
  */
/*A2108_ErrorCode A2108_IDVBSxRx_GetDVBSBER( AVL_puint32 puiBER, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;

	r = A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_dvbs_ber_addr, puiBER);
	if( 0xffffffff == (*puiBER) )
	{
		r |= A2108_EC_GeneralFail;
	}
	return(r);
}
	 */

 /*Get the RF frequency offset. This is typically used to adjust the tuner to center the signal in the 
 tuner bandwidth for best performance. 
Parameters: 
 piRFOffset_100kHz;  Carries back the offset of the RF frequency in the unit of 100kHz. For example,
  if the signal RF frequency is 1000M, the tuner is at 998M, and the rc_specinv  register is set 
  consistent to the hardware, this value will be -20 (20 x 100kHz  = -2M).  If the rc_specinv is 
  opposite to the hardware, the offset will be 20. For the setup of the rc_specinv register, please refer 
  to the SDK function A2108_IBlindScan_Scan for more information.  pAVLChip;  A pointer that points to 
  an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on. 	    
Remarks: 
Call this function after the AVL2108 chip locked the input signal.    */
/*A2108_ErrorCode A2108_IDVBSxRx_GetRFOffset( AVL_pint16 piRFOffset_100kHz, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	r = A2108_II2C_Read16((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_int_carrier_freq_100kHz_addr, (AVL_puint16)piRFOffset_100kHz);
	return(r);
}
  */

/*Get the current locked signal information.
Parameters: 
pSignalInfo;  Carries back the detected signal information. Refer to A2108_Signal_Info.
pAVLChip;  A pointer that points to an A2108_Chip object which is used to tell the function which AVL2108 chip it is working on.  
 结构体p_SignalInfo返回信号信息 */
/*A2108_ErrorCode A2108_IDVBSxRx_GetSignalInfo( struct A2108_SignalInfo * pSignalInfo, const struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r = A2108_EC_OK;
	AVL_uint32 uiTemp;
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_pilot_addr, &uiTemp);
	pSignalInfo->m_pilot = (enum A2108_Pilot)(uiTemp);
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_code_rate_addr, &uiTemp);
	pSignalInfo->m_coderate = (enum A2108_FecRate)(uiTemp);
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rs_modulation_addr, &uiTemp);
	pSignalInfo->m_modulation = (enum A2108_ModulationMode)(uiTemp);
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), rx_Rolloff_addr, &uiTemp);
	pSignalInfo->m_rolloff = (enum A2108_RollOff)((uiTemp>>22) & 0x03);
	return(r);
}	*/

/*A2108_ErrorCode A2108_IDVBSxRx_SetDishPointingMode( AVL_uchar ucMode, const struct A2108_Chip *pAVLChip )
{
	A2108_ErrorCode r = A2108_EC_OK;
	enum A2108_FunctionalMode enumFunctionalMode;

	r |= A2108_IBase_GetFunctionalMode(&enumFunctionalMode, pAVLChip);
	if(enumFunctionalMode == A2108_FunctMode_Demod)
	{
		if(ucMode == 1)
		{
			r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_aagc_acq_gain_addr, 12);
			r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_dishpoint_mode_addr, 1);
		}
		else
		{
			r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rc_aagc_acq_gain_addr, 10);
			r |= A2108_II2C_Write16((AVL_uint16)(pAVLChip->m_SlaveAddr), rp_dishpoint_mode_addr, 0);
		}
	}
	else
	{
		r = A2108_EC_GeneralFail;
	}

	return(r);
}	   */
