/**
  ******************************************************************************
  * File Name          : I2C.h
  * Description        : This file supports jmk designed functions to utilize
  *                      of the I2C instances.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __i2c_jmk_H
#define __i2c_jmk_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c2;

void I2C_WriteOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data);
void I2C_WriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
void I2C_ReadOneByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Data);
void I2C_ReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,uint8_t *pData, uint16_t Size );

void RandomAccesPicEEReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t EEaddress, uint8_t *pByteBuffer, uint8_t buferLength);
void RandomAccesPicEEWriteBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t EEaddress, uint8_t *pByteBuffer, uint8_t buferLength);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ i2c_jmk_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
