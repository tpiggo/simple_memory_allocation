/* Includes */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sma2.h"

int main(int argc, char *argv[])
{
	int i, count = 0;
	void *ptr, *limitafter = NULL, *limitbefore = NULL;
	char *c[32], *ct;
	int *c2[32];
	char str[60];

	// Test 1: Find the holes
	puts("Test 1: Excess As Memory Allocation...");

	// Allocating 32 kbytes of memory..
	for (i = 0; i < 32; i++)
	{
		c[i] = (char *)sma_malloc(1024);
		// sprintf(str, "c[i]: %p", c[i]);
		// puts(str);
	}

	// Now deallocating some of the slots ..to free
	for (i = 10; i < 13; i++)
	{
		//Tested front coal
        sma_free(c[i]);
		// sprintf(str, "Freeing c[i]: %p", c[i]);
		// puts(str);
	}
    free_list_info();
	// Allocate some storage .. this should go into the freed storage
	ct = (char *)sma_malloc(3 * 1024);
    if (ct == c[10])
        puts("\t\t\t\t PASSED \n");
    else
    {
        puts("\t\t\t\t FAILED \n");
    }
    
    free_list_info();
    sma_free(c[15]);
    sma_free(ct);
    //Tests double Coal
    sma_free(c[8]);
    sma_free(c[9]);
    free_list_info();


    puts("Test 3: Check for Worst Fit algorithm...");
	// Sets Policy to Worst Fit
	sma_mallopt(WORST_FIT);
    // Allocating 512 kbytes of memory..
	for (i = 0; i < 32; i++)
    {
        c2[i] = (int *)sma_malloc(16 * 1024);
        sprintf(str, "c2[%d]: %p", i, c2[i]);
		puts(str);
    }

    free_list_info();
    int totalFreed = 0;
    for (int i = 20; i<32; i++)
    {
        sma_free(c2[i]);
        totalFreed += 16*1024;
    }
    free_list_info();
    sprintf(str, "totalFreed = %d, sbrk=%p", totalFreed, sbrk(0));
    puts(str);
    return 0;
}