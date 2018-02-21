/**
  @file I2C_jmk.c
  @brief : Wrapper functions to utilize and build upon HAL I2C functionality.
<pre>
	 Note: Some of these may not presently be utilized.
	       Wrapper function names may improve understanding the intention of the code.
	       Wrapper functions reduce the number input parameters to make code more specific to
	       a particular intention.

</pre>

   @author 	Joe Kuss (JMK)
   @date 	2/16/2018

   \pagebreak
*/

/* Includes ------------------------------------------------------------------*/
#include "i2c_jmk.h"
#include "led.h"

#define SIZE_OF_ONE_BYTE	1
#define TIMEOUT_100MS		100

HAL_StatusTypeDef I2CWriteStatus;
I2C_HandleTypeDef hi2c2;

/**
 *  @brief Wrapper for writing one byte via I2C.
 *  Calls HAL routine "HAL_I2C_Master_Receive", configured to allow 100 MS timeout for write attempt.
 *
 *  @param hi2c	- pointer to "I2C_HandleTypeDef" HAL I2C handle information.
 *  @param DevAddress - target I2C address.
 *  @param Data - Outgoing byte.
 *
 */

void I2C_WriteOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data)
{
	I2CWriteStatus = HAL_I2C_Master_Transmit(hi2c, DevAddress, &Data, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);
}

/**
 *  @brief Wrapper for writing multiple bytes via I2C.
 *  Calls HAL routine "HAL_I2C_Master_Transmit"
 *
 *  @param hi2c	- pointer to "I2C_HandleTypeDef" HAL I2C info handle.
 *  @param DevAddress - target I2C address.
 *  @param pData - pointer to buffer containing outgoing bytes.
 *  @param Size  - number of bytes to write.
 *
 */
void I2C_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	I2CWriteStatus = HAL_I2C_Master_Transmit(hi2c, DevAddress, pData, Size, (uint32_t)TIMEOUT_100MS);
}


/**
 *  @brief Wrapper for reading one byte via I2C.
 *  Calls HAL routine "HAL_I2C_Master_Receive", configured to allow 100 MS timeout for read attempt.
 *
 *  @param hi2c	- pointer to "I2C_HandleTypeDef" HAL I2C handle information.
 *  @param DevAddress - target I2C address.
 *  @param pData - pointer to location to store incoming byte.
 *
 */
void I2C_ReadOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pdata)
{
	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, DevAddress, pdata, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);
}

/**
 *  @brief Wrapper for reading multiple bytes via I2C.
 *  Calls HAL routine "HAL_I2C_Master_Receive"
 *
 *  @param hi2c	- pointer to "I2C_HandleTypeDef" HAL I2C handle information.
 *  @param DevAddress - target I2C address.
 *  @param pData - pointer to buffer to hold incoming bytes.
 *  @param Size  - number of bytes to read.
 *
 */
void I2C_ReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,uint8_t *pData, uint16_t Size )
{
	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, DevAddress, pData, Size, (uint32_t)TIMEOUT_100MS);
}

/*
 * Note:
 *
 * Although the above "I2C Read and Write" routines do what they say they are going to do,
 * This is not what is required for the STM32VLDISCOVERY  to be the Master
 * I2C controller for the slave PicMPLabExpress_16F15376 PIC board demo "EEprom" simulator
 * which is a layer higher than basic I2C bus reads and writes.
 *
 * 		Requirements:
 *
 *      From PicMPLabExpress_16F15376 PIC board demo code base
 *      "void I2C1_StatusCallback(..)" in file: "i2c1.c"
 *      of  Microchip PIC MCC auto-generated demo code for a I2C "MSSP" slave.
 *		This slave driver emulates an EEPROM Device.
 *
 *    	Sequential reads from the EEPROM will return data at the next EEPROM address.
 *
 *    	Random access reads can be performed by writing a single byte
 *    	EEPROM address, followed by 1 or more reads.
 *
 *   	Random access writes can be performed by writing a single byte
 *   	EEPROM address, followed by 1 or more writes.
 *
 *   	Every read or write will increment the internal EEPROM address.
 *
 *    	When the end of the EEPROM is reached, the EEPROM address will
 *   	continue from the start of the EEPROM.
 *
 *		In both cases read and write we must write a first byte to the PIC
 *		to tell it the desired starting address of the memory range in the
 *		simulated EEprom.
 */

