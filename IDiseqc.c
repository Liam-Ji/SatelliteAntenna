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



//$Revision: 355 $ 
//$Date: 2008-04-21 16:12:24 -0400 (Mon, 21 Apr 2008) $
// 
#include "IDiseqc.h"
#include "II2C.h"
#include "IBSP.h"

///@cond

enum A2108_Diseqc_OPStatus
{
	A2108_DOS_Uninitialized = 0,	///< = 0  Disec has not been initialized yet.
	A2108_DOS_Initialized   = 1,	///< = 1  Disec has been initialized yet.
	A2108_DOS_InContinuous  = 2,	///< = 2  Disec is in continuous mode.
	A2108_DOS_InTone        = 3,	///< = 3  Disec is in tone burst mode.
	A2108_DOS_InModulation  = 4		///< = 4  Disec is in modulation mode.
};

static enum A2108_Diseqc_OPStatus Diseqc_OP_Status = A2108_DOS_Uninitialized;

/// Check if it is safe to switch Diseqc operation mode.
/// 
/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
/// 
/// @return ::A2108_ErrorCode, 
/// Return ::A2108_EC_OK if it is OK to switch.
/// Return ::A2108_EC_Running if it is not safe to switch since the last transmit is not done yet.
/// Return ::A2108_EC_I2CFail if there is a I2C problem.
/*A2108_ErrorCode A2108_IDiseqc_IsSafeToSwitchMode( struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r = A2108_EC_OK;
	AVL_uint32 i1;
	switch( Diseqc_OP_Status )
	{
	case A2108_DOS_InModulation:
	case A2108_DOS_InTone:
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_st_addr, &i1);
		if( 1 != ((i1 & 0x00000040) >> 6) )	//check if the last transmit is done
		{
			r |= A2108_EC_Running;
		}
		break;
	case A2108_DOS_InContinuous:
	case A2108_DOS_Initialized:
		break;
	default:
		r |= A2108_EC_GeneralFail;
		break;
	}
	return(r);
}	 */

///@endcond

A2108_ErrorCode A2108_IDiseqc_Initialize( const struct A2108_Diseqc_Para * pDiseqcPara, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r=0;
	AVL_uint32 i1;
//	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	if( A2108_EC_OK == r )
	{
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_srst_addr, 1);

		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_samp_frac_n_addr, 200);		//2M=200*10kHz
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_samp_frac_d_addr, pAVLChip->m_DemodFrequency_10kHz);

		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tone_frac_n_addr, ((pDiseqcPara->m_ToneFrequency_kHz)<<1));
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tone_frac_d_addr, pAVLChip->m_DemodFrequency_10kHz*10);

		// Initialize the tx_control
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
		i1 &= 0x00000300;
		i1 |= 0x20;		//reset tx_fifo
		i1 |= ((AVL_uint32)(pDiseqcPara->m_TXGap) << 6);
		i1 |= ((AVL_uint32)(pDiseqcPara->m_TxWaveForm) << 4);
		i1 |= (1<<3);			//enable tx gap.
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);
		i1 &= ~(0x20);	//release tx_fifo reset
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);

		// Initialize the rx_control
		i1 = ((AVL_uint32)(pDiseqcPara->m_RxWaveForm) << 2);
		i1 |= (1<<1);	//active the receiver
		i1 |= (1<<3);	//enveolop high when tone present
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_cntrl_addr, i1);
		i1 = (AVL_uint32)(pDiseqcPara->m_RxTimeout);
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_msg_tim_addr, i1);

		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_srst_addr, 0);

		if( A2108_EC_OK == r )
		{
			Diseqc_OP_Status = A2108_DOS_Initialized;
		}
	}
//	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}

/*A2108_ErrorCode A2108_IDiseqc_ReadModulationData( AVL_puchar pucBuff, AVL_puchar pucSize, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
	AVL_uchar pucBuffTemp[4];
//	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));

	if( A2108_DOS_InModulation == Diseqc_OP_Status )
	{
		// In modulation mode
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_st_addr, &i1);
		if( 0x00000004 == (i1 & 0x00000004) )
		{
			*pucSize = (AVL_uchar)((i1 & 0x00000078)>>3);
			//Receive data
			for( i1=0; i1<*pucSize; i1++ )
			{
				r |= A2108_II2C_Read((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_fifo_map_addr, pucBuffTemp, 4);
				pucBuff[i1] = pucBuffTemp[3];
			}
		}
		else
		{
			r = A2108_EC_GeneralFail;
		}
	}
	else
	{
		r = A2108_EC_GeneralFail;
	}

//	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}	*/

/*A2108_ErrorCode A2108_IDiseqc_SendModulationData( const AVL_puchar pucBuff, AVL_uchar ucSize, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1, i2;
	AVL_uchar pucBuffTemp[8];

	if( ucSize>8 )
	{
		r = A2108_EC_MemoryRunout;
	}
	else
	{
//		r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));   
		r |= A2108_IDiseqc_IsSafeToSwitchMode(pAVLChip);
		if( A2108_EC_OK ==  r)
		{
			//reset rx_fifo
			r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_cntrl_addr, &i2);
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_cntrl_addr, (i2|0x01));
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_cntrl_addr, (i2&0xfffffffe));

			r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
			i1 &= 0xfffffff8;	//set to modulation mode and put it to FIFO load mode
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);

			//trunk address
			ChunkAddr(diseqc_tx_fifo_map_addr, pucBuffTemp);
			pucBuffTemp[3] = 0;
			pucBuffTemp[4] = 0;
			pucBuffTemp[5] = 0;
			for( i2=0; i2<ucSize; i2++ )
			{
				pucBuffTemp[6] = pucBuff[i2];

				r |= A2108_II2C_Write((AVL_uint16)(pAVLChip->m_SlaveAddr), pucBuffTemp, 7);
			}                           
			i1 |= (1<<2);  //start fifo transmit.
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);

			if( A2108_EC_OK == r )
			{
				Diseqc_OP_Status = A2108_DOS_InModulation;
			}
		}
///		r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	}

	return(r);
}
	  */
/*A2108_ErrorCode A2108_IDiseqc_GetTxStatus( struct A2108_Diseqc_TxStatus * pTxStatus, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
//	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	if( (A2108_DOS_InModulation == Diseqc_OP_Status) || (A2108_DOS_InTone == Diseqc_OP_Status) )
	{
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_st_addr, &i1);
		pTxStatus->m_TxDone = (AVL_uchar)((i1 & 0x00000040)>>6);
		pTxStatus->m_TxFifoCount = (AVL_uchar)((i1 & 0x0000003c)>>2);
	}
	else
	{
		r |= A2108_EC_GeneralFail;
	}
//	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}	 */

/*A2108_ErrorCode A2108_IDiseqc_GetRxStatus( struct A2108_Diseqc_RxStatus * pRxStatus, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	if( A2108_DOS_InModulation == Diseqc_OP_Status )
	{
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_st_addr, &i1);
		pRxStatus->m_RxDone = (AVL_uchar)((i1 & 0x00000004)>>2);
		pRxStatus->m_RxFifoCount = (AVL_uchar)((i1 & 0x000000078)>>3);
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_rx_parity_addr, &i1);
		pRxStatus->m_RxFifoParChk = (AVL_uchar)(i1 & 0x000000ff);
	}
	else
	{
		r |= A2108_EC_GeneralFail;
	}
//	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}
	*/
/*A2108_ErrorCode A2108_IDiseqc_SetLNBOut( AVL_uchar uiOut, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
	i1 &= 0xfffffdff;
	i1 |= ((uiOut & 0x1)<<9);
	r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);
	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}
	 */
/*A2108_ErrorCode A2108_IDiseqc_GetLNBOut( AVL_puchar puiOut, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
	i1 &= 0x00000200;
	*puiOut = (AVL_uchar)(i1>>9);
	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}
	  */
/*A2108_ErrorCode A2108_IDiseqc_SendTone( AVL_uchar ucTone, AVL_uchar ucCount, struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1, i2;
	AVL_uchar pucBuffTemp[8];
	if( ucCount>8 )
	{
		r = A2108_EC_MemoryRunout;
	}
	else
	{
		r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
		r |= A2108_IDiseqc_IsSafeToSwitchMode(pAVLChip);

		if( A2108_EC_OK == r )
		{
			//No data in the FIFO.
			r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
			i1 &= 0xfffffff8;  //put it into the FIFO load mode.
			if( 0 == ucTone )
			{
				i1 |= 0x01;
			}
			else
			{
				i1 |= 0x02;
			}
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);

			//trunk address
			ChunkAddr(diseqc_tx_fifo_map_addr, pucBuffTemp);
			pucBuffTemp[3] = 0;
			pucBuffTemp[4] = 0;
			pucBuffTemp[5] = 0;
			pucBuffTemp[6] = 1;

			for( i2=0; i2<ucCount; i2++ )
			{
				r |= A2108_II2C_Write((AVL_uint16)(pAVLChip->m_SlaveAddr), pucBuffTemp, 7);
			}

			i1 |= (1<<2);  //start fifo transmit.
			r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);
			if( A2108_EC_OK == r )
			{
				Diseqc_OP_Status = A2108_DOS_InTone;
			}
		}
		r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	}
	return(r);
}  */

/*A2108_ErrorCode A2108_IDiseqc_StartContinuous (struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	r |= A2108_IDiseqc_IsSafeToSwitchMode(pAVLChip);

	if( A2108_EC_OK == r )
	{
		//No data in FIFO
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
		i1 &= 0xfffffff8; 
		i1 |= 0x03;		//switch to continuous mosw
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);

		//start to send out wave
		i1 |= (1<<10);  
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);
		if( A2108_EC_OK == r )
		{
			Diseqc_OP_Status = A2108_DOS_InContinuous;
		}
	}
	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}	*/

/*A2108_ErrorCode A2108_IDiseqc_StopContinuous (struct A2108_Chip * pAVLChip )
{
	A2108_ErrorCode r;
	AVL_uint32 i1;
	r = A2108_IBSP_WaitSemaphore(&(pAVLChip->m_semDiseqc));
	if( A2108_DOS_InContinuous == Diseqc_OP_Status )
	{
		r |= A2108_II2C_Read32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, &i1);
		i1 &= 0xfffff3ff;
		r |= A2108_II2C_Write32((AVL_uint16)(pAVLChip->m_SlaveAddr), diseqc_tx_cntrl_addr, i1);
	}

	r |= A2108_IBSP_ReleaseSemaphore(&(pAVLChip->m_semDiseqc));
	return(r);
}
	*/