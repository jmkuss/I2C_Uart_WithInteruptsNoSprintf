/**
  ******************************************************************************
  * File Name          : I2C_jmk.c
  * Description        : This file provides jmk designed functions to utilize
  *                      HAL I2C functionality.
  ******************************************************************************
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <i2c_jmk.h>
#include "led.h"


#define SIZE_OF_ONE_BYTE	1
#define TIMEOUT_100MS		100

HAL_StatusTypeDef I2CWriteStatus;
I2C_HandleTypeDef hi2c2;

// Routines writen by Joe Kuss:==================================================================
void I2C_WriteOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data)
{

//	HAL_StatusTypeDef I2CWriteStatus;
	//##### Was
	//I2CWriteStatus = HAL_I2C_Master_Transmit(&hi2c2, DevAddress, &Data, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);
	// More properly:
	I2CWriteStatus = HAL_I2C_Master_Transmit(hi2c, DevAddress, &Data, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);
}

void I2C_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{

	I2CWriteStatus = HAL_I2C_Master_Transmit(hi2c, DevAddress, pData, Size, (uint32_t)TIMEOUT_100MS);

}

void I2C_ReadOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data)
{

	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, DevAddress, &Data, (uint16_t) SIZE_OF_ONE_BYTE, (uint32_t)TIMEOUT_100MS);

}

void I2C_ReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,uint8_t *pData, uint16_t Size ){

	I2CWriteStatus = HAL_I2C_Master_Receive(hi2c, DevAddress, pData, Size, (uint32_t)TIMEOUT_100MS);

}

//##############################################################
// Although the above routines do what they say they are going to do,
// What they do is not what the PIC board demo "EEprom" simulator
// requires. So for use with simulator on PIC these are needed:

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



