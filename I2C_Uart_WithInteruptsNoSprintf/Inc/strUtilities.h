/*
 * strUtilities.h
 *
 *  Created on: Nov 16, 2017
 *      Author: jmk
 */

#ifndef STRUTILITIES_H_
#define STRUTILITIES_H_

// What is the maximum size of stringToCreate, presently:
// For Hex we need max of 0x12345678 plus the null char at the end 	= 11 chars.
// ===> Actually for hex we do not insert the 0x directly into sringToFill,
// ===> So sometimes we use a different define: 8 hex chars + 1 null:
#define STR_U32_HEX_MAX 9

// For Dec we need max of 4,294,967,295 no commas, plus null at end = 11 chars, also.

#define STR_U32_LENGTH_MAX	11



typedef enum {suDECIMAL,suHEX} suConversionType;

typedef enum {su8BIT, su16BIT, su32BIT} suDataWidthType;

extern char stringToFill[STR_U32_LENGTH_MAX];	// Buffer for string to create.

/* ---------- Function Prototypes ------------------------------------ */

// These two assume we are manipulating extern "stringToFill"
void suPadForDataWidth(char * pCharArray, suDataWidthType eDataWidth );
void suReverseOrder(char * pCharArray);

bool suStringToU32(char * stringToConvert, suConversionType type, uint32_t * u32Ptr);
void suU32ToString( uint32_t u32Value, suConversionType type, char * stringToCreate );


#endif /* STRUTILITIES_H_ */
