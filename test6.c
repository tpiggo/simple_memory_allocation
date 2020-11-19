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
    sma_free(c[6]);
    sma_free(c[8]);
    sma_free(c[7]);
    freeListInfo();


    return 0;
}