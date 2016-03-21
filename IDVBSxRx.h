
//$Revision: 914 $ 
//$Date: 2009-07-30 09:59:11 -0400 (Thu, 30 Jul 2009) $
// 
#ifndef IABSRx_h_h
	#define IABSRx_h_h

	#include "avl2108.h"

	
	#define	rc_rfagc_pol_addr							0x0000043c
	#define	rc_specinv_addr								0x00000470
	#define	rc_mpeg_posedge_addr						0x000004DC
	#define	rc_mpeg_serial_addr							0x000004DE
	#define	rc_mpeg_mode_addr							0x0000045C
	#define rc_aagc_acq_gain_addr						0x00000518
	#define	rs_int_SNR_dB_addr							0x00000680
	#define	rc_int_dmd_clk_MHz_addr						0x00000580
	#define	rc_int_fec_clk_MHz_addr						0x00000582
	#define rc_int_mpeg_clk_MHz_addr					0x00000584
	#define	rc_int_sym_rate_Hz_addr						0x00000490
    #define	rc_format_addr								0x0000044C
	#define	rc_alpha_addr								0x00000440
	#define	rc_alpha_setting_addr						0x0000046C
	#define	rc_decode_mode_addr							0x0000248A
    #define	rc_iq_swap_mode_addr						0x00002488
	#define	rc_show_demod_status_addr					0x000004E8
	#define rc_int_carrier_freq_half_range_MHz_addr		0x0000058A
	#define	rc_show_per_addr							0x000004EA
	#define	rc_show_ber_addr							0x000004EC
	#define rs_main_state_addr							0x00000640
	#define	rs_ScatterData_rdy_addr						0x000007B4
	#define	rs_fec_lock_addr							0x00000790
	#define	rs_int_carrier_freq_100kHz_addr				0x0000076E
	#define	rs_code_rate_addr							0x00000648
	#define	rs_modulation_addr							0x0000064c
	#define	rs_pilot_addr								0x00000650
	#define	rp_uint_BER_addr							0x000007F8
	#define	rp_uint_PER_addr							0x000007FC
	#define rx_aagc_gain								0x0040004C
    #define rx_Rolloff_addr								0x00400030
	#define scatter_data_addr							0x00001A70
	#define patchtext_addr								0x0000248C
	#define patchglobalvar_addr							0x00002498
	#define rp_dvbs_ber_addr							0x00002480
	#define rc_dvbs_fec_max_trial_for_same_phase		0x000005AE
	#define rc_dvbs_fec_fcnt_step_addr					0x000005A4
	#define rc_para_rate_adj_n							0x000004E4
	#define rc_para_rate_adj_d							0x000004E6
	#define rp_dishpoint_mode_addr						0x0000247A
	#define rp_lock_mode_addr							0x00002478
	#define rp_functional_mode_addr						0x00002476
	#define rs_tuner_status_addr						0x000007B2
    
	/// @endcond

	/// Initialize the demodulator.
	/// 
    /// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function must be called first before all other functions declared in this interface.
	A2108_ErrorCode A2108_IDVBSxRx_Initialize(const struct A2108_Chip * pAVLChip );

	/// Set the polarity of the RF AGC. This value should be set up according to the tuner requirement.
	/// 
	/// @param enumAGCPola The polarity of the RF AGC. Refer to ::A2108_RFAGCPola.
    /// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IDVBSxRx_SetRFAGCPola( enum A2108_RfagcPola enumAGCPola, const struct A2108_Chip * pAVLChip );

	/// Set up the MPEG output mode.
	/// 
	/// @param pMpegMode Refer to ::A2108_MPEG_Info. Provides the MPEG output mode information. This parameter must be initialized before it is past to the function.
    /// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IDVBSxRx_SetMpegMode( const struct A2108_MpegInfo * pMpegMode, const struct A2108_Chip * pAVLChip );

	/// Get the RF signal level.
	/// 
	/// @param puiRFSignalLevel carries back the signal level from 0 to 65535, corresponding the weakest signal and the strongest signal.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function actually return back the value of the RFAGC. The corresponding dbm value varies upon the tuner. User needs to get the relationship from the Tuner data sheet or through measurement.
	/// The following figure is a example curve for Sharp BS2S7HZ6306 tuner.
	/// 
	/// @image html Sharp_Signal_Level.PNG
	A2108_ErrorCode A2108_IDVBSxRx_GetSignalLevel(AVL_puint16 puiRFSignalLevel , const struct A2108_Chip * pAVLChip );

	/// Sets the carrier frequency sweep range for subsequent LockChannel calls. Using this function is optional, the default value is 500 (+/-5MHz). This function is typically used to narrow the sweep range in cases where there are multiple narrow bandwidth signals within the default range and there is the possibility that the wrong signal will be locked.  If multiple signals are detected within the specified sweep range, the AVL2108 will lock to the stronger one.
	/// 
	/// @param uiFreqSweepRange_10kHz Half of the frequency sweep range in units of 10kHz. The maximum value is 500 (sweep from -5MHz to +5MHz).
    /// @param pAVLChip A pointer to the ::A2108_Chip object for which the frequency sweep range is being set.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if the frequency sweep range is successfully upated.
	/// Return ::A2108_EC_I2CFail if there is an I2C communication problem.
	/// @remarks The input parameter uiFreqSweepRange_10kHz specifies half of the actual sweep range. The whole range will be [-uiFreqSweepRange_10kHz, +uiFreqSweepRange_10kHz]. Note that this sweep range must be large enough to accomodate the RF path frequency uncertainty.
	A2108_ErrorCode A2108_IDVBSxRx_SetFreqSweepRange(AVL_uint16 uiFreqSweepRange_10kHz, const struct A2108_Chip * pAVLChip );

	/// Configures the m_Flags member of the A2108_Channel object to the desired lock mode.  On boot up the AVL2108 default channel lock mode is A2108_LOCK_MODE_FIXED.
	/// 
	/// @param psChannel  Pointer to the channel object for which to configure the lock mode.
	/// @param enumChannelLockMode The desired lock mode.  Refer to ::A2108_LockMode. 
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK after the m_Flags member of the psChannel input parameter is configured with the desired lock mode.
	A2108_ErrorCode A2108_IDVBSxRx_SetChannelLockMode( struct A2108_Channel * psChannel, enum A2108_LockMode enumChannelLockMode );

	/// Locks to a channel using the parameters specified in \a psChannel.
	/// 
	/// @param psChannel  Holds the channel related parameters needed by the AVL2108 to lock to the input signal.
	/// @param pAVLChip A pointer to the A2108_Chip object for which the lock operation is being performed. 
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if the lock parameters and command are successfully sent to the AVL2108.
	/// Return ::A2108_EC_I2CFail if there is an I2C communication problem.
	/// Return ::A2108_EC_Running if the lock command could not be sent to the AVL2108 because the AVL2108 is still processing a previous command.
	/// @remarks Calling this function commands the AVL2108 to lock to a particular channel. Use the function 
	/// GetLockStatus to determine if the AVL2108 has successfully locked to the channel. The channel lock operation is 
	/// performed in either fixed mode or adaptive mode depending on how the psChannel input parameter has been configured.  
	/// Please see the function SetChannelLockMode for more details regarding how to configure the channel object for either mode.  This function can perform automatic IQ swap if \a psChannel->m_Flags has the CI_FLAG_IQ_AUTO_BIT bit set.
	A2108_ErrorCode A2108_IDVBSxRx_LockChannel( struct A2108_Channel * psChannel, struct A2108_Chip * pAVLChip );

	/// Check if the AVL2108 locked the signal.
	/// 
	/// @param puiLockStatus Carries back the lock status. If the AVL2108 locked the signal, \a puiLockStatus will be 1, otherwise, it will be 0.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function should be called after the ::A2108_IDVBSxRx_LockChannel was called.
	A2108_ErrorCode A2108_IDVBSxRx_GetLockStatus( AVL_puint16 puiLockStatus, const struct A2108_Chip * pAVLChip );

	/// Reset the BER/PER accumulation.
	/// 
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	A2108_ErrorCode A2108_IDVBSxRx_ResetErrorStat( struct A2108_Chip * pAVLChip );

	/// Read back the scatter data from the AVL2108 chip. It is useful to visualize the input signal.
	/// 
	/// @param ucpData  Carries back the scatter data. The buffer size MUST >= 2*(*pSize) because there are two bytes per set of the IQ. The first (*pSize) bytes in the buffer are I values, and the following (*pSize) bytes are Q values.
	/// @param puiSize  As input tells the function how many sets of IQ the caller likes to read back; As output, tells the caller how many sets of IQ are read back.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks Normally, the size of the scatter data will be 132 sets of IQ. i.e. 264 bytes.
	A2108_ErrorCode A2108_IDVBSxRx_GetScatterData( AVL_puchar ucpData, AVL_puint16 puiSize, const struct A2108_Chip * pAVLChip );

	/// Read back the current SNR calculation result.
	/// 
	/// @param puiSNR_db carries back the current SNR value. The value is scaled by 100. For example, read back 2578 means the SNR value is 25.78 db.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks It takes a few seconds for the chip to calculate a stable SNR value after FEC lock. The function returns an SNR value of 0 before a stable SNR value is calculated.
	A2108_ErrorCode A2108_IDVBSxRx_GetSNR( AVL_puint32 puiSNR_db, const struct A2108_Chip * pAVLChip );

	/// Read back the current PER calculation result.
	/// 
	/// @param puiPER carries back the current PER value. The value is scaled by 1e-9. For example, read back 123456 means the PER value is 0.000123456.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks It take a few seconds for the chip to calculates a stable PER value after the FEC lock. Function will give 0 before a stable PER is calculated.
	A2108_ErrorCode A2108_IDVBSxRx_GetPER( AVL_puint32 puiPER, const struct A2108_Chip * pAVLChip );

	/// Read back the current BER calculation result.
	/// 
	/// @param puiBER carries back the current BER value. The value is scaled by 1e-9. For example, read back 123456 means the BER value is 0.000123456.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks It take a few seconds for the chip to calculates a stable BER value after the FEC lock. Function will give 0 before a stable BER is calculated.

	A2108_ErrorCode A2108_IDVBSxRx_GetDVBSBER( AVL_puint32 puiBER, const struct A2108_Chip * pAVLChip );

	/// Reset the DVBS BER calculation.
	/// 
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function works together with ::A2108_IDVBSxRx_GetDVBSBER. Used to reset the internal data counters. After reset, user needs to give AVL2108 chip some time to statistic enough symbols. Availink suggests to wait at least 5M symbols.
	A2108_ErrorCode A2108_IDVBSxRx_ResetDVBSBER( const struct A2108_Chip * pAVLChip );

	/// Get the RF frequency offset. This usually used to adjust the tuner setup.
	/// 
	/// @param piRFOffset_100kHz Carries back the offset of the RF frequency in the unit of 100kHz. For example, if the real RF frequency is 1000M, and the tuner is tuned to 998M, this value will be -20 100kHz( i.e. -2M).
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks Call this function after the AVL2108 chip locked the input signal. 
	A2108_ErrorCode A2108_IDVBSxRx_GetRFOffset( AVL_pint16 piRFOffset_100kHz, const struct A2108_Chip * pAVLChip );

	/// Get the current locked signal information.
	/// 
	/// @param pSignalInfo Carries back the detected signal information. Refer to ::A2108_Signal_Info.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks Call this function after AVL2108 chip locked the input signal.
	A2108_ErrorCode A2108_IDVBSxRx_GetSignalInfo( struct A2108_SignalInfo * pSignalInfo, const struct A2108_Chip * pAVLChip );

	/// Set the dish pointing mode.  Allows the user to enable or disable dish pointing mode.
	/// 
	/// @param ucMode Indicates whether dish pointing mode is being enabled or disabled.  A value of 0 indicates disable and a value of 1 indicates enable.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IDVBSxRx_SetDishPointingMode( AVL_uchar ucMode, const struct A2108_Chip *pAVLChip );


#endif
