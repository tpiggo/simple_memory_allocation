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
    puts("Test 2: Program break expansion test...");

	count = 0;
	for (i = 1; i < 5; i++)
	{
		limitbefore = sbrk(0);
		ptr = sma_malloc(1024 * 32 * i);
		limitafter = sbrk(0);
		sprintf(str, "iter %d ", i);
		puts(str);
		freeListInfo();

		if (limitafter > limitbefore)
			count++;
	}

	// Testing if the program breaks are incremented correctly
	if (count > 0 && count < 40)
		puts("\t\t\t\t PASSED\n");
	else
		puts("\t\t\t\t FAILED\n");

	puts("Free List");
	freeListInfo();

    return 0;
}