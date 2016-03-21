
//$Revision: 916 $ 
//$Date: 2009-07-31 18:13:11 -0400 (Fri, 31 Jul 2009) $
// 
#ifndef IBase_h_h
	#define IBase_h_h

	#include "avl2108.h"
	#include "ITuner.h"


	#define core_reset_b_reg		0x600000
	#define reset_register			0x6C0000
	#define pll_r1					0x6C0080
	#define pll_r2					0x6C00C0
	#define pll_r3					0x6C0100
	#define pll_r4					0x6C0140
	#define pll_r5					0x6C0180
    #define pll_softvalue_en		0x6C0200

	#define gpio_data_in_to_reg		0x6C0004
	#define gpio_data_reg_out		0x6C0008
	#define gpio_reg_enb			0x6C000c

	#define rom_ver_addr			0x00100000
	#define error_msg_addr			0x0000042c
	#define	rc_patch_ver_addr		0x000004A8
	#define rx_cmd_addr				0x00000400
	#define core_ready_word_addr	0x00000434
	#define rc_sigma_delta_reg1		0x006C0024

	#define OP_RX_NOOP		    	0x00
	#define OP_RX_LD_DEFAULT    	0x01
	#define OP_RX_INIT_GO       	0x02
	#define OP_RX_RESET_BERPER  	0x03
	#define OP_RX_HALT          	0x04
	#define OP_RX_SLEEP         	0x05
	#define OP_RX_WAKE          	0x06
	#define OP_RX_BLIND_SCAN    	0x08
	#define OP_RX_STDOUT_MODE   	0x09
	#define OP_RX_TUNER_LOCK    	0x0b
	#define OP_RX_TUNER_LOCK_ST 	0x0c
	#define OP_RX_HELLO				0xFF
	///@endcond

	///@cond
	#define A2108_API_VER_MAJOR	0x03
	#define A2108_API_VER_MINOR	0x03
	#define A2108_API_VER_BUILD	0x00

	/// Downloads the firmware to the AVL2108.
	/// 
	/// @param pInitialData Pointer to the buffer which holds the firmware data.
	/// @param pAVLChip Pointer to the ::A2108_Chip object for which the firmware is being downloaded.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if the firmware download is successful. 	
	/// Return ::A2108_EC_I2CFail if there is an I2C communication problem.
	/// @remarks This function is an 'internal' function. Availink does not recommend that the user call it directly. 
	A2108_ErrorCode A2108_IBase_UploadFirmware(const AVL_puchar pFirmwareData,  const struct A2108_Chip * pAVLChip);

	///@endcond
	
	/// Check if the chip is finished the initialization procedure.
	/// 
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if chip is ready for further operations. 
	/// Return ::A2108_EC_GeneralFail if AVL2108 did not successfully boot up.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IBase_GetStatus( struct A2108_Chip * pAVLChip );

	/// Reads back the version information of the chip.
	/// 
	/// @param pVerInfo carries back the version informations. Refer to ::A2108_Ver_Info for details.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
//	A2108_ErrorCode A2108_IBase_GetVersion( struct A2108_VerInfo * pVerInfo, const struct A2108_Chip * pAVLChip ) ;

	/// Initialize the AVL2108 Chip. This function loads the patch into the chip and boot up the chip. 
	/// Call ::A2108_IBase_GetStatus later to find out if the initialization is finished.
	/// 
	/// @param pPLLConf  Points to a ::A2108_PLLConf object the function will use it to initialize the PLL.
	/// @param pInitialData The pointer point to the buffer which holds the initialize data.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IBase_Initialize( const struct A2108_PllConf * pPLLConf, AVL_puchar pInitialData,  struct A2108_Chip * pAVLChip ) ;

	/// Send a \a ucOpCmd command to the AVL2108 chip.
	/// 
	/// @param ucOpCmd  The OP_RX_xxx command code defined in avl2108op.h.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function is a 'internal' function. Availink doesn't recommend user call it directly. 
	/// The following diagram denotes the internal operation of the function.
	/// 
	/// @image html A2108_IBase_SendRxOP.png
	A2108_ErrorCode A2108_IBase_SendRxOP(AVL_uchar ucOpCmd, struct A2108_Chip * pAVLChip );

	/// Check if the last OP_Rx command is finished.
	/// 
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if last command is finished. 
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function is a 'internal' function. Availink doesn't recommend user call it directly.
	A2108_ErrorCode A2108_IBase_GetRxOPStatus(const struct A2108_Chip * pAVLChip );

	/// Set AVL2108 chip into the \a enumRunningLevel mode.
	/// 
	/// @param enumRunningLevel The mode which the caller likes to put AVL2108 into.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks After "wake up" from the sleep mode, ::A2108_IDVBSxRx_LockChannel need to be called to lock the channel. This function is a unblock function. In other words, involke this function only notice AVL2108 that it should change the running level. User needs to call ::A2108_IBase_GetRxOPStatus to check if the command is performed.
	A2108_ErrorCode A2108_IBase_SetRunningLevel( enum A2108_RunningLevel enumRunningLevel,  struct A2108_Chip * pAVLChip ) ;

	/// Switch the chip functionality by uploading different firmware to the A2108 chip. A2108 chip can work
	/// in either normal demodulation mode or blind scan mode. Each of them needs different firmware support.
	/// 
	/// @param pInitialData The pointer point to the buffer which holds the firmware data. Depend on the mode user likes to switch to, the firmware could
	/// be one of two candidates: Firmware for blind scan or firmware for normal demodulation.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if chip is ready for further operations. 	
	/// Return ::A2108_EC_GeneralFail if AVL2108 did not successfully boot up with the firmware.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IBase_SetFunctionalMode(const AVL_puchar pFirmwareData,  AVL_puchar pTunerControlData, const struct AVL_Tuner * pTuner, const struct A2108_Chip * pAVLChip);

	/// Check the current functional mode. A2108 chip can work in either normal demodulation mode or blind scan mode.
	/// 
	/// @param pFunctionalMode A pointer to an ::A2108_FunctionalMode object which is used to carry back the current functional mode.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if chip is ready for further operations. 		
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IBase_GetFunctionalMode(enum A2108_FunctionalMode * pFunctionalMode,  const struct A2108_Chip * pAVLChip);


	/// Set up the GPIO pins direction.
	/// 
	/// @param ucDir There are three GPIO pins in AVL2108 chip, corresponding to bit 0, 1 and 2. Set the bit to 1 will make the corresponding pin to be input pin.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks There are totally three GPIO pins. Bit0 corresponding to pin CS_0; Bit 1 corresponding to pin CS_1; Bit 2 corresponding to pin GPIO_Clock.
	A2108_ErrorCode A2108_IBase_SetGPIODir( AVL_uchar ucDir, const struct A2108_Chip * pAVLChip );

	/// Set the output GPIO's output level.
	/// 
	/// @param ucVal There are three GPIO pins in AVL2108 chip, corresponding to bit 0, 1 and 2. Set the bit to 1 will make the corresponding pin to output high.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks The function only works for output GPIO pins. Set the value to a input GPIO pin take no effect.
	/// There are totally three GPIO pins. Bit0 corresponding to pin CS_0; Bit 1 corresponding to pin CS_1; Bit 2 corresponding to pin GPIO_Clock.
	A2108_ErrorCode A2108_IBase_SetGPIOVal( AVL_uchar ucVal, const struct A2108_Chip * pAVLChip );

	/// Read the GPIO pins current level.
	/// 
	/// @param pucVal  There are three GPIO pins in AVL2108 chip, corresponding to bit 0, 1 and 2. if the bit read back is 1, the corresponding pin's current level is higher.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks The function works for both input and output GPIO pins.
	/// There are totally three GPIO pins. Bit0 corresponding to pin CS_0; Bit 1 corresponding to pin CS_1; Bit 2 corresponding to pin GPIO_Clock.
	A2108_ErrorCode A2108_IBase_GetGPIOVal( AVL_puchar pucVal, const struct A2108_Chip * pAVLChip );

	/// Saves the AVL2108 configuration.  This function is an internal Availink function.  It is called just before
	//  an AVL2108 functional mode change to preserve the AVL2108 configuration.
	/// 
	/// @param puiBuff32  Pointer to a buffer of 32-bit unsigned integers.  The functions writes some of the AVL2108 configuration information to this buffer.
	/// @param puiBuff16  Pointer to a buffer of 16-bit unsigned integers.  The functions writes some of the AVL2108 configuration information to this buffer.
	/// @param pAVLChip Pointer to the ::A2108_Chip object for which the configuration information is being stored.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if the configuration information has been stored.
	/// Return ::A2108_EC_I2CFail if there is an I2C communication problem.
	/// @remarks This function is an 'internal' function. Availink does not recommend that the user call it directly. 
	A2108_ErrorCode A2108_IBase_SaveRxConfig(AVL_puint32 puiBuff32, AVL_puint16 puiBuff16, const struct A2108_Chip * pAVLChip);

	/// Restores the AVL2108 configuration.  This function is an internal Availink function.  It is called just after
	//  an AVL2108 functional mode change to restore the AVL2108 configuration.
	/// 
	/// @param puiBuff32  Pointer to a buffer of 32-bit unsigned integers which contains some of the AVL2108 configuration information.
	/// @param puiBuff16  Pointer to a buffer of 16-bit unsigned integers which contains some of the AVL2108 configuration information.
	/// @param pAVLChip Pointer to the ::A2108_Chip object for which the configuration information is being restored.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if the configuration information has been restored.
	/// Return ::A2108_EC_I2CFail if there is an I2C communication problem.
	/// @remarks This function is an 'internal' function. Availink does not recommend that the user call it directly. 
	A2108_ErrorCode A2108_IBase_RestoreRxConfig(AVL_puint32 puiBuff32, AVL_puint16 puiBuff16, const struct A2108_Chip * pAVLChip);


#endif
