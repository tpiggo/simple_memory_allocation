
/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes

typedef struct __free_block_head{
	int size;
	struct __free_block_head *next;
	struct __free_block_head *prev;
} free_block_head_t;

//	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
#define BLOCK_HEADER sizeof(int)
#define POINTERS 2* sizeof(char *)
//	TODO: Add constants here

typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

char ps[50];
char *sma_malloc_error;
void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
int minFreeSize = sizeof(free_block_head_t) + BLOCK_HEADER;
void *nextFitStart;
int nextFitSet = 0;
void *heapStart = NULL;
void *heapEnd = NULL;
int maxpBrk = 64;

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
	void *pMemory = NULL;

	// Checks if the free list is empty
	if (freeListHead == NULL)
	{
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{
		// Allocate memory from the free memory list

		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{
			// Allocate memory by increasing the Program Break
			// freelist size = 128 - freelist
			pMemory = allocate_pBrk(size);
		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}
	// Updates SMA Info
	totalAllocatedSize += size;

	return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_freeList(ptr);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.
	if (ptr  == NULL || size < 0)
	{
		sprintf(sma_malloc_error, "Problem in realloc! Too small or ptr is NULL");
		return NULL;
	}
	void *retBlock = NULL;
	// Checks if the block of memory to the right is free
	void *block = (void *)((int *)ptr - 1);
	int ptrSize = ((free_block_head_t *)block)->size;
	if (ptrSize % 2  != 0)
		ptrSize--;

	if (size > ptrSize + minFreeSize)
	{
		void *nextBlock = (void *)((char *)ptr + ptrSize + BLOCK_HEADER);
		if (nextBlock != heapEnd && *(int *)nextBlock % 2 == 0 &&  *(int *)nextBlock+ptrSize > size)
		{
			int excessSize = ptrSize + *(int *)nextBlock - size;
			retBlock = expand_block(block, (void *)nextBlock, size, excessSize);
		}
		else
		{
			retBlock = sma_malloc(size);
			memcpy(retBlock, ptr, ((free_block_head_t *)ptr)->size);
			sma_free(ptr);
		}
	}
	else
	{
		// Need to chop
		chop_and_add(block, size, ptrSize);
		retBlock = (char *)block + BLOCK_HEADER;
	}
	return retBlock;

}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	
	if(size % 8 != 0)
		size += (8 - size %8);
	void *newBlock = NULL;
	int excessSize;
	int extraSize = 1024 * maxpBrk + 2 * BLOCK_HEADER;
	if (size > extraSize)
		excessSize = size;
	else
		excessSize = size + extraSize;
	int totalSize = excessSize + 2 * BLOCK_HEADER;
	if (heapStart == NULL)
		heapStart = sbrk(0);
	newBlock = sbrk(totalSize);
	heapEnd = sbrk(0);
	if (newBlock == NULL)
	{
		sprintf(sma_malloc_error, "Error allocating\n");
		return NULL;
	}
	int *blockFront = (int *)((char *)newBlock - BLOCK_HEADER);
	// If not null cast it to a free_block_head type
	((free_block_head_t *)newBlock)->next = NULL;
	((free_block_head_t *)newBlock)->prev = NULL;
	((free_block_head_t *)newBlock)->size = excessSize;
	// move the pointer to the end and set tail tag
	int *tailTag = (int *)((char *)newBlock + excessSize - BLOCK_HEADER);
	*tailTag = excessSize;
	excessSize = excessSize - size;
	if ((void *)blockFront > heapStart && *blockFront % 2 == 0)
	{
		newBlock = front_coalescence(newBlock, *blockFront);
		remove_block_freeList(newBlock);
		totalFreeSize += totalSize;
		totalSize = *(int *)newBlock;
		excessSize = totalSize- size;
	}
	//	Allocates the Memory Block by sending the whole newBlock to get allocated accordingly.
	allocate_block(newBlock, size, excessSize, 0);
	return (char *)newBlock + BLOCK_HEADER;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;
	int minSize = sizeof(free_block_head_t) + BLOCK_HEADER;
	if(size % 8 != 0)
		size += (8 - size %8);

	void *ptr = freeListHead;
	excessSize = 0;
	
	while (ptr != NULL)
	{
		int ptrSize = ((free_block_head_t *)ptr)->size;
		if (ptrSize > minSize && ptrSize >= size && ptrSize-size > excessSize)
		{
			worstBlock = ptr;
			excessSize = ptrSize-size;
			blockFound = 1;
		}
		ptr = ((free_block_head_t *)ptr)->next;
	}
	//	Checks if appropriate block is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
		worstBlock = (void *)((char *)worstBlock + BLOCK_HEADER);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}

	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;
	if(size % 8 != 0)
		size += (8 - size %8);
	
	if (nextFitSet == 0)
	{
		nextFitStart = freeListHead;
	}
		
	nextFitSet = 1;
	free_block_head_t *listEntry = (free_block_head_t *)nextFitStart;
	if (size % 8 != 0)
		size += 8 - size % 8;

	while (blockFound == 0)
	{	
		if (listEntry->size >= size)
		{
			blockFound = 1;
			nextBlock = (void *)listEntry;
			excessSize = listEntry->size - size;
			nextFitStart = listEntry;
		} 
		else 
		{
			if (listEntry->next != NULL)
				listEntry = listEntry->next;
			else
				listEntry = freeListHead;
			if (listEntry == nextFitStart)
				break;
		}
	}


	if (blockFound)
	{
		allocate_block(nextBlock, size, excessSize, 1);
		nextBlock = (void *)((char *)nextBlock + BLOCK_HEADER);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}

	return nextBlock;

	return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation
	addFreeBlock = excessSize > minFreeSize;
	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
		// Here we need to allocate, and potentially split, the free block
		// Set the tail
		excessSize -= 2 * BLOCK_HEADER;
		int *tailTag = newBlock + size + BLOCK_HEADER;
		*tailTag = size + 1;

		excessFreeBlock = newBlock + size + 2 * BLOCK_HEADER;
		((free_block_head_t *)excessFreeBlock)->size =  excessSize;
		((free_block_head_t *)newBlock)->size = size;
		// Set the split blocks tails
		tailTag = excessFreeBlock + excessSize + BLOCK_HEADER;
		*tailTag = excessSize;

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
			//	Adds excess free block to the free list
			excessFreeBlock = (void *)((char *)excessFreeBlock + BLOCK_HEADER);
			add_block_freeList(excessFreeBlock);
		}
		// set the front header
		((free_block_head_t *)newBlock)->size++;
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	TODO: Add excessSize to size and assign it to the new Block
		if (excessSize > 0)
			size += excessSize;
		int *tailTag = newBlock + size + BLOCK_HEADER;
		*tailTag = size + 1;
		((free_block_head_t *)newBlock)->size = size;
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}

		((free_block_head_t *)newBlock)->size ++;
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	//	TODO: Replace the old block with the new block
	free_block_head_t *nBlock = (free_block_head_t *)newBlock;
	free_block_head_t *oBlock = (free_block_head_t *)oldBlock;
	nBlock->prev = oBlock->prev;
	nBlock->next = oBlock->next;
	
	if (oBlock->prev != NULL)
		oBlock->prev->next = newBlock;
	if (oBlock->next != NULL)
		oBlock->next->prev = newBlock;
	// Clear these in order to give this memory space back to the user
	oBlock->prev = NULL;
	oBlock->next = NULL;

	if (oldBlock == freeListHead)
	{
		// Swap head of the list if it is the head of the list
		freeListHead = newBlock;
	}

	if (oldBlock == freeListTail)
	{
		// Swap tail of the list if it is the tail of the list
		freeListTail = newBlock;
	}

	if (nextFitStart == oldBlock)
	{
		nextFitStart = newBlock;
	}
	//	Updates SMA info
	totalFreeSize -= oBlock->size;
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
	int flag = 0;
	// Coming from free, the block was in use, therefore, we need to reset the length and reset flag
	block = (void *)((char *)block - BLOCK_HEADER);
	if ((*(int *)block % 2) != 0)
	{
		*(int *)block = *(int *)block - 1;
		void *ptr = block + BLOCK_HEADER + *(int *)block;
		*(int *)ptr = *(int *)ptr - 1;
		flag = 1;
	}
	int blockSize = *(int *)block;
	//	Updates SMA info
	if (flag == 1)
	{
		totalAllocatedSize -= blockSize;
	}
	totalFreeSize += blockSize;
	// Edge cases: No list
	if (freeListHead == NULL)
	{
		freeListHead = block;
		freeListTail = block;
	}
	// It is not the only thing in memory, add it to the end, but check if the last element is beside you
	// If it is, merge.
	else
	{	
		void *currentBlock = block;
		int *blockBefore = (int *)((char *)block - BLOCK_HEADER);
		int *blockAfter = (int *)((char *)block + 2 * BLOCK_HEADER + *(int *)block);
		if ((void *)blockBefore > heapStart && (void *)blockAfter < heapEnd)
		{
			if (*blockBefore % 2 == 0 && *blockAfter % 2 == 0 && *blockAfter != 0 && *blockBefore != 0)
			{
				block = front_coalescence(block, *blockBefore);
				remove_block_freeList(block);
				block = rear_coalescence(block, *blockAfter);
			}
			else if (*blockBefore % 2 == 0 && *blockBefore != 0)
			{
				block = front_coalescence(block, *blockBefore);
			} 
			else if (*blockAfter % 2 == 0 && *blockAfter != 0)
			{
				block = rear_coalescence(block, *blockAfter);
			}
			else
			{
				add_sorted_list(block);
			}
		}
		else if(*blockBefore % 2 == 0 && (void *)blockBefore > heapStart)
		{
			block = front_coalescence(block, *blockBefore);
		}
		else if ((void *)blockAfter < heapEnd && *blockAfter % 2 == 0)
		{
			block = rear_coalescence(block, *blockAfter);
		}
		else
		{	
			add_sorted_list(block);
		}

	}
	
	void *nextBlock = (void *)((char *)block + *(int *)block + 2 * BLOCK_HEADER);
	if (nextBlock == heapEnd && *(int *)block > MAX_TOP_FREE)
	{
		limit_max_top(block);
	}
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	if (nextFitStart == block)
	{
		if (pBlock->next != NULL)
		{
			nextFitStart = pBlock->next;
		}
		else
		{
			nextFitStart = freeListHead;
		}
	}

	if (block == freeListHead && block == freeListTail)
	{
		freeListHead = NULL;
		freeListTail = NULL;
	}
	else if (block == freeListHead)
	{
		freeListHead = pBlock->next;
		pBlock->next->prev = NULL;
	}
	else if (block == freeListTail)
	{
		freeListTail = pBlock->prev;
		pBlock->prev->next = NULL;
	}
	else
	{
		pBlock->prev->next = pBlock->next;
		pBlock->next->prev = pBlock->prev;
		pBlock->next = NULL;
		pBlock->prev = NULL;
	}
	//	Updates SMA info
	totalFreeSize -= *(int *)block;
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	int *pSize;

	//	Points to the address where the Length of the block is stored
	pSize = (int *)ptr;

	//	Returns the deferenced size
	return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size
	void *ptr = freeListHead;
	while (ptr != NULL)
	{
		int ptrSize = ((free_block_head_t *)ptr)->size;
		if (largestBlockSize < ptrSize)
		{
			largestBlockSize = ptrSize;
		}
		ptr = ((free_block_head_t *)ptr)->next;;
	}

	return largestBlockSize;
}

void *front_coalescence(void *block, int lengthBefore)
{
	free_block_head_t *blockInfront = (free_block_head_t *)((char *)block - 2 * BLOCK_HEADER - lengthBefore);
	blockInfront->size += 2 * BLOCK_HEADER + ((free_block_head_t *)block)->size;
	int *blockTail = (int *)((char *)block + BLOCK_HEADER + *(int *)block);
	*blockTail =  blockInfront->size;
	return blockInfront;
}

void *rear_coalescence(void *block, int lengthBehind)
{
	//int blockSize = *(int *)block;
	free_block_head_t *blockBehind = (free_block_head_t *)((char *)block + 2 * BLOCK_HEADER + *(int *)block);
	free_block_head_t *pBlock = (free_block_head_t *)block;
	pBlock->size += 2 * BLOCK_HEADER + blockBehind->size;
	int *blockTail = (int *)((char *)block + BLOCK_HEADER + pBlock->size);
	*blockTail =  pBlock->size;
	if (blockBehind->next != NULL && blockBehind->prev != NULL)
	{
		
		pBlock->prev = blockBehind->prev;
		pBlock->next = blockBehind->next;
		blockBehind->prev->next = block;
		blockBehind->next->prev = block;
	}
	else if (blockBehind->next != NULL)
	{
		pBlock->next = blockBehind->next;
		blockBehind->next->prev = block;
	}
	else if (blockBehind->prev != NULL)
	{
		pBlock->prev = blockBehind->prev;
		blockBehind->prev->next = block;
	}
	else
	{
		pBlock->prev = blockBehind->prev;
		pBlock->next = blockBehind->next;
	}

	if (blockBehind == freeListHead)
	{
		freeListHead = block;
	}
	if (blockBehind == freeListTail)
	{
		freeListTail= block;
	}

	// Set the values to NULL
	blockBehind->next = NULL;
	blockBehind->prev = NULL;
	blockBehind->size = 0;

	return block;
}

void add_to_tail(void *block)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	pBlock->next = NULL; 
	((free_block_head_t *)freeListTail)->next = block;
	pBlock->prev = freeListTail;
	freeListTail = block;
}

void add_sorted_list(void *block)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	free_block_head_t *listEntry = (free_block_head_t *)freeListHead;
	int added = 0;
	while (listEntry != NULL)
	{
		if (listEntry > pBlock)
		{
			added = 1;
			if (listEntry == freeListHead)
			{
				pBlock->next = listEntry;
				listEntry->prev = pBlock;
				freeListHead = block;
				pBlock->prev = NULL;
			}
			else 
			{
				listEntry->prev->next = pBlock;
				pBlock->next = listEntry;
				pBlock->prev = listEntry->prev;
				listEntry->prev = pBlock;
			}
			break;
		}
		listEntry = listEntry->next; 
	}
	if (added == 0)
	{
		add_to_tail(block);
	}
}

void limit_max_top(void *block)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	int extraFree = pBlock->size - MAX_TOP_FREE;
	pBlock->size -= extraFree;
	int *tailTag = (int *)((char *)block + pBlock->size + BLOCK_HEADER);
	*tailTag = pBlock->size;
	sbrk(-extraFree);
	totalFreeSize -= extraFree;
	// Update the top of the heap
	heapEnd = sbrk(0);
}

void *expand_block(void *block, void *blockBehind, int size, int excessSize)
{
	free_block_head_t *rBlock = (free_block_head_t *)blockBehind;
	int *blockSize = (int *)block;
	int replace = excessSize > minFreeSize;
	remove_block_freeList(blockBehind);
	if (replace != 0)
	{
		puts("Here");
		int total = size + excessSize + 2 * BLOCK_HEADER;
		int *tailTag = (int *)((char *)blockBehind + rBlock->size + BLOCK_HEADER);
		*tailTag = total;
		rBlock->size = 0;
		rBlock->prev = NULL;
		rBlock->next = NULL;
		*blockSize = total;
		sprintf(ps,"total=%d; size=%d", total, size);
		puts(ps);
		chop_and_add(block, size, total);
	}
	else
	{
		size += excessSize;
		int *tailTag = (int *)((char *)blockBehind + rBlock->size + BLOCK_HEADER);
		*tailTag = size+1;
		// Empty the tail tag in front.
		tailTag = (int *)((char *)blockBehind - BLOCK_HEADER);
		*tailTag = 0;
		// Empty the struct;
		rBlock->size = 0;
		rBlock->next = NULL;
		rBlock->prev = NULL;
		*blockSize = size+1;
	}
	return (char *)block + BLOCK_HEADER;
}


void chop_and_add(void *block, int newSize, int oldSize)
{
	// chop and add to the list
	int *tailTag = (int *)((char *)block + oldSize + BLOCK_HEADER); // for the old block
	int *blockHeader = (int *)block;
	*blockHeader = newSize + 1;
	*tailTag = oldSize - newSize - 2 * BLOCK_HEADER; // newBlocks length

	free_block_head_t *newBlock = (free_block_head_t *)((char *)block + newSize + 2 * BLOCK_HEADER);
	newBlock->next = NULL;
	newBlock->prev = NULL;
	newBlock->size = oldSize - newSize - 2 * BLOCK_HEADER;
	tailTag = (int *)((char *)block + newSize + BLOCK_HEADER); 
	*tailTag = newSize + 1;
	add_block_freeList((char *)newBlock + BLOCK_HEADER);
}

void free_list_info()
{
	void *ptr = freeListHead;
	int i = 0;
	puts("=================");
	while (ptr != NULL)
	{
		char str[50];
		sprintf(str, "currentaddr: %p", ptr);
		puts(str);
		int ptr_size = ((free_block_head_t *)ptr)->size;
		sprintf(str, "Size of free: %d", ptr_size);
		puts(str);
		void *ptr_n = ((free_block_head_t *)ptr)->next;
		sprintf(str, "next add: %p", ptr_n);
		puts(str);
		void *ptr_p = ((free_block_head_t *)ptr)->prev;
		sprintf(str, "prev addr: %p", ptr_p);
		puts(str);
		ptr = ptr_n;
		if (ptr != NULL)
			puts("-----------------");
		i++;
	}
	puts("=================");
	
}