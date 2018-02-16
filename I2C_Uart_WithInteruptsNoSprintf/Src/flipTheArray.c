/**
  @file flipTheArray.c
  @brief Functions to flip the elements in an array:
<pre>
	 // Add to main in order to test:

	 // These arrays for flip integer array demo:

	 static int intArrayEven[] = {1,2,3,4,5,6};
	 static int intArrayOdd[] = {1,2,3,4,5};
	 static int intArrayOne[] = {1};
	 static int * intArrayZero;    // is sizeof deref null ptr zero ?
	                               // no, but it is 1 which also works for us..

	  //#define NUMBER_OF_ELEMENTS(x)  (sizeof(x) / sizeof((x)[0]))

	  // Flip the Even array:
	  flipIntArray( intArrayEven, NUMBER_OF_ELEMENTS(intArrayEven)); // 6 elements
	  // Flip the Odd array:
	  flipIntArray( intArrayOdd, NUMBER_OF_ELEMENTS(intArrayOdd));   // 5
	  // Flip the 1 element array:
	  flipIntArray( intArrayOne, NUMBER_OF_ELEMENTS(intArrayOne ));   // 1
	  // Flip the 0 element array:
	  flipIntArray( intArrayZero, NUMBER_OF_ELEMENTS(intArrayZero )); // 0
</pre>

   @author 	Joe Kuss (JMK)
   @date 	2/16/2018

   \pagebreak
*/


#include "stddef.h"

/**
 * <pre>
 * Flip the order of an array of integers
 *
 * </pre>
 *
 * @param pIntArray     Pointer to starting integer in the array.
 * @param ArraySize 	Size of the array.
 *
 *
 */
void flipIntArray( int *pIntArray, size_t ArraySize )
{
     int i,HighElement;
    // Array could have size 0, 1, or greater and be even or be odd.
    // Does array size even or odd work the same way ? 
    // Lets take size of 2 or 3 to give it some thought: {0,1} or {0,1,2}.
    // Any other array size ( even or odd ) works the same.
    // Once we get up to the halfway point we can quit.
    // The array index starts at 0, 
    // For even 2/2=1-->0, therefore we go up to (ArraySize/2)-1
    // For odd 3/2=1-->0, we do not flip the odd center of the array,
    // therefore it works the same as even, we go up to (ArraySize/2)-1.
   
    // Note that if ArraySize were to be 
    // 0 or 1 this for loop will not run, which is OK.
    // Otherwise runs from i==0 to i==((ArraySize/2)-1)
       
    for (i=0;i<(ArraySize/2);i++) 
    {
        HighElement = pIntArray[ArraySize-i-1];
        pIntArray[ArraySize-i-1] = pIntArray[i];
        pIntArray[i] = HighElement;
    }
}


/**
 * <pre>
 * Flip the order of an array of characters
 *
 * </pre>
 *
 * @param pCharArray    Pointer to starting character in the array.
 * @param ArraySize 	Size of the array.
 *
 *
 */
void flipCharArray(char * pCharArray, size_t ArraySize)
{
    int i,HighElement;
   // Array could have size 0, 1, or greater and be even or be odd.
   // Does array size even or odd work the same way ?
   // Lets take size of 2 or 3 to give it some thought: {0,1} or {0,1,2}.
   // Any other array size ( even or odd ) works the same.
   // Once we get up to the halfway point we can quit.
   // The array index starts at 0,
   // For even 2/2=1-->0, therefore we go up to (ArraySize/2)-1
   // For odd 3/2=1-->0, we do not flip the odd center of the array,
   // therefore it works the same as even, we go up to (ArraySize/2)-1.

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
