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



///@file
///@brief Provide function to perform I2C atomic operations.
///@details User should always use the function provided in this file to perform I2C operations. These functions provide 
/// multi_thread protection and chunk the big packages to smaller package so that it can meet the hardware limitation. 
/// These functions also eliminate the difference between the big endian and little endian system. Please be noted that 
/// these I2C functions can only be used to perform I2C operations for AVL2108 chip. When read/write data fomr/to AVL2108 
/// chip, all the data are considered as unsigned data type. Users need to do cast themselves.
/// 
//$Revision: 323 $ 
//$Date: 2008-03-14 11:31:11 -0400 (Fri, 14 Mar 2008) $
// 
#ifndef II2C_h_h
	#define II2C_h_h

	#include "avl2108.h"

	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_MemoryRunout if the semaphores can not be initialized.
	/// @remarks This is a internal function. This function must be called before any I2C operations. (We already call it in function ::InitA2108ChipObject, so the users do not need to worry about it.)
	A2108_ErrorCode A2108_II2C_Initialize(void);

	/// @endcond
	
	/// Read \a uiSize bytes from AVL2108 to \a pucBuff from AVL2108 internal address \a uiOffset.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param uiOffset  The AVL2108 internal address where the function will read data from.
	/// @param pucBuff  Carries back the data read from the AVL2108 chip.
	/// @param uiSize Tells the function read back how many bytes.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks The function will first perform a I2C write to tell AVL2108 the internal address.
	A2108_ErrorCode A2108_II2C_Read( AVL_uint16 uiSlaveAddr, AVL_uint32 uiOffset, AVL_puchar pucBuff, AVL_uint16 uiSize);

	/// Read \a uiSize bytes from AVL2108 to \a pucBuff.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param pucBuff  Carries back the data read from the AVL2108 chip.
	/// @param uiSize Tells the function read back how many bytes.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks The function will directly perform a I2C read.
	A2108_ErrorCode A2108_II2C_ReadDirect( AVL_uint16 uiSlaveAddr, AVL_puchar pucBuff, AVL_uint16 uiSize);

	/// Perform a I2C write operation.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param pucBuff  Holds the data which will be sent to AVL2108 chip.
	/// @param uiSize Tells the function how many bytes will be sent out.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_II2C_Write( AVL_uint16 uiSlaveAddr, AVL_puchar pucBuff, AVL_uint16 uiSize);

	/// Read back a 16 bits unsigned integer from AVL2108 chip.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param uiAddr  The AVL2108 internal address where the function will read data from.
	/// @param puiData Carries back the 16 bits unsigned integer.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_II2C_Read16( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint16 puiData );

	/// Read back a 32 bits unsigned integer from AVL2108 chip.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param uiAddr  The AVL2108 internal address where the function will read data from.
	/// @param puiData Carries back the 32 bits unsigned integer.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_II2C_Read32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_puint32 puiData );

	/// Write a 16 bits unsigned integer to AVL2108 chip.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param uiAddr  The AVL2108 internal address where the function will read data from.
	/// @param uiData The data will be write to AVL2108 chip.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_II2C_Write16( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint16 uiData );

	/// Write a 32 bits unsigned integer to AVL2108 chip.
	/// 
	/// @param uiSlaveAddr  The slave address of the AVL2108 chip.
	/// @param uiAddr  The AVL2108 internal address where the function will read data from.
	/// @param uiData The data will be write to AVL2108 chip.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if every thing is OK.
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_II2C_Write32( AVL_uint16 uiSlaveAddr, AVL_uint32 uiAddr, AVL_uint32 uiData );



#endif
