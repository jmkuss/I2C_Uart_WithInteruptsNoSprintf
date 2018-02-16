/**
  @file serialCmdParser.c
  @brief Serial Command processor
<pre>
  This is a generic command processor.
  These commands are kept as as simple as possible, mildly cryptic,
  but in easily expandable format to allow rapid implementation and
  modification.

  Table lookup can interpret these into English readable.

  c[x] - "do this" command 					(may include parameters)
  s[x] - "status set"   		        	(no parameters, read only)
  d[x] - "data set", R/W 					(when W command is followed by = data.)
  m[xxxx] - "Generic I/O,SFR, memory, R/W   (when W command is followed by = uint8_t.)
  h[x] - "Display a help page"				(Page# = x)

  h - A single "h" or "H" halts (and resets) the system.

  Notes:

  x 	= uint_8t , index range 0 to 255.
  xxxx	= uint_32t, address range 0x00000000 to 0xFFFFFFFF

  Codes c,s and d are made more specific by adding [x].
  Exactly what they do will be determined by enumeration of [x].

  Code m directly reads/writes the address space of the processor,
  (possibly with some restrictions).

  "help y[x]" returns usage page for command y[x].

  h[x] displays sections of complete command set manual,
  presently this is only h[1].
  </pre>

   @author 	Joe Kuss (JMK)
   @date 	2/14/2018

   \pagebreak
*/


#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "uart_jmk.h"
#include "strUtilities.h"

// Delay counter
#define DELAY_COUNT   500000

extern bool Cmd_Recieved;

/// Command Response Codes Enumeration.
enum commandResponse
{
	eOK,
	eUnknownCmd,
	eGotC,
	eGotD,
	eGotH,
	eGotM,
	eUintExpected,
	eNoFurtherComment,
	eAttachReadOnly,
	eIndexRangeExceeded,
	eIndexError
};
typedef enum commandResponse eCOMMAND_RESPONSE;

/// Holds latest command response
eCOMMAND_RESPONSE cmdResponse;


#define strEQUALITY 0

// Ref:
// https://stackoverflow.com/questions/797318/how-to-split-a-string-literal-across-multiple-lines-in-c-objective-c
/// Help page displayed on terminal in response to "h[1]"
char  * help_1_str = "\r\n"
                "Help page #1 - Command Interpreter Overview: ----------- \r\n\r\n"
                "c[x]    - \"Execute Command #x\"       (may include parameters)\r\n"
                "s[x]    - \"Read a Status Set\"        (no parameters, read only)\r\n"
                "d[x]    - \"Data Set\", R/W            (if W, command followed by = data.)\r\n"
                "m[xxxx] - \"I/O,SFR,Mem.,@[addr],\"R/W (if W, command followed by = uint8_t.)\r\n"
                "h[x]    - \"Display help page\"        (Page# = x)\r\n"
                "\r\n"
                "h       - \"h\" or \"H\" halts (and resets) the system.\r\n"
                "\r\n"
                "Notes:\r\n"
                "\r\n"
                "x 	= uint_8t , index range 0 to 255.\r\n"
                "xxxx	= uint_32t, address range 0x00000000 to 0xFFFFFFFF\r\n"
                "\r\n"
                "Codes c,s and d are made more specific by adding [x].\r\n"
                "Exactly what they do will be determined by enumeration of [x].\r\n"
                "\r\n"
                "Code m directly reads/writes the address space of the processor,\r\n"
                "(possibly with some restrictions).\r\n"
                "------------------------------------------------------------- \r\n\r\n";

\pagebreak
/**
 * <pre>
 * Scans MainString, and returns pointer to
 * start of Data section of message via subString.
 * Data section is delimited by the "=" symbol.
 * Will return false if can not find data section
 * (characters) following a single "=".
 * </pre>
 *
 * @param subString 	Points to start of data section.
 * @param MainString 	Points to start of input string.
 * @retval 				True if found "=" indicating data.
 *
 */
