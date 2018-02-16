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
#include <i2c_jmk.h>
#include "led.h"

#define SIZE_OF_ONE_BYTE	1
#define TIMEOUT_100MS		100

HAL_StatusTypeDef I2CWriteStatus;
I2C_HandleTypeDef hi2c2;

/**
 *  Wrapper for writing 1 byte via I2C.
 */
void I2C_WriteOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data)
{
	I2CWriteStatus = HAL_I2C_Master_Transmit(hi2c, DevAddress, &Data, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);
}

/**
 *  Wrapper for writing multple bytes via I2C.
 */
void I2C_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
	I2CWriteStatus = HAL_I2C_Master_Transmit(hi2c, DevAddress, pData, Size, (uint32_t)TIMEOUT_100MS);
}

/**
 *  Wrapper for reading one byte via I2C.
 */
void I2C_ReadOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data)
{
	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, DevAddress, &Data, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);
}

/**
 *  Wrapper for reading multiple bytes via I2C.
 */
void I2C_ReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,uint8_t *pData, uint16_t Size )
{
	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, DevAddress, pData, Size, (uint32_t)TIMEOUT_100MS);
}


/**
  * @brief  Returns the selected Button state, for button specified.
  * This function designed to allow multiple ports and pins, for multiple buttons.
  * @param  Button: Specifies the Button to be checked.
  * Presently there is only one choice for this enumeration.
  * @arg BUTTON_USER: USER Blue Push Button
  * @retval The Button GPIO pin value.
  */

/**
  * @brief  Returns pressed state of the blue "User" push button.
  * This function is specific for blue button on STM VL discovery board.
  * @param  Void - no parameter required.
  * @retval true - when blue button activated (pressed).
  */

/**
 *  @brief Read one or more bytes starting at selected address
 *  <pre>
 *  This routine is designed to work as the I2C master to a
 *  PicMPLabExpress_16F15376 demo board programmed to be a slave
 *  "EEprom" simulator
 *
 *
 *  </pre>
 *
 * @param *hi2c 		Pointer to structure "I2C_HandleTypeDef".
 * @param DevAddress 	Value holding configured PIC I2C address ###...
 * @param EEaddress		Random address for start of simulated eeprom.
 * @param pByteBuffer   Pointer to buffer inside STM32 to hold bytes read.
 * @param bufferLength	Number of bytes requested to be read ####.
 *
 *
 * *
 */
// Although the "I2C Read and Write" routines do what they say they are going to do,
// This is not what is required for the STM32VLDISCOVERY  to be the Master
// I2C controller for the slave PicMPLabExpress_16F15376 PIC board demo "EEprom" simulator
// So reading bytes from the PIC 16F15376are needed:

/*
 *		Requirements:
 *      From function "void I2C1_StatusCallback(..)" in file: "i2c1.c"
 *      of MCC auto-generated demo code for a I2C "MSSP" slave.

		This slave driver emulates an EEPROM Device.
    	Sequential reads from the EEPROM will return data at the next
    	EEPROM address.

    	Random access reads can be performed by writing a single byte
    	EEPROM address, followed by 1 or more reads.

    	Random access writes can be performed by writing a single byte
    	EEPROM address, followed by 1 or more writes.

    	Every read or write will increment the internal EEPROM address.

    	When the end of the EEPROM is reached, the EEPROM address will
    	continue from the start of the EEPROM.
*/
//##############################################################
// In both cases read and write we must write a first byte to the PIC
// to tell it the desired starting address of the memory range in the
// simulated EEprom.

void RandomAccesPicEEReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t EEaddress, uint8_t *pByteBuffer, uint8_t bufferLength) {

	// Notice we do not have to specify a timeout (of 100) like we did in the polling version.

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still doing bytes
		// from previous EERead or Write...
		//asm("nop");
		STM32vldisc_LEDToggle(LED3);
	}
	I2CWriteStatus = HAL_I2C_Mem_Read_IT(hi2c, DevAddress, (uint16_t) EEaddress, (uint16_t)1, pByteBuffer, (uint16_t) bufferLength);
}

HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

void RandomAccesPicEEWriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t EEaddress, uint8_t *pByteBuffer, uint8_t bufferLength) {

	// The 1 below means that the address in the target device only uses 1 byte.

	// This while is here to prevent consecutive calls to EEWrite or EERead
	// from crashing into eachother, they are non blocking, but this does not
	// mean that you should call one of these before the previous is finished.
	// May want to verify this code is correct....

	while (hi2c->State != HAL_I2C_STATE_READY)
	{
		// While transfer count from previous is not zero we are still sending bytes..
		//asm("nop");
		STM32vldisc_LEDToggle(LED3);
	}

	I2CWriteStatus = HAL_I2C_Mem_Write_IT(hi2c, DevAddress, (uint16_t) EEaddress, (uint16_t)1, pByteBuffer, (uint16_t) bufferLength);
}

//====================================================================================



