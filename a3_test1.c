#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sma.h"

int main(int argc, char *argv[])
{
	int i, count = 0;
	void *ptr, *limitafter = NULL, *limitbefore = NULL;
	char *c[32], *ct;
	char str[60];
	/*
	// Test 1: Find the holes
	puts("Test 1: Hole finding test...");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 10; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		//sprintf(str, "c[%d]: %p", i, c[i]);
		//puts(str);
	}
	puts("After FML");
	freeListInfo();
    for (i = 1; i < 8; i++)
	{
		sma_free(c[i]);
		sprintf(str, "c[%d]: %p", i, c[i]);
		puts(str);
	}
	puts("After FFL");
	freeListInfo();
	sprintf(str, "c[9] = %p", c[9] );
	puts(str);
	// Allocate some storage .. this should go into the freed storage
	ct = (char *)sma_malloc(5 * 1024);
	sprintf(str, "ct: %p", ct);
	puts(str);
	freeListInfo();
	// Testing if you are finding the available holes
	if (ct < c[9])
		puts("\t\t\t\t PASSED\n");
	else
	{
		puts("\t\t\t\t FAILED\n");
		exit(0);
	}
	sma_mallinfo();
	freeListInfo();
	*/

	puts("Test 3: Check for Worst Fit algorithm...");
	// Sets Policy to Worst Fit
	sma_mallopt(WORST_FIT);

	// Allocating 512 kbytes of memory..
	for (i = 0; i < 32; i++)
	{
		c[i] = (char *)sma_malloc(16 * 1024);
		sprintf(str, "c[%d]: %p", i, c[i]);
		puts(str);
	}
		

	// Now deallocating some of the slots ..to free
	// One chunk of 5x16 kbytes
	puts("here!");
	sma_free(c[31]);
	sma_free(c[30]);
	sma_free(c[29]);
	sma_free(c[28]);
	sma_free(c[27]);
	//freeListInfo();
	
	// One chunk of 3x16 kbytes
	//puts("Freeing 3x16kb");
	sma_free(c[25]);
	sma_free(c[24]);
	sma_free(c[23]);
	//freeListInfo();
	//puts("Freed 3x16kb");
	// One chunk of 2x16 kbytes
	//puts("Freeing 2x16kb");
	sma_free(c[20]);
	sma_free(c[19]);
	//freeListInfo();
	//puts("Freed 2x16kb");
	// One chunk of 3x16 kbytes
	//puts("Freeing 3x16kb");
	sma_free(c[10]);
	sma_free(c[9]);
	sma_free(c[8]);
	//freeListInfo();
	//puts("Freed 3x16kb");
	// One chunk of 2x16 kbytes
	//puts("Freeing 2x16kb");
	sma_free(c[5]);
	sma_free(c[4]);
	freeListInfo();
	//puts("Freed 2x16kb");
	char *cp2 = (char *)sma_malloc(16 * 1024 * 2);
	//freeListInfo();
	// Testing if the correct hole has been allocated
	if (cp2 != NULL)
	{
		if (cp2 == c[27] || cp2 == c[28] || cp2 == c[29] || cp2 == c[30])
			puts("\t\t\t\t PASSED\n");
		else
			puts("\t\t\t\t FAILED\n");
	}
	else
	{
		puts("\t\t\t\t FAILED\n");
	}

	//	Freeing cp2
	sma_free(cp2);
    return 0;
}