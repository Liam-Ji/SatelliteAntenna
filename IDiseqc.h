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
/// @brief Declares the functions for Diseqc operations.
/// @details There are some limitation of the Diseqc operation. First, it is a half duplex bus. In one moment, you can either send or read. Second, the maximum size of each transmission is 8 bytes.
/// The diseqc can work in different modes, such as modulation mode, tone mode and continuous  mode. There are functions corresponding to these modes, if  user changes the operation mode by calling functions
/// belong to another mode, the previous operation will be cancelled if it has not been finished yet.
///
//$Revision: 355 $ 
//$Date: 2008-04-21 16:12:24 -0400 (Mon, 21 Apr 2008) $
// 
#ifndef IDiseqc_h_h
	#define IDiseqc_h_h

	#include "avl2108.h"


	#define diseqc_tx_cntrl_addr			0x00700000
	#define diseqc_tone_frac_n_addr			0x00700004
	#define diseqc_tone_frac_d_addr			0x00700008
	#define diseqc_tx_st_addr				0x0070000c
	#define diseqc_rx_parity_addr			0x00700010
	#define diseqc_rx_msg_tim_addr			0x00700014
	#define diseqc_rx_st_addr				0x00700018
	#define diseqc_rx_cntrl_addr			0x0070001c
	#define diseqc_srst_addr				0x00700020
	#define diseqc_bit_time_addr			0x00700024
	#define diseqc_samp_frac_n_addr			0x00700028
	#define diseqc_samp_frac_d_addr			0x0070002c
	#define diseqc_bit_decode_range_addr	0x00700030
	#define diseqc_rx_fifo_map_addr			0x00700040
	#define diseqc_tx_fifo_map_addr			0x00700080

	///@endcond

	///
	/// When transmit data in Tone0 or Tone1 mode, there is a gap between two tones. This enum define the gap length.
	enum A2108_Diseqc_TxGap
	{
		A2108_DTXG_15ms = 0,		///< = 0  The gap is 15 ms.
		A2108_DTXG_20ms = 1,		///< = 1  The gap is 20 ms.
		A2108_DTXG_25ms = 2,		///< = 2  The gap is 25 ms.
		A2108_DTXG_30ms = 3			///< = 3  The gap is 30 ms.
	};

	///
	/// Defines the transmit mode.
	enum A2108_Diseqc_TxMode
	{
		A2108_DTM_Modulation = 0,			///< = 0  Use modulation mode.
		A2108_DTM_Tone0 = 1,				///< = 1  Send out tone 0.
		A2108_DTM_Tone1 = 2,				///< = 2  Send out tone 1.
		A2108_DTM_Continuous = 3			///< = 3  Continuous send out pulses.
	};

	///
	/// The Diseqc out put could have two different wave modes.
	enum A2108_Diseqc_WaveFormMode
	{
		A2108_DWM_Normal = 0,			///< = 0  The normal wave mode.
		A2108_DWM_Envelope = 1			///< = 1  The wave is put into envelopes.
	};

	///
	/// After data were transmitted to the Diseqc device, the Diseqc device may return some data. 
	/// Diseqc will open the input FIFO for ceratin time to receive the data and then close the input FIFO. Data will be abandoned 
	/// if they are send too late.
	enum A2108_Diseqc_RxTime
	{
		A2108_DRT_150ms = 0,		///< = 0  Wait 150 ms and close the input FIFO.
		A2108_DRT_170ms = 1,		///< = 1  Wait 170 ms and close the input FIFO.
		A2108_DRT_190ms = 2,		///< = 2  Wait 190 ms and close the input FIFO.
		A2108_DRT_210ms = 3			///< = 3  Wait 210 ms and close the input FIFO.
	};

	/// Hold the Diseqc configure parameters to setup the Diseqc component.
	/// 
	struct A2108_Diseqc_Para
	{
		AVL_uint16 m_ToneFrequency_kHz;			///< The Diseqc bus speed in the unit of kHz. Normally, it should be 22kHz. 
		enum A2108_Diseqc_TxGap m_TXGap;				///< transmit gap.
		enum A2108_Diseqc_WaveFormMode m_TxWaveForm;	///< Transmitter Wave format.
		enum A2108_Diseqc_RxTime m_RxTimeout;			///< Time window size of the receiver.
		enum A2108_Diseqc_WaveFormMode m_RxWaveForm;	///< Receiver Wave format.
	};

	/// Hold the Diseqc Transmitter status.
	/// 
	struct A2108_Diseqc_TxStatus
	{
		AVL_uchar m_TxDone;				///< 1 if transmit finished. 0 if Diseqc is still in transmitting.
		AVL_uchar m_TxFifoCount;		///< How many bytes are still in the transmitter's FIFO
	};

	/// Hold the Diseqc configure parameters to setup the Diseqc component.
	/// 
	struct A2108_Diseqc_RxStatus
	{
		AVL_uchar m_RxFifoCount;		///< How many bytes are already in the receiver's FIFO.
		AVL_uchar m_RxFifoParChk;		///< The parity check result byte of the received data. There will be one check bit for each byte in the receive FIFO starting with LSB. The upper bits without corresponding data are undefined. If a bit is 1, the corresponding byte in the FIFO has good parity. For example, if three bytes are in the FIFO, and the parity check value is 0x03 (value of bit 2 is zero), then the first and the second bytes in the receive FIFO are good. The third byte had bad parity. 
		AVL_uchar m_RxDone;				///< 1 if the receiver window is turned off, 0 if it is still in receiving state.
	};

	/// Initialized the Diseqc component according to \a pDiseqcPara.
	/// 
	/// @param pDiseqcPara  Input, pass the Diseqc setup parameters to the function.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IDiseqc_Initialize( const struct A2108_Diseqc_Para * pDiseqcPara, struct A2108_Chip * pAVLChip );

	/// Read data back from the Diseqc input FIFO.
	/// 
	/// @param pucBuff The buffer used to carry back data read from the Diseqc component.
	/// @param pucSize As input, tells the function how many bytes the caller likes to read back. the maximum is 8; As output, tells the caller how many bytes are actually read back.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK. 
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_GeneralFail if Diseqc is not in the modulation mode or the Diseqc is still receiving the data.
	/// @remarks Availink recommends to call ::A2108_IDiseqc_GetRxStatus to get the rx status before call this function. 

	A2108_ErrorCode A2108_IDiseqc_StartContinuous (struct A2108_Chip * pAVLChip );

	/// Stop to send out continuous  22k Diseqc wave.
	/// 
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IDiseqc_StopContinuous (struct A2108_Chip * pAVLChip );


#endif
