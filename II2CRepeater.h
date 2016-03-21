
//$Revision: 514 $ 
//$Date: 2008-08-12 17:51:32 -0400 (Tue, 12 Aug 2008) $
// 
#ifndef II2CRepeater_h_h
	#define II2CRepeater_h_h

	#include "avl2108.h"

	#define	rc_i2cm_speed_kHz_addr			0x000005CC
	#define	rc_i2cm_time_out_addr			0x000005CE
	#define i2cm_cmd_addr					0x00000404
	#define i2cm_rsp_addr					0x00000418
	#define i2cm_status_addr				0x00000438
	#define	rs_tuner_status_addr			0x000007B2

	#define I2CM_CMD_LENGTH   0x14
	#define I2CM_RSP_LENGTH   0x14

	#define OP_I2CM_NOOP      0x00
	#define OP_I2CM_INIT	  0x01	//≥ı ºªØ√¸¡Ó
	#define OP_I2CM_WRITE     0x02	//–¥√¸¡Ó
	#define OP_I2CM_READ      0x03	//∂¡√¸¡Ó

	enum A2108_I2CM_STATUS
	{
		A2108_I2CM_STATUS_OK            = 0,	///< everything is OK.
		A2108_I2CM_STATUS_BAD_OPCODE    = 1,	///< The OP_I2CM_XXX is not valid.
		A2108_I2CM_STATUS_SLAW_NACK     = 2,	///< The slave device doesn'r give a ACK after the slave_addr+w is ent out.
		A2108_I2CM_STATUS_DATA_NACK     = 3,	///< The slave device doesn'r give a ACK after the a byte of data is sent out.
		A2108_I2CM_STATUS_SLAR_NACK     = 5,	///< The slave device doesn'r give a ACK after the slave_addr+r is ent out.
		A2108_I2CM_STATUS_BUS_ERROR     = 6,	///< AVL2108 internal bus error when access I2C repeater hardware.
	};

	///@endcond

	///@cond

	/// Send operation command to firmware to perform a I2C repeater operation. Other SDK functions should chunk the command and it's parameters into a buffer array before call this function.
	/// 
	/// @param pBuff The buffer array which contains the operation command and the parameters.
	/// @param ucSize The size of the \a pBuff.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	A2108_ErrorCode A2108_II2CRepeater_SendOP(AVL_puchar pBuff, AVL_uchar ucSize, struct A2108_Chip * pAVLChip );

	///@endcond

	/// Initialize the I2C repeater.
	/// 
	/// @param I2CBusClock_kHz The clock speed of the I2C bus between the tuner and the AVL2108 chip.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// @remarks This function must be called before any other function in this interface. This is a nonblocking function, call ::A2108_II2CRepeater_GetOPStatus to determine if the I2C repeater is initialized.
	A2108_ErrorCode A2108_II2CRepeater_Initialize( AVL_uint16 I2CBusClock_kHz, struct A2108_Chip * pAVLChip );

	/// Read data back from the tuner via the I2C repeater.  This function is used with tuners which insert a stop
	/// bit between messages.
	/// 
	/// @param ucSlaveAddr The slave address of the tuner device. Please note AVL2108 only support 7 bits slave address.
	/// @param pucBuff The buffer which carry back the read data.
	/// @param uiSize How many bytes the caller want to read back.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// Return ::A2108_EC_GeneralFail if \a uiSize is bigger than 20
	/// @remarks This function will trigger a I2C read operation. It is used with tuners which insert a stop bit between messages.  The read position (or device internal address) could be determined by calling ::A2108_II2CRepeater_SendData.
	A2108_ErrorCode A2108_II2CRepeater_ReadData(  AVL_uchar ucSlaveAddr, AVL_puchar pucBuff, AVL_uint16 uiSize, struct A2108_Chip * pAVLChip );

	/// Read data back from the tuner via the I2C repeater.  This function is used with tuners which do not insert a stop
	/// bit between messages.
	/// 
	/// @param ucSlaveAddr The slave address of the tuner device. Please note AVL2108 only support 7 bits slave address.
	/// @param pucBuff The buffer which carry back the read data.
	/// @param ucRegAddr The address of the register being read.
	/// @param uiSize How many bytes the caller want to read back.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// Return ::A2108_EC_GeneralFail if \a uiSize is bigger than 20
	/// @remarks This function will trigger a I2C read operation. It is used with tuners which do not insert a stop bit between messages.
	A2108_ErrorCode A2108_II2CRepeater_ReadData_Multi(  AVL_uchar ucSlaveAddr, AVL_puchar pucBuff, AVL_uchar ucRegAddr, AVL_uint16 uiSize, struct A2108_Chip * pAVLChip );

	/// Write data to the tuner via the I2C repeater.
	/// 
	/// @param ucSlaveAddr The slave address of the tuner device. Please note AVL2108 only support 7 bits slave address.
	/// @param ucBuff The buffer which hold the send out data.
	/// @param uiSize How many bytes the caller want to wirte.
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	/// Return ::A2108_EC_GeneralFail if \a uiSize is bigger than 17
	/// @remarks The internal register address is buried in the \a ucBuff. This function is a nonblocking function, call ::A2108_II2CRepeater_GetOPStatus to determine if the write operation is done.
	A2108_ErrorCode A2108_II2CRepeater_SendData(  AVL_uchar ucSlaveAddr, const AVL_puchar ucBuff, AVL_uint16 uiSize, struct A2108_Chip * pAVLChip );

	/// Check if the last I2C repeater operation is finished.
	/// 
	/// @param pAVLChip A pointer point to a ::A2108_Chip object which is used to tell function which chip it is working on.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if there is no pending I2C operation.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// Return ::A2108_EC_Running if AVL2108 is still working on last command.
	A2108_ErrorCode A2108_II2CRepeater_GetOPStatus(const struct A2108_Chip * pAVLChip );


#endif