// We use char ** because we want to output the result
// to an external variable that is char*, in order to do
// this we must use a pointer to char*, thus char**
bool getDataInputString(char **subString, char * MainString)
{
	char * findItPtr;
	size_t strLen1;

	// Get length of substring delimited by "="
	strLen1 = strcspn(MainString, "=");

	if (strLen1 != strlen(MainString))
	{
		// Delimiter "=" found, the substring is shorter:

		strLen1++; // to skip past the "="
		findItPtr = &(MainString[strLen1]);
	}
	else
	{
		// delimiter "=" not found,
		findItPtr = NULL;
	}

	// Reject for no "="
	if (findItPtr == NULL)
	{
		// Just as a backstop (or we could return NULL...)
		*subString = MainString;
		return false;
	}
	else
	{
		// Q: Do this dereference just because variable
		// was defined as char ** ?
		// A: No, it is done because we want the parameter
		//    To of an "out" type that can write to an external
		//    variable location of type char* from this function.
		*subString = findItPtr;
		return true;
	}
}

// FUNCTION:	bool convStringToUint(uint_32t *uInt32Ptr, char *strPtr) ===
/**
 * <pre>
 * Convert  string via "strPtr" to  uint_32t if possible.
 * Return false if string is invalid format for uint_32t
 *
 * Numerical value is returned via pointer "uInt32Ptr"
 *</pre>
 * Will be able to, also, interpret the hex 0x00000 format, and accept
 * 8, 16, and 32 bit hex values
 *
 * @param uInt32Ptr 	External location of uint_32t result.
 * @param strPtr		String to convert, if possible.
 * @retval 				True if string holds valid uint_32t.
 *
 */
bool convStringToUint(uint32_t *uInt32Ptr, char *strPtr)
{
	unsigned long ulData;		// Note Arm compiler says uint_32t same as ulong.
	bool   bValidStringConversion;

	//### using "stroul"
	//ulData = strtoul(strPtr, &strPtr, 0); // "0" = base can be dec,oct or hex, nice !

	// Using "suStringToU32" assume string was HEX
	bValidStringConversion = suStringToU32(strPtr, suHEX, &ulData);

	if (bValidStringConversion == false)
	{
		// If not Hex, try decimal..
		bValidStringConversion = suStringToU32(strPtr, suDECIMAL, &ulData);
	}


	*uInt32Ptr = ulData;	// put the result in the loc pointed to.


	return bValidStringConversion;
}


// Function:	bool getU32Index(uint32_t *numberPtr, char *strPtr) ==========
/**
 * <pre>
 * Scan a string and convert the index xxxx located in between
 * the delimiters [ and ], into a numeric uint32_t.
 *
 * This index may be represented as hex: 0x0 to 0xFFFFFFFF or as
 * an unsigned decimal up to 32 bits.
 *
 * If no index exists or improper numerical format (to large etc)
 * then function will return false.
 * </pre>
 *
 * @param uInt32Ptr 	External location of uint_32t result.
 * @param strPtr		String to convert, if possible.
 * @retval 				True if converted index into valid uint_32t.
 *
 */
bool getU32Index(uint32_t *uInt32Ptr, char *strPtr)
{
	char separator1[] = "[";
	char separator2[] = "]";
	unsigned long  ul;
	bool bValidStringConversion;
	size_t strLen1;
	size_t strLen2;
	char savedChar;
	char *ptr;

	bValidStringConversion = false;

	// Get number of chars (span) before "["
	// this will be full length of string if do not see "["
	strLen1 = strcspn(strPtr, separator1);

	if (strLen1 != strlen(strPtr))
	{
		// did see the first '['.

		// "+1" to skip '['
		strLen1++;
		strLen2 = strcspn(&(strPtr[strLen1]), separator2);

		if (strLen2 != strlen( &(strPtr[strLen1]) ))
		{
			// did see the second ']'
			// Now we know substring spans from
			// strPtr[strLen1] to strPtr[strLen1 + strLen2 - 1]

			// We expect to see digit right away after the '[', this makes
			// coding simpler and less bug prone..

			if ( isdigit(strPtr[strLen1]) )
			{
				// Save char where the next delimiter is: ']'
				savedChar = strPtr[strLen1+strLen2];
				// Put a null there to facilitate strtoul:
				strPtr[strLen1+strLen2] = '\0';

				ptr = &(strPtr[strLen1]);

				//ul = strtoul(ptr, &ptr, 0);

				// Using "suStringToU32" assume string was HEX

				bValidStringConversion = suStringToU32(ptr, suHEX, &ul);

				if (bValidStringConversion == false)
				{
					// If not Hex, try decimal..
					bValidStringConversion = suStringToU32(ptr, suDECIMAL, &ul);
				}


				*uInt32Ptr = ul;

				// Restore char where the end of index delimiter is s.b. ']'
				strPtr[strLen1+strLen2] = savedChar;
			}
		}
	}

	return bValidStringConversion;
}

// Function:	bool getU8Index(uint8_t *uInt8Ptr, char *strPtr) ==========
/**
 * <pre>
 * Scan a string and convert the index xx located in between
 * the delimiters [ and ], into a numeric uint8_t.
 *
 * This index may be represented as hex: 0x0 to 0xFF or as
 * an unsigned decimal up to 8 bits.
 *
 * If no index exists or improper numerical format (to large etc)
 * then function will return false.
 * </pre>
 *
 * @param uInt8Ptr 		External location of uint_8t result.
 * @param strPtr		String to convert, if possible.
 * @retval 				True if converted index into valid uint_8t.
 *
 */
bool getU8Index(uint8_t *uInt8Ptr, char *strPtr)
{
	char separator1[] = "[";
	char separator2[] = "]";
	unsigned long  ul;
	bool result;
	bool bValidStringConversion;
	size_t strLen1;
	size_t strLen2;
	char savedChar;
	char *ptr;

	result = false;

	// Get number of chars (span) before "["
	// this will be full length of string if do not see "["
	strLen1 = strcspn(strPtr, separator1);

	if (strLen1 != strlen(strPtr))
	{
		// did see the first '['.

		// "+1" to skip '['
		strLen1++;
		strLen2 = strcspn(&(strPtr[strLen1]), separator2);

		if (strLen2 != strlen( &(strPtr[strLen1]) ))
		{
			// did see the second ']'
			// Now we know substring spans from
			// strPtr[strLen1] to strPtr[strLen1 + strLen2 - 1]

			// We expect to see digit right away after the '[', this makes
			// coding simpler and less bug prone..

			if ( isdigit(strPtr[strLen1]) )
			{
				// Save char where the next delimiter is: ']'
				savedChar = strPtr[strLen1+strLen2];
				// Put a null there to facilitate strtoul:
				strPtr[strLen1+strLen2] = '\0';

				// We are assured strtoul will be successful by isdigit..
				// ( ###  Assumes that chars after first digit are also digits.. )
				// ( ###  Any value > 0xFF will be rejected. )
				ptr = &(strPtr[strLen1]);

				//ul = strtoul(ptr, &ptr, 0);

				// Using "suStringToU32" assume string was HEX

				bValidStringConversion = suStringToU32(ptr, suHEX, &ul);

				if (bValidStringConversion == false)
				{
					// If not Hex, try decimal..
					bValidStringConversion = suStringToU32(ptr, suDECIMAL, &ul);
				}

				if ((ul <= 255) && (bValidStringConversion == true))
				{
					*uInt8Ptr = (uint8_t) ul;
					result = true;
				}
				// Restore char where the end of index delimiter is s.b. ']'
				strPtr[strLen1+strLen2] = savedChar;
			}
		}
	}
		return result;
}


// The received bytes are picked up by ISR, and handled by the callback
// routine "HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)", in uart_jmk.c
// This routine will flag "Transfer_cplt" which occurs every time terminal
// sends a CR 0x0D charactor.

/**
 * <pre>
 * Interpret and execute the latest command string, from the input terminal.
 * This will also send back the appropriate response for the particular
 * command.
 * </pre>
 *
 * @param cmdStr		Command string to parse, interpret, execute.
 *
 */

void cmdHandler(char * cmdStr)
{
	bool	isU8Index;
	bool	isU32Index;
	bool	isInputDataStr;
	bool	isUintData;

	unsigned int strLength;
	unsigned int cmdLength;

	uint32_t 	uIntData;
	uint32_t	index,i;

	char * dataStrPtr;

    char letter1;
    char cmdToken[] = "12345";

    cmdResponse = eOK;

	strLength = strlen(cmdStr);

	/**
	 * <b>Flow of Control - </b> <br>
	 * */
	///
	/// <pre>Get numerical index in command string "cmd[index]", if it exists:
	/// Typically the index is expected to be < 256. </pre>

	isU32Index = getU32Index(&index, cmdStr);
	isU8Index  = ((index <= 255) && (isU32Index == true));



	/// <pre>Get the length of cmdToken "cmd" - max 5 chars.., min 0 chars.
	/// <em>Note: If "[" is not seen then cmdLength will be same as command string
	/// If length of cmdToken is > 5 we look at only first 5 chars, max.</em> </pre>
	cmdLength = strcspn(cmdStr, "[");

	if (cmdLength > 5) cmdLength = 5;
	if (cmdLength)
	{
		for (i=0; i<cmdLength; i++)
		{
			cmdToken[i] = toupper(cmdStr[i]); // make all upper case..
		}
		// Add in the null char to end the string..
		cmdToken[i] = '\0';
	}

	letter1 = cmdToken[0];

	//-----------------------------------------------------------

	/// <pre>Get Str pointer for input data after single equals sign, if it exists.
	/// <em> Example: for d[xxx]=yyyy, we provide input data "yyyy" </em> </pre>
	isInputDataStr = getDataInputString(&dataStrPtr, cmdStr);


	// See if this data qualifies for unsigned integer data (32 bit unsigned)
	/// If input data exists, then attempt to convert it to numeric.
	if (isInputDataStr)
	{
		isUintData = convStringToUint(&uIntData, dataStrPtr);
	}


	/// <pre>cmdToken is first 5 chars of command string, at most.
	/// i.e. chars that come before the [xx], if index exists.</pre>

	// Assume default..
	cmdResponse = eUnknownCmd;

	/// <pre>Look at first char of cmdToken
	/// For valid first char, parse out the command and what to do with it. <br></pre>
	/**
	 * <b> Flow of Control - Parse Cases:</b> <br>
	 */
	switch (letter1)
	{
		/// 'C[xx]' indicates a command "xx".
		case 'C':
			if (cmdLength == 1)
			{
				cmdResponse = eIndexError; // assume worst, hope  best:
				if ( isU8Index  )
				{
					// with no data to be input (written)
					// Read out the current data

					// ### Previously used sprintf to convert u32 to decimal value text
					// sprintf(respBuffer, "Received C[%u]\r\n", (unsigned int)index);

					// Now convert u32 to string and build up a total string as response:
					suU32ToString( index, suDECIMAL, stringToFill );
					strcpy(respBuffer, "Command requested: C[");
					strcat(respBuffer,stringToFill);
					strcat(respBuffer, "]\r\n");


					cmdResponse = eNoFurtherComment;
					//cmdResponse = eGotC;
				}
				else if (isU32Index)
				{
					cmdResponse = eIndexRangeExceeded;
				}

			}
			break;

		/// 'D[xx]' indicates a data set read/write.
		case 'D':
			if (cmdLength == 1)
			{
				cmdResponse = eIndexError;
				if ( isU8Index )
				{
					// with no data to be input (written)
					// Read out the current data
					//sprintf(respBuffer, "Received D[%u]\r\n", (unsigned int)index);

					// Now convert u32 to string and build up a total string as response:
					suU32ToString( index, suDECIMAL, stringToFill );
					strcpy(respBuffer, "Data set requested: D[");
					strcat(respBuffer,stringToFill);
					strcat(respBuffer, "]\r\n");

					cmdResponse = eNoFurtherComment;
					//cmdResponse = eGotD;
				}
				else if (isU32Index)
				{
					cmdResponse = eIndexRangeExceeded;
				}

			}
			break;

		/// 'H[xx]' indicates help page request. <br>
		/// 'H' by itself indicates, system halt/restart request.
		case 'H':	// "help" or "halt" ?
			if (strLength == 1)
			{
				// Halt command !
				strcpy(respBuffer, "System Halted !\r\n");
				cmdResponse = eNoFurtherComment;
			}
			else if (cmdLength == 1)
			{
				cmdResponse = eIndexError; // assume worst, hope  best:
				if ( isU8Index  )
				{
					//
					if (index == 1)
					{
					  // This is h[1] , our very first help page !
					  strcpy(respBuffer,help_1_str);
					}
					else
					{
						// with no data to be input (written)
						// Read out the current data
						//sprintf(respBuffer, "Received H[%u]\r\n", (unsigned int)index);
						// Now convert u32 to string and build up a total string as response:
						suU32ToString( index, suDECIMAL, stringToFill );
						strcpy(respBuffer, "Help page requested: H[");
						strcat(respBuffer,stringToFill);
						strcat(respBuffer, "]");
						strcat(respBuffer, " - does not exist !\r\n");
					}

					cmdResponse = eNoFurtherComment;
					//cmdResponse = eGotH;
				}
				else if (isU32Index)
				{
					cmdResponse = eIndexRangeExceeded;
				}
			}
			break;

		/// 'S[xx]' indicates status of type "xx" request.
		case 'S':	// "Status", "Streaming Status" ?
			if (cmdLength == 1)
			{	// Must be the S command.
				cmdResponse = eIndexError; // assume worst, hope  best:
				if ( isU8Index  )
				{
					// with no data to be input (written)
					// Read out the current data
					//sprintf(respBuffer, "Received status request: S[%u]\r\n", (unsigned int)index);
					// Now convert u32 to string and build up a total string as response:
					suU32ToString( index, suDECIMAL, stringToFill );
					strcpy(respBuffer, "Status requested: S[");
					strcat(respBuffer,stringToFill);
					strcat(respBuffer, "]\r\n");
					cmdResponse = eNoFurtherComment;

				}
				else if (isU32Index)
				{
					cmdResponse = eIndexRangeExceeded;
				}
			}
			/// 'SS[xx]' indicates  streaming (repeating) status of type "xx" request.
			else if (strcmp(cmdToken, "SS") == strEQUALITY)
			{
				cmdResponse = eIndexError; // assume worst, hope  best:
				if ( isU8Index  )
				{
					// with no data to be input (written)
					// Read out the current data
					//sprintf(respBuffer, "Received streaming status request: SS[%u]\r\n", (unsigned int)index);
					// Now convert u32 to string and build up a total string as response:
					suU32ToString( index, suDECIMAL, stringToFill );
					strcpy(respBuffer, "Streaming status requested: SS[");
					strcat(respBuffer,stringToFill);
					strcat(respBuffer, "]\r\n");
					cmdResponse = eNoFurtherComment;
				}
				else if (isU32Index)
				{
					cmdResponse = eIndexRangeExceeded;
				}
			}
			break;
		/// 'M[xxxxxxxx]' or 'M[xx..]=yy..' => Memory/IO/SFR read or write.
		case 'M':	// Report any 4 bytes worth of memory/IO/SFR's
			if (cmdLength == 1)
			{	// Must be the M command.
				cmdResponse = eIndexError; // assume worst, hope  best:
				if ( isU32Index  )
				{
					uint32_t *MemorySpacePtr;
					MemorySpacePtr = (uint32_t *)index;
					// sprintf(respBuffer, "Memory M[0x%08X]", (unsigned int)index);
					// Now convert u32 to string and build up a total string as response:
					suU32ToString( index, suHEX, stringToFill );
					strcpy(respBuffer, "Memory/IO query: M[0x");
					strcat(respBuffer,stringToFill);
					strcat(respBuffer, "]");

					// Translate what we read via MemorySpacePtr, to hex string
					//sprintf(strOf20CharsMax, " = 0x%08X \r\n", (unsigned int)*MemorySpacePtr);
					suU32ToString( (unsigned int)*MemorySpacePtr, suHEX, stringToFill );
					strcat(respBuffer, " = 0x");
					strcat(respBuffer,stringToFill);
					strcat(respBuffer, "\r\n");

					//strcat(respBuffer,strOf20CharsMax);

					cmdResponse = eNoFurtherComment;
					//cmdResponse = eGotM;
				}
			}
			break;
	}

    switch (cmdResponse)
    {
    case eOK:
        strcpy(respBuffer, "Ok\r\n");
    	break;
    case eUnknownCmd:
    	strcpy(respBuffer, "Unknown command !\r\n");
    	break;
    case eGotC:
        strcpy(respBuffer, "Stub cmd C, Ok\r\n");
    	break;
    case eGotD:
        strcpy(respBuffer, "Stub cmd D, Ok\r\n");
    	break;
    case eGotH:
        strcpy(respBuffer, "Stub cmd H, Ok\r\n");
    	break;
    case eGotM:
        strcpy(respBuffer, "Stub cmd M, Ok\r\n");
    	break;
    case eNoFurtherComment:
        break;
    case eAttachReadOnly:
    	strcat(respBuffer, " is read only..");
		break;
    case eUintExpected:
        strcpy(respBuffer, "Unsigned int format expected.\r\n");
    	break;
    case eIndexRangeExceeded:
        strcpy(respBuffer, "Index range Exceeded.\r\n");
    	break;
    case eIndexError:
    	strcpy(respBuffer, "Index Expected.\r\n");
    	break;
    }

      //UartPutString(crlf_msg, false);

      /// <br> Finally send response based on what command received back to terminal.
      UartPutString(respBuffer, false);
      ///  \pagebreak
      //UartPutString(crlf_msg, false);
}
