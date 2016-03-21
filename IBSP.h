

//$Revision: 355 $ 
//$Date: 2008-04-21 16:12:24 -0400 (Mon, 21 Apr 2008) $
// 
#ifndef IBSP_h_h
	#define IBSP_h_h

	#include "avl2108.h"

	#define MAX_II2C_READ_SIZE	64		///< The maximum number of bytes the back end chip can handle in a single I2C read operation. This value must >= 2.
	#define MAX_II2C_Write_SIZE	64		///< The maximum number of bytes the back end chip can handle in a single I2C write operation. This value must >= 8.


#define SSDA_IN 	(P5 &BIT7)   //数据线上的电平
#define SCLK_1  	(P5 |= BIT6) //时钟高
#define SCLK_0 	    (P5 &=~BIT6) //时钟低
#define SDATA_1 	(P5 |= BIT7) //数据高
#define SDATA_0 	(P5 &=~BIT7) //数据低


	void 	Start_I2C(void);
 	void 	Stop_I2c(void);
    AVL_uchar	AckCheck(void);
    AVL_uchar  SendByteAck(unsigned char c);  
    AVL_uchar RcvByte(void);
//    void  Ack_I2c(char a);		//-----------------bj
// 	void Delay(AVL_uint32 uiMS);
	void A2108_IBSP_Delay_ms(AVL_uint32 uiMS);
	/// Prepare everything for BSP operations.
	/// 
	/// @remarks This function never be called inside the SDK. User can redeclare it to any prototype.
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	A2108_ErrorCode A2108_IBSP_Initialize(void);

	/// Destroy all the resource allocated in A2108_IBSP_Initialize and BSP operations.
	/// 
	/// @remarks This function never be called inside the SDK. User can redeclare it to any prototype.
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	A2108_ErrorCode A2108_IBSP_Dispose(void);

	/// Implement a delay to delay uiMS milliseconds.
	/// 
	/// @param uiMS: How many milliseconds to delay.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	void A2108_IBSP_Delay( AVL_uint32 uiMS );

	/// Perform a I2C read operation.
	/// 
	/// @param uiSlaveAddr The I2C device's slave address. This address is a address not include the last R/W bit. Users should shift 1 bits themselves inside the function.
	/// @param pucBuff The buffer carries back the read data.
	/// @param puiSize As input, tells the function how many bytes the caller likes to read. As output, tells caller how many bytes are actually read back. If there is a error, this value will be set to 0.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	/// @remarks This function should perform a directly I2C read operation without write the internal address of the device first. SDK will handle it automatical.
	A2108_ErrorCode A2108_IBSP_I2CRead( AVL_uint16 uiSlaveAddr,  AVL_puchar pucBuff, AVL_uint16 uiSize );

	/// Perform a I2C write operation.
	/// 
	/// @param uiSlaveAddr The I2C device's slave address. This address is a address not include the last R/W bit. Users should shift 1 bits themselves inside the function.
	/// @param pucBuff The buffer carries the data to write out.
	/// @param puiSize As input, tells the function how many bytes the caller likes to write. As output, tells caller how many bytes are actually wrote out. If there is a error, this value will be set to 0.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	/// Return ::A2108_EC_I2CFail if there is a I2C problem.
	A2108_ErrorCode A2108_IBSP_I2CWrite( AVL_uint16 uiSlaveAddr,  AVL_puchar pucBuff, AVL_uint16 uiSize );

	/// Initializes a semaphore object.
	/// 
	/// @param pSemaphore A pointer to a ::AVL_semaphore object which need to be initialized.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	/// @remarks All the semaphore objects should be initialized with 1 as maximum count and the initialized state should be signaled. ie, after initialized, the very first query should success.
	A2108_ErrorCode A2108_IBSP_InitSemaphore( AVL_psemaphore pSemaphore ); 

	/// Query the semaphore. if the semaphore is hold by others, the function should be blocked until the semaphore is available.
	/// 
	/// @param pSemaphore A pointer to a ::AVL_semaphore object which need to be queried.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	A2108_ErrorCode A2108_IBSP_WaitSemaphore( AVL_psemaphore pSemaphore );         

	/// Release the semaphore so that others can use it.
	/// 
	/// @param pSemaphore A pointer to a ::AVL_semaphore object which need to be released.
	/// 
	/// @return ::A2108_ErrorCode, 
	/// Return ::A2108_EC_OK if everything is OK. 
	A2108_ErrorCode A2108_IBSP_ReleaseSemaphore( AVL_psemaphore pSemaphore );


#endif