/*
 	 	 Sample code taken from earlier experiments with PIC demo board simulated eeprom

 	 	uint16_t targetAddressU16;

		// PIC I2C ADDR on demo board is programmed to be 0x08 but it seems our system, HAL
		// does not shift right this value by one to fit in I2C stream,
		// so we do it here:

		targetAddressU16 = 0x0008;
		targetAddressU16 <<=1;

 	 	// First example:
		// This below just writes 0x55 to I2C at PIC device addr (Ox08 actually)
		// Data in the eeprom is not directly addressable, but will start at 0
		// assuming the PIC board had been just reset, and advance by 1 for
		/  each byte read or written to by us,
		// (If this was done 128 times in a row it will overflow at 127 and go back to 0)
		// Only way to know where eeprom data, index is presently is to
		// reset the PIC (restart it's program) to know index (byte address) is preset to 0

		// First example is different since we do not specify the EE address at all,
		// But maybe PIC just assumes 0 if it does not get an address ???
		I2C_WriteOneByte(&hi2c2, targetAddressU16, (uint8_t) 0x55);

		// Next example:
		// Read all 128 of the bytes back from the slave PIC16F15376 starting at ee addresss "0"
		// Clear local buffer before it to know we really are reading it back.
		clearEEPROM_Buffer(); // clears the local EEPROM_Buffer, so really know new data arrived.
		RandomAccesPicEEReadBytes(&hi2c2, targetAddressU16, (uint8_t)0, EEPROM_Buffer,sizeof(EEPROM_Buffer) );

		// Next example:
		// This time actually clear the buffer on the PIC side:
		clearEEPROM_Buffer();
		// Write out a clear buffer to the PIC
		RandomAccesPicEEWriteBytes(&hi2c2, targetAddressU16, (uint8_t)0, EEPROM_Buffer,sizeof(EEPROM_Buffer));
		// Read it back
		RandomAccesPicEEReadBytes(&hi2c2, targetAddressU16, (uint8_t)0, EEPROM_Buffer,sizeof(EEPROM_Buffer));
 */

/**
 *  @brief Read one or more bytes from PIC demo board simulated eeprom
 *  <pre>
 *  Calls HAL routine: HAL_I2C_Mem_Read_IT - Which enables I2C interrupts,
 *  and initiates Dev Address write plus future EE address I2C reads over a period
 *  of time until all bytes are read.
 *  Upon all needed interrupts this will execute HAL_I2C_EV_IRQHandler
 *  and associated call backs, depending on state inside source file:
 *  "stm32f1xx_hal_i2c.c"
 *
 *  This routine is designed to work as the I2C master to a
 *  PicMPLabExpress_16F15376 demo board programmed as a slave
 *  "EEprom" simulator
 *
 *
 *  </pre>
 *
 * @param *hi2c 		Pointer to structure "I2C_HandleTypeDef".
 * @param DevAddress 	Configured PIC I2C address.
 * @param EEaddress		Random address for start of simulated eeprom.
 * @param pByteBuffer   Pointer to buffer inside STM32 to hold bytes read.
 * @param bufferLength	Number of bytes requested to be read.
 */


/* Type reference only
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
*/

void RandomAccesPicEEReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t EEaddress, uint8_t *pByteBuffer, uint8_t bufferLength) {

	// Notice we do not have to specify a timeout (of 100) like we did in the polling version.

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still doing bytes
		// from previous EERead or Write...
		//asm("nop");
		STM32vldisc_LEDToggle(LED3);
	}

	// Non blocking (interrupt based) call to read some bytes.
	I2CWriteStatus = HAL_I2C_Mem_Read_IT(hi2c, DevAddress, (uint16_t) EEaddress, (uint16_t)1, pByteBuffer, (uint16_t) bufferLength);
}


/**
 *  @brief Write one or more bytes to PIC demo board simulated eeprom
 *  <pre>
 *  This routine is designed to work as the I2C master to a
 *  PicMPLabExpress_16F15376 demo board programmed as a slave
 *  "EEprom" simulator
 *  </pre>
 *
 * @param *hi2c 		Pointer to structure "I2C_HandleTypeDef".
 * @param DevAddress 	Configured PIC I2C address.
 * @param EEaddress		Random address for start of simulated eeprom.
 * @param pByteBuffer   Pointer to buffer inside STM32 to hold bytes read.
 * @param bufferLength	Number of bytes requested to be read.
 */

void RandomAccesPicEEWriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t EEaddress, uint8_t *pByteBuffer, uint8_t bufferLength) {

	// The 1 below means that the address in the target device only uses 1 byte.

	// This while is to prevent consecutive calls to EEWrite or EERead
	// from crashing into each other, they are non blocking, but this does not
	// mean that you should call one of these before the previous is finished.

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still sending bytes..
		//asm("nop");
		STM32vldisc_LEDToggle(LED3);
	}

	// Non blocking (interrupt based) call to write some bytes.
	I2CWriteStatus = HAL_I2C_Mem_Write_IT(hi2c, DevAddress, (uint16_t) EEaddress, (uint16_t)1, pByteBuffer, (uint16_t) bufferLength);
}

//====================================================================================



