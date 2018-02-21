/**
  @file strUtilities.c
  @brief String Utility - "su" functions
<pre>

</pre>

   @author 	Joe Kuss (JMK)
   @date 	11/16/2017 - Original.
   @date 	2/19/2018  - Added Doxygen comments, etc.

*/

#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include "strUtilities.h"

// Ref: https://stackoverflow.com/questions/18688971/c-char-array-initialization
/// Initialized to fill with nulls:
char suStringToFill[STR_U32_LENGTH_MAX] = "";

/**
 * @brief Add leading zeros to string representing a hex number depending on data width.
 * <pre>
 * Function assumes the original string had no leading 0's.
 * </pre>
 *
 * @param pCharArray - pointer to string (char array) being modified.
 * @param eDataWidth - selects desired data width 0 padding: su8BIT, su16BIT, su32BIT.
 *
 * String being created can be up to 10 chars max + 1 null at end.
 */
void suPadForHexDataWidth(char * pCharArray, suDataWidthType eDataWidth )
{
	int OriginalArraySize = 0;
	int ZeroCharsNeeded;
	int i = 0;
	int j = 0;

	// init  scratch string to all nulls.
	char stringScratch[STR_U32_LENGTH_MAX]  = "";

   // Determination of original string size:

   while (pCharArray[i] != '\0')
   {
	   i++;
   }
   OriginalArraySize = i;

   switch (eDataWidth)
   {
   case (su8BIT):
		ZeroCharsNeeded = (HEXCHARS_FOR_8BITS - OriginalArraySize); // 2
		break;
   case (su16BIT):
		ZeroCharsNeeded = (HEXCHARS_FOR_16BITS - OriginalArraySize); //4
		break;
   case (su32BIT):
		ZeroCharsNeeded = (HEXCHARS_FOR_32BITS - OriginalArraySize); // 8
		break;
   }

   // The end result will always be a char array with (STR_U32_LENGTH_MAX-1) chars..
   // Build up the updated string, leave the last null alone.
   for (i=0, j=0; i<(STR_U32_LENGTH_MAX-1); i++)
   {
	   if (i<ZeroCharsNeeded)
	   {
		   stringScratch[i] = '0';
	   }
	   else
	   {
		   // Start reading from the start of pCharArray, the non '0' numerical chars or NULLs
		   stringScratch[i] = pCharArray[j++];
		   // Note: j only increments here.
	   }
   }
   // Copy the updated string to the original one, assume last null is already there.
   for (i=0; i<(STR_U32_LENGTH_MAX-1); i++)
   {
	   pCharArray[i] = stringScratch[i];
   }
}

/**
 * @brief Flip the order of characters in a string.
 * <pre>
 * Note: If the string was part of a char array with trailing NULLs,
 *       only the chars up to the first null char will be flipped.
 * </pre>
 *
 * @param pCharArray - pointer to string (char array) being flipped.
 */
void suReverseOrder(char * pCharArray)
{
    int HighElement;
    int ArraySize = 0;
    int i = 0;

   // Determination of string size a.k.a. "ArraySize is
   // built into this function right here:

   while (pCharArray[i] != '\0')
   {
	   i++;
   }
   ArraySize = i;

   // Array of chars in string could have size 0, 1, or greater and be even or be odd.

   // Does array size even or odd work the same way ?
   // Lets take size of 2 or 3 to give it some thought: {0,1} or {0,1,2}.
   // Any other array size ( even or odd ) works the same.
   // Once we get up to the halfway point we can quit. (half way need not be flipped)
   // The array index starts at 0,
   // For even 2/2=1-->0, therefore we go "up to" (ArraySize/2)-1
   // For odd 3/2=1-->0, we do not flip the odd center of the array,
   // therefore it works the same as even, we go "up to" (ArraySize/2)-1.

   // Note that if ArraySize were to be
   // 0 or 1 this for loop will not run, which is OK.
   // Otherwise runs from i==0 to i==((ArraySize/2)-1)

   for (i=0;i<(ArraySize/2);i++)
   {
       HighElement = pCharArray[ArraySize-i-1];
       pCharArray[ArraySize-i-1] = pCharArray[i];
       pCharArray[i] = HighElement;
   }
}

/**
 * @brief Convert a string to a U32.
 * <pre>
 * It skips all leading whitespace and then converts string to U32 if it is valid.
 *
 * Note: This function is used instead of stdlib.h  "strtoul(..)" to convert string to U32.
 * </pre>
 *
 * @param stringToConvert - Input string
 * @param type - Conversion type, either suDECIMAL or suHEX.
 * @param u32Ptr - Pointer to u32 value of conversion.
 *
 */
bool suStringToU32(char * stringToConvert, suConversionType type, uint32_t * u32Ptr)
{

	int  i;


	char charBeingConverted;

	bool 		conversionComplete 	= false;
	bool 		failFlag	 		= true;			// Assume failure.
	uint64_t	u64Result			= 0;			// Need 64 bit for proper 32 bit deximal conv ..
	uint32_t 	u32Result 			= 0;
	uint32_t	latestCharConverted	= 0;

	// Advance past all leading white space ---------------:
	i = 0;
	while (isspace( stringToConvert[i] ) )
	{
		i++;
	}
	stringToConvert = &(stringToConvert[i]);
	// ---------------------------------------------------

	if (type == suDECIMAL)
	{
		// In the case of decimal we presently do not expect unsigned.
		// We only expect characters from 0 to 9.
		// If the number calculated can range from 0 to 0xFFFFFFFF
		// which in decimal ranges from 0 to 4,294,967,295 = maxVal
		// We do not allow for commas, actually, so we expect to see
		// 1 to 10 characters, and fail if they exceed max val

		for ( i=0; i<=10; i++) // i up to 8 allowed to check for overflow..
		{
			charBeingConverted = stringToConvert[i];

			switch (i)
			{
				case 10:
					if (charBeingConverted == '\0')
					{
						if (u64Result <= 0xFFFFFFFF)
						{
							// Last char[10] must be null to not fail
							failFlag = false;
						}
						// if > 0xFFFFFFFF, invalid number for u32 !
					}
					conversionComplete = true;
					break;
				default:
					// As coded the earlier prev cases were 10.
					// So this covers 0..9 (for loop only goes to 10)

					if (charBeingConverted == '\0') // End of string found.
					{
						conversionComplete = true;
						if (i > 0)
						{
							// We picked up at least 1 valid decimal char to convert,
							// so had something like 0 or 01 or 001, all these are
							// valid u32 numbers
							failFlag = false;
						}
					}
					else if (isdigit(charBeingConverted) == false) //  check for 0..9
					{
						// Non Numeric is end of area to convert.
						// This does not allow success if last byte string is not a number..
						conversionComplete = true;
						break;
					}
					else
					{
						// We have a valid decimal digit to add to conversion calculation:
						// Since previously validated "isdigit", Char was '0' to '9'.

						// Convert 0-9 ascii to 0-9 binary
						latestCharConverted = charBeingConverted - '0';

						// Multiply by 10 to continue to the next decimal digit:
						u64Result *= 10;
						u64Result += latestCharConverted;
					}
			}	// End switch (i)

			if (conversionComplete == true)
			{
				break; // Exit for loop early if flag says "no more"
			}
		}	// End for ( i=0; i<=10; i++)

	}
	else // type == suHEX
	{
		// string to convert is expected to have the following format:
		// 0x and then 1 to 8 more valid hex digits. "0x12345678"
		// ** If   > 8 hex digits, that is an overflow, and function will return "fail".
		// ** i.e. > 10 chars including leading "0x"
		for ( i=0; i<=10; i++) // i up to 8 allowed to check for overflow..
		{
			charBeingConverted = toupper(stringToConvert[i]); // code pig ??

			switch (i)
			{
				case 0:
					if (charBeingConverted != '0')
					{
						conversionComplete = true; // fail flag already init to true...
					}
					break;
				case 1:
					if (charBeingConverted != 'X')
					{
						conversionComplete = true;
					}
					break;
				case 10:
					if (charBeingConverted == '\0')
					{
						// Last char[10] must be null to not fail
						failFlag = false;
					}
					conversionComplete = true;
					break;
				default:
					// As coded the earlier prev cases were 0,1, and 10.
					// So this covers 2..9 (for loop only goes to 10)

					if (charBeingConverted == '\0') // End of string found.
					{
						conversionComplete = true;
						if (i > 2)
						{
							// We picked up at least 1 valid hex char to convert,
							// so had something like 0x5 or 0x05 or 0x505, all these are
							// valid u32 numbers
							failFlag = false;
						}
					}
					else if (isxdigit(charBeingConverted) == false) // code pig ??
					{
						conversionComplete = true;
						break;
					}
					else
					{
						// We have a valid hex digit to add to conversion calculation:
						// Since previously validated "isxdigit", Char is 0 to 9 or it is A to F
						if (charBeingConverted <= '9')
						{
							// Convert 0-9 ascii to 0-9 binary
							latestCharConverted = charBeingConverted - '0';
						}
						else
						{
							// Convert A-F ascii to 10 to 15 binary
							charBeingConverted = charBeingConverted - 'A'; // Now have 0 to 5
							latestCharConverted = charBeingConverted + 10; // Now have 10 to 15.
						}

						// Shift previous result to make room for latest 4 bit nibble (hex digit):
						u32Result <<= 4;
						u32Result += latestCharConverted;
					}
			}	// End switch (i)

			if (conversionComplete == true)
			{
				break; // Exit for loop early if flag says "no more"
			}
		}	// End for ( i=0; i<=10; i++)
	}	// End Else  (type == suHEX)

	if (type == suDECIMAL)
	{
		*u32Ptr = (uint32_t)u64Result;
	}
	else // type == suHEX
	{
		*u32Ptr = u32Result;
	}
	return (!failFlag);					// true if passes conversion.
}

/**
 * @brief Convert a u32 to a text string, of type DECIMAL or HEX
 * <pre>
 * A Hex string in format "0x12345678", with leading 0's expected
 * or
 * A decimal string in format 9999 with no leading 0's expected.
 *
 * This function is used instead of "sprintf" to do conversion from binary to text.
 * </pre>
 *
 * @param u32Value 			- u32 value to be converted.
 * @param type 				- Conversion type, either suDECIMAL or suHEX.
 * @param stringToCreate	- Output string
 */
void suU32ToString( uint32_t u32Value, suConversionType type, char * stringToCreate )
{
	int 	i=0;
	char 	scratchChar;
	int		j;

	// is it char * , or char ** for "stringToCreate"

	// Init all elements of "stringToCreate" to '\0'
	// so, when we stop adding printable digits the next will be null.
	for (j=0; j<STR_U32_LENGTH_MAX; j++) { stringToCreate[j] = '\0'; }

	// What is the maximum size of stringToCreate, presently:
	// For Hex we need max of 0x12345678 plus the null char at the end 	= 11 chars.
	// For Dec we need max of 4,294,967,295 no commas, plus null at end = 11 chars, also.

	if (type == suDECIMAL)
	{
		stringToCreate[i++] = (char)(u32Value % 10) + '0';
		while (u32Value)
		{
			u32Value /= 10;
			if (u32Value != 0)
			{
				stringToCreate[i++] =  (char)(u32Value % 10) + '0';
			}
//			else
//			{
//				stringToCreate[i++] = '\0'; // Already done in initialization.
//			}
		}

	}
	else // type == suHEX
	{
		scratchChar	 = (char)(u32Value % 16);

		if (scratchChar < 9)
		{
			scratchChar = scratchChar + '0';
		}
		else
		{
			scratchChar = (scratchChar - 10) + 'A';
		}

		stringToCreate[i++] = scratchChar;

		while (u32Value)
		{
			u32Value >>= 4;


			if (u32Value != 0)
			{
				scratchChar	 = (char)(u32Value % 16);

				if (scratchChar < 9)
				{
					scratchChar = scratchChar + '0';
				}
				else
				{
					scratchChar = (scratchChar - 10) + 'A';
				}
				stringToCreate[i++] = scratchChar;
			}
//			else
//			{
//				stringToCreate[i++] = '\0';
//			}

		}
	}	// End else (type == suHEX)

	// But now the array of characters looks like this:
	// d0,d1,d2,d3,0,0,0..0 but we need to reverse the order of the
	// non zero (null) terms.

	// Reverse the order of the string part of array only.
	suReverseOrder(stringToCreate);

	// For hex, pad in extra zeros since we want to make clear number is u32 address or data
	if (type == suHEX)
	{
		suPadForHexDataWidth(stringToCreate, su32BIT );
	}

}

