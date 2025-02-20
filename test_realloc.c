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
	puts("Test 1: Chop and free test...");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 32; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		// sprintf(str, "c[i]: %p", c[i]);
		// puts(str);
	}

	// Now deallocating some of the slots ..to free
	for (i = 10; i < 20; i++)
	{
		//Tested front coal
        sma_free(c[i]);
	}
    free_list_info();
    char *cp3 = (char *)sma_malloc(1024*2);
    cp3 = (char *)sma_realloc(cp3, 1024*1);
    if (cp3 > c[31])
        puts("\t\t\t\t PASSED\n");
    else
    {
        puts("\t\t\t\t FAILED\n");
    }
    free_list_info();
    puts("Test 2: expand test...");
    char *cp4 = (char *)sma_malloc(1024*2);
    free_list_info();
    puts("REALLOCING");
    cp4 = (char *)sma_realloc(cp4, 1024*4);
    free_list_info();
    if (cp4 > c[31])
    {
        puts("\t\t\t\t PASSED\n");
    }
    else
    {
        puts("\t\t\t\t FAILED\n");
    }
    free_list_info();
    sma_free(cp4);
    free_list_info();
    return 0;
}