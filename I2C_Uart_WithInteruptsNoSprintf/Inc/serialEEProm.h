/**
  @file serialEEProm.h
  @brief This file contains common defines and function prototypes for serialEEProm.c
<pre>
	Presently the target device is Atmel AT24C256.
	Ref: AT24C128/256 data sheets - 0670F–SEEPR–2/02

</pre>

   @author 	Joe Kuss (JMK)
   @date 	03/01/2018

*/


#ifndef SERIALEEPROM_H_
#define SERIALEEPROM_H_

#include "stm32f1xx_hal.h"

#define AT24C_PAGE_SIZE 64

typedef enum eArrayFillType
			{ FILL_0, FILL_FF, FILL_INDEX, FILL_REVERSE_INDEX }
		    enumArrayFillType;

typedef enum eAT24C_7BitAddr
			{ A0A1_00 = 0x50, A0A1_01 = 0x51, A0A1_10 = 0x52, A0A1_11 = 0x53 }
			enumAT24C_7BitAddr;

// Typedefs:
typedef struct {
  uint8_t array[AT24C_PAGE_SIZE];
  uint8_t bytesInPage;				// Can not initialize a typedef: Both  =  AT24C_PAGE_SIZE or = sizeof(pageByteArrayStruct.array) not allowed.
} pageByteArrayStruct;

// Global variables:
extern uint8_t 					eePromByteRead;
extern uint8_t 					eePromByteToBeWritten;
extern pageByteArrayStruct 		eePromWritePageBytes;
extern pageByteArrayStruct 		eePromReadPageBytes;

//########
extern pageByteArrayStruct bas1;
extern pageByteArrayStruct bas2;

void sEEPromPageBufferFill(pageByteArrayStruct* pByteArrayStruct, enumArrayFillType fillType );

HAL_StatusTypeDef sEEPromByteWrite(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer);
HAL_StatusTypeDef sEEPromBytesWrite(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer, uint16_t bufferLength);
HAL_StatusTypeDef sEEPromCurrentAddrReadByte(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint8_t *pByteRcvd);
HAL_StatusTypeDef sEEPromCurrentAddrReadBytes(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bits, uint8_t *pBytesRcvd, uint16_t expectedByteCount);
HAL_StatusTypeDef sEEPromRandomAddrByteRead(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer);
HAL_StatusTypeDef sEEPromRandomAddrReadBytes(I2C_HandleTypeDef *hi2c, enumAT24C_7BitAddr addr7Bit, uint16_t EEaddress, uint8_t *pByteBuffer, uint8_t bufferLength);

#endif /* SERIALEEPROM_H_ */
