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
	puts("Test 1: Hole finding test...");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 5; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		sprintf(str, "c[%d]: %p", i, c[i]);
		puts(str);
	}
	/*
    for (i = 0; i < 32; i++)
	{
		sma_free(c[i]);
		// sprintf(str, "c[i]: %p", c[i]);
		// puts(str);
	}
	*/
	sma_mallinfo();
    return 0;
}