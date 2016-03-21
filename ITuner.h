
//$Revision: 355 $ 
//$Date: 2008-04-21 16:12:24 -0400 (Mon, 21 Apr 2008) $
// 
#ifndef ITuner_h_h
	#define ITuner_h_h

	#include "avl2108.h"

	#define	rc_tuner_frequency_100kHz_addr			0x000005D0
	#define	rc_tuner_LPF_100kHz_addr				0x000005D2
	#define	rc_tuner_slave_addr_addr				0x000005D4
	#define	rc_tuner_max_LPF_100kHz_addr			0x000005D6
	#define	rc_tuner_LPF_margin_100kHz_addr			0x000005D8
	#define	rc_tuner_use_internal_control_addr		0x000005DA

	/// @endcond

	/// The structure to represent a Tuner object and it's controls.
	/// 
/*	struct AVL_Tuner
	{
		AVL_uint16 m_uiSlaveAddress;		///<tuner写地址 The Tuner slave address. It is the write address of the tuner device. i.e. it is 8 bits address and the last bit is the R/W flag which is zero. Although this data structure support 10 bits I2C address. The AVL2108 chip not supports it. 
		AVL_uint16 m_uiI2CBusClock_kHz;		///< The clock speed of the tuner dedicated I2C bus, in a unit of kHz.
		AVL_uint16 m_uiFrequency_100kHz;	///<将要锁定的频率The frequency tuner will lock to, in a unit of 100kHz.
		AVL_uint16 m_uiLPF_100kHz;			///< 低通滤波The LPF of the tuner.
		void *m_pParameters;				///< The tuner's customized parameters, such s BBGain etc.
		struct A2108_Chip * m_pAVLChip;		///< The AVL2108 chip connected with the Tuner.
		A2108_ErrorCode (* m_pInitializeFunc)(struct AVL_Tuner *);	 	///< Point to the initialize function.
		A2108_ErrorCode (* m_pGetLockStatusFunc)(struct AVL_Tuner *); 	///< Point to the GetLockStatus function.
		A2108_ErrorCode (* m_pDumpDataFunc)(AVL_puchar ucpData, AVL_puchar ucpSize, struct AVL_Tuner *); 	///< Point to the m_pDumpData function. This function is an optional function, used to dump debug information.
		A2108_ErrorCode (* m_pLockFunc)(struct AVL_Tuner *);			///< Point to Lock function.
	};
		*/
	/// Helper function used to calculate the right LPF based on the symbol rate.
	/// 
	/// @param uiSymbolRate_10kHz  The symbol rate, with a unit of 10kHz.
	/// @param pTuner  Point to a AVL_Tuner object. The function will fill the pTuner->uiLPF_100kHz
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Availink provide a implementation of this function in ITuner.c. Users can of cause ignore that implementation and provide their own calculation.
	A2108_ErrorCode A2108_ITuner_CalculateLPF(AVL_uint16 uiSymbolRate_10kHz, struct AVL_Tuner *pTuner);

	/// Load the internal tuner control code into the AVL2108.
	/// 
	/// @param pTunerControlData  A buffer holds the internal tuner control data, which is provided by Availink.
	/// @param pTuner  Point to a AVL_Tuner object.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks Availink provide some tuner control data with the chip for a few different tuners. If users need to control a new tuner, please contact the customer support department of Availink.
	A2108_ErrorCode A2108_ITuner_LoadControlData(AVL_puchar pTunerControlData, const struct AVL_Tuner *pTuner);

#endif
