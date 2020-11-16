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
	puts("===============================");
	puts("SMA stats");
	sma_mallinfo();
	puts("===============================");
	// Test 1: Find the holes
	puts("Test 1: Hole finding test...");
	puts("Testing REPLACE ");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 32; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		sprintf(str, "c[%d]: %p", i, c[i]);
		puts(str);
	}
	// Now deallocating some of the slots ..to free
	for (i = 10; i < 18; i++)
	{
		sma_free(c[i]);
		sprintf(str, "Freeing c[%d]: %p", i, c[i]);
		puts(str);
	}
	for (i = 4; i < 8; i++)
	{
		sma_free(c[i]);
		sprintf(str, "Freeing c[%d]: %p", i, c[i]);
		puts(str);
	}
	// Allocate some storage .. this should go into the freed storage
	ct = (char *)sma_malloc(8 * 1024 + 40); 
	// Testing if you are finding the available holes
	if (ct < c[31])
		puts("\t\t\t\t PASSED\n");
	else
	{
		puts("\t\t\t\t FAILED\n");
		exit(0);
	}
	freeListInfo();
	//	Test 6: Print Stats
	puts("Test 6: Print SMA Statistics...");
	puts("===============================");
	sma_mallinfo();
    return 0;
}