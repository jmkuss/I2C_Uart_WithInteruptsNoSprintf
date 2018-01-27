/**
  ******************************************************************************
  * File Name          : flipTheArray.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLIPTHEARRAY_H
#define __FLIPTHEARRAY_H
/* Includes --------------------------------------------------------------------*/

/* Macros --------------------------------------------------------------------*/
#define NUMBER_OF_ELEMENTS(x)  (sizeof(x) / sizeof((x)[0]))
/* Function Prototypes: --------------------------------------------------------*/

void flipIntArray( int *pArray, size_t ArraySize );
void flipCharArray(char * pCharArray, size_t ArraySize);

 #endif /* __FLIPTHEARRAY_H */
