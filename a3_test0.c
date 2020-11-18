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
	// Test 1: Find the holes
	puts("Test 1: Excess Memory Allocation...");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 32; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		//sprintf(str, "c[%d]: %p", i, c[i]);
		//puts(str);
	}
    freeListInfo();
	//	Test 6: Print Stats
	puts("Test 6: Print SMA Statistics...");
	puts("===============================");
	sma_mallinfo();
    return 0;
}