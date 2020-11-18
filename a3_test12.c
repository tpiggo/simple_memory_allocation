/* Includes */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sma.h"

int main(int argc, char *argv[])
{
	int i, count = 0;
	void *ptr, *limitafter = NULL, *limitbefore = NULL;
	char *c[32], *ct;
	int *c2[32];
	char str[60];

    // Test 1: Find the holes
	puts("Test 1: Excess Memory Allocation...");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 32; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		// sprintf(str, "c[i]: %p", c[i]);
		// puts(str);
	}

	// Now deallocating some of the slots ..to free
	for (i = 10; i < 18; i++)
	{
		sma_free(c[i]);
		// sprintf(str, "Freeing c[i]: %p", c[i]);
		// puts(str);
	}

	// Allocate some storage .. this should go into the freed storage
	ct = (char *)sma_malloc(5 * 1024);
	// sprintf(str, "CT : %p", ct);
	// puts(str);

	// Testing if you are allocating excess memory at the end
	if (ct > c[31])
		puts("\t\t\t\t PASSED test1\n");
	else
		puts("\t\t\t\t FAILED\n");

	// Test 2: Program Break expansion Test
	puts("Test 2: Program break expansion test...");
	count = 0;
	for (i = 1; i < 40; i++)
	{
		limitbefore = sbrk(0);
		ptr = sma_malloc(1024 * 32 * i);
		limitafter = sbrk(0);

		if (limitafter > limitbefore)
			count++;
	}
	// Testing if the program breaks are incremented correctly
	if (count > 0 && count < 40)
		puts("\t\t\t\t PASSED test 2\n");
	else
		puts("\t\t\t\t FAILED\n");

    freeListInfo();

    //	Test 6: Print Stats
	puts("Test 6: Print SMA Statistics...");
	puts("===============================");
	sma_mallinfo();
    return 0;
}