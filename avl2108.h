
//$Revision: 914 $ 
//$Date: 2009-07-30 09:59:11 -0400 (Thu, 30 Jul 2009) $
// 
#ifndef avl2108_h_h
	#define avl2108_h_h

	#include "avl.h"

	#define CI_FLAG_IQ_BIT                  0x00        /// bit 0
	#define CI_FLAG_IQ_BIT_MASK             0x01        /// one bit
	#define CI_FLAG_IQ_NO_SWAPPED           0x00		/// IQ is not swapped
	#define CI_FLAG_IQ_SWAPPED              0x01		/// IQ is swapped

	#define CI_FLAG_DVBS2_BIT                0x02		/// bit 4, 3, 2
	#define CI_FLAG_DVBS2_BIT_MASK           0x1c		/// three bits 
	#define CI_FLAG_DVBS 	                 0x00		/// DVB-S standard
	#define CI_FLAG_DVBS2                    0x01		/// DVB-S2 standard
	#define CI_FLAG_DVBSDTV                  0x02		/// DirectTV DVBs
	#define CI_FLAG_DVBSDTV_AMC              0x03		/// DirectTV DVBS2
	#define CI_FLAG_DVBS2_UNDEF              0x04       /// Do not know the standard      

	#define CI_FLAG_IQ_AUTO_BIT				0x05		/// bit 5
	#define CI_FLAG_IQ_AUTO_BIT_MASK		0x20        /// one bit
	#define CI_FLAG_IQ_AUTO_BIT_OFF			0x00        /// Do not do auto swapp IQ and try again
	#define CI_FLAG_IQ_AUTO_BIT_AUTO		0x01        /// Auto swapp IQ and try again

	#define CI_FLAG_LOCK_MODE_BIT			0x06		/// bit 6 controls channel lock mode
	#define CI_FLAG_LOCK_MODE_BIT_MASK		0x40		/// Lock mode bit mask

	#define TCS_free						0x0
	#define TCS_locked						0x1
	#define TCS_I2CM_busy					0x10
	#define TCS_wrong_para					0x20
	#define TCS_running						0xff

	typedef AVL_uchar A2108_ErrorCode;		///< Defines the error code. \sa A2108_EC_OK, A2108_EC_GeneralFail, A2108_EC_I2CFail, A2108_EC_TimeOut, A2108_EC_Running, A2108_EC_Tuner, A2108_EC_MemoryRunout, A2108_EC_BSP_ERROR1, A2108_EC_BSP_ERROR2

	#define A2108_EC_OK					0	///< There is no error.
	#define A2108_EC_GeneralFail		1	///< Some generic failure happened.
	#define A2108_EC_I2CFail			2	///< I2C bus failed.
	#define A2108_EC_TimeOut			4	///< Operation failed in given time period
	#define A2108_EC_Running			8	///< AVL2108 is working on a previous command.
	#define A2108_EC_MemoryRunout		32	///< There is not enough memory to finish the current job.
	#define A2108_EC_BSP_ERROR1			64	///< The user defined error code, used to report BSP operation failure.
	#define A2108_EC_BSP_ERROR2			128	///< The user defined error code, used to report BSP operation failure.

	///
	/// Up to four AVL2108 chips are supported in one single I2C bus.
	enum A2108_ChipSlaveAddr
	{
		A2108_SA_0 = 0x0c,		///< =0x0c
		A2108_SA_1 = 0x0d,		///< =0x0d
		A2108_SA_2 = 0x0e,		///< =0x0e
		A2108_SA_3 = 0x0f		///< =0x0f
	};

	///
	/// Represents the code rate. AVL2108 can automatically detects the code rate of the input signal.
	enum A2108_FecRate
	{
		RX_DVBS_1_2 = 0,		///< = 0
		RX_DVBS_2_3 = 1,		///< = 1
		RX_DVBS_3_4 = 2,		///< = 2
		RX_DVBS_5_6 = 3,		///< = 3
		RX_DVBS_6_7 = 4,		///< = 4
		RX_DVBS_7_8 = 5,		///< = 5
		RX_DVBS2_1_4 = 6,		///< = 6
		RX_DVBS2_1_3 = 7,		///< = 7
		RX_DVBS2_2_5 = 8,		///< = 8
		RX_DVBS2_1_2 = 9,		///< = 9
		RX_DVBS2_3_5 = 10,		///< = 10
		RX_DVBS2_2_3 = 11,		///< = 11
		RX_DVBS2_3_4 = 12,		///< = 12
		RX_DVBS2_4_5 = 13,		///< = 13
		RX_DVBS2_5_6 = 14,		///< = 14
		RX_DVBS2_8_9 = 15,		///< = 15
		RX_DVBS2_9_10 = 16		///< = 16
	};

	///
	/// Represent the Pilot mode of the signal. AVL2108 can automatically detect the Pilot mode of the input signal.
	enum A2108_Pilot
	{
		RX_Pilot_off = 0,		///< = 0
		RX_Pilot_on = 1			///< = 1
	};

	///
	/// Represents the channel lock mode.
	enum A2108_LockMode
	{
		A2108_LOCK_MODE_FIXED = 0,		///< = 0
		A2108_LOCK_MODE_ADAPTIVE = 1	///< = 1
	};

	///
	/// Represents the modulation mode. AVL2108 can automatically detects the modulation mode of the input signal.
	enum A2108_ModulationMode
	{
		A2108_MM_QPSK = 0,		///< = 0
		A2108_MM_8PSK = 1,		///< = 1
		A2108_MM_16APSK = 2,	///< = 2
		A2108_MM_32APSK = 3		///< = 3
	};

	///
	/// The roll off of the signal. AVL2108 can automatically detects this value from the input signal.
	enum A2108_RollOff
	{
		A2108_RO_20 = 0,		///<  = 0 Rolloff is 0.20
		A2108_RO_25 = 1,		///<  = 1 Rolloff is 0.25
		A2108_RO_35 = 2			///<  = 2 Rolloff is 0.35
	};

	///
	/// The MPEG output format. The default value in AVL2108 chip is \a A2108_MPF_TS
	enum A2108_MpegFormat
	{
		A2108_MPF_TS = 0,		///< = 0  The transport stream.
		A2108_MPF_TSP = 1,		///< = 1  Transport Stream Packet.
	};

	///
	/// The MPEG out port mode. The default value in AVL2108 chip is \a A2108_MPM_Parallel
	enum A2108_MpegMode
	{
		A2108_MPM_Parallel = 0,		///< = 0  Output parallel MPEG data
		A2108_MPM_Serial = 1		///< = 0  Output serial MPEG data via D7.
	};

	///
	/// The MPEG output clock mode. This mode should be set to meet the back end chip's requirement.
	/// The default value in AVL2108 is \a A2108_MPCM_Rising
	enum A2108_MpegClockMode
	{
		A2108_MPCM_Falling = 0,		///<  = 0  MPEG data should be sampled on falling edge of the clock.
		A2108_MPCM_Rising = 1		///<  = 1  MPEG data should be sampled on rising edge of the clock.
	};

	///
	/// There is a RFAGC in AVL2108 chip to work with the external tuner. This enum is used to
	/// set up the polarity of the RFAGC according to the tuner.
	/// The default value in AVL2108 is \a A2108_RA_Invert
	enum A2108_RfagcPola
	{
		A2108_RA_Normal = 0,	///< = 0  The normal way. Increase when signal turns to stronger
		A2108_RA_Invert = 1		///< = 1  The invert way. Increase when signal turns to weaker. 
	};

	///
	/// used to switch chip between sleep mode and halt mode.
	enum A2108_RunningLevel
	{
		A2108_RL_Halt = 0,	///< = 0 Put the chip in halt state.
		A2108_RL_Sleep = 1	///< = 1 put the chip into sleep mode to save power.
	};

	///
	/// used to indicate the chip functional mode: demodulation mode or blind scan mode.
	enum A2108_FunctionalMode
	{
		A2108_FunctMode_Demod = 0,	    ///< = 0 The chip is in demod mode.
		A2108_FunctMode_BlindScan = 1	///< = 1 The chip is in blind scan mode
	};

	/// 
	/// used to switch the tuner control between internal control and external control. Internal tuner control 
	/// code is provided by Availink only and it offer a better performance especially when doing blind scan. External 
	/// tuner control could be created by Availink or User themselves. It is convenient to User when they like to
	/// use a tuner not supported by Availink.
	enum A2108_TunerControl
	{
		A2108_TC_External = 0,		///<  = 0  Use external tuner control code.
		A2108_TC_Internal = 1		///<  = 1  Use internal tuner control code.
	};

	/// Put all MPEG related information together.
	struct A2108_MpegInfo
	{
		enum A2108_MpegFormat m_mpf;  
		enum A2108_MpegMode m_mpm;
		enum A2108_MpegClockMode m_mpcm;
	} ;

    /// Hold the information of the locked signal.
	struct A2108_SignalInfo
	{
		enum A2108_Pilot m_pilot;				///< the pilot information of the signal. \sa ::A2108_Pilot.
		enum A2108_FecRate m_coderate;			///< the code rate information of the signal. \sa ::A2108_FecRate.
		enum A2108_ModulationMode m_modulation;	///< the modulation mode of the signal. \sa ::A2108_ModulationMode.
		enum A2108_RollOff m_rolloff;			///< the roll off of the signal. \sa ::A2108_RollOff.
	};

    ///Put all information together for locking a channel. 
	/* 
   This structure stores carrier channel information. */ 
	struct A2108_Channel
	{
		AVL_uint32 m_uiFrequency_kHz;		///< The satellite carrier frequency, in a unit of kHz. This member is used ONLY for carrying back information from blind scan.
		AVL_uint32 m_uiSymbolRate_Hz;		///< The symbol rate in a unit of Hz. 
		AVL_uint32 m_Flags;					///< Other informations like the signal standard and IQ spectrum \sa CI_FLAG_DVBS, CI_FLAG_DVBS2, CI_FLAG_IQ_SWAPPED ...
	};

    /// The structure to hold all information to distinguish one AVL2108 chip from another since there could be up to 4 chips in a single I2C BUS.
	struct A2108_Chip			
	{
		enum A2108_ChipSlaveAddr m_SlaveAddr;	///< Chip's I2C slave address.
		AVL_uint16 m_StdBuffIndex;			///< The internal stdout port buffer index.
		AVL_uint16 m_DemodFrequency_10kHz;	///< The demod clock corresponding current Pll configure, in the unit of 10kHz.
		AVL_uint16 m_FecFrequency_10kHz;	///< The FEC clock corresponding current Pll configure, in the unit of 10kHz.
		AVL_uint16 m_MpegFrequency_10kHz;	///< The MPEG clock corresponding to the current PLL configuration, in units of 10 kHZ.
		AVL_semaphore m_semRx;				///< A semaphore used to protect the RX command channel.
		AVL_semaphore m_semI2CRepeater;		///< A semaphore used to protect the I2C repeater channel.
		AVL_semaphore m_semI2CRepeater_r;	///< A semaphore used to protect the I2C repeater channel data reading.
		AVL_semaphore m_semDiseqc;			///< A semaphore used to protect the Diseqc operations.
	};

	/* Used to configure the AVL2108's PLL. SDK provided a PLL configuration array in IBSP.h. User can
	cut off unused elements in that array to reduece the footprint. Availink strongly against users changed
	 the PLL setup value themselves. Please contact Availink if there is a requirment to do that.  
	 用来配置2108PLL，在IBSP.h中提供了PLL配置矩阵，用户可以去掉不用的元素。不要自己设置值	 */
	struct A2108_PllConf
	{
		AVL_uint16 m_r1;						///< value for PLL register r1
		AVL_uint16 m_r2;						///< value for PLL register r2
		AVL_uint16 m_r3;						///< value for PLL register r3
		AVL_uint16 m_r4;						///< value for PLL register r4
		AVL_uint16 m_r5;						///< value for PLL register r5
		AVL_uint16 m_RefFrequency_kHz;			///< The reference clock, in the unit of kHz.
		AVL_uint16 m_DmodFrequency_10kHz;		///< The demod clock corresponding current Pll configure, in the unit of 10kHz.
		AVL_uint16 m_FecFrequency_10kHz;		///< The FEC clock corresponding current Pll configure, in the unit of 10kHz.
		AVL_uint16 m_MpegFrequency_10kHz;		///< The MPEG clock corresponding current Pll configure, in the unit of 10kHz.
	} ;

	/// The structure to represent the version information of the AVL2108 chip. \sa AVL_VerInfo
	struct A2108_VerInfo
	{
		struct AVL_VerInfo m_Chip;		///< Hardware version information. Should be 3.2.8.
		struct AVL_VerInfo m_API;		///< SDK version information. Vary on different SDK release.
		struct AVL_VerInfo m_Patch;	///< The version of the internal patch.
	} ;


	 	struct AVL_Tuner
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

	/// Initialize a ::A2108_Chip object. Users first define a object of the ::A2108_chip, and then pass the address of the object to this function. This function will initialize semaphores for the object and fill in the I2C slave address.
	/// 
	/// @param pA2108ChipObject  A pointer points to a ::A2108_Chip object. Function will fill it up.
	/// @param enumSlaveAddress  The slave address for the \a A2108ChipObject
	/// 
	/// @return ::A2108_ErrorCode, Return ::A2108_EC_OK if every thing is OK.
	/// @remarks This function will initialize the semaphores without any checking. It is User's responsibility to make sure each Object only be initialized once. Otherwise, there maybe a semaphores resource leak.
	A2108_ErrorCode InitA2108ChipObject(struct A2108_Chip *pA2108ChipObject, enum A2108_ChipSlaveAddr enumSlaveAddress);

	/// Declaration of the global PLL configuration array variable. The variable must be defined in IBSP.c

#endif
