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

char ps[100];
char *sma_malloc_error;
void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
int minFreeSize = sizeof(free_block_head_t) + BLOCK_HEADER;
void *heapStart;
void *heapEnd;
void *nextFitStart = NULL;
int nextFitSet = 0;
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
	int maxKB = 64;
	void *newBlock = NULL;
	int headers = 2 * BLOCK_HEADER;
	int newBlockSize = size + headers;
	int excessSize = maxKB * 1024 ;
	int totalSize = newBlockSize + excessSize + headers ;
	if (heapStart == NULL)
		heapStart = sbrk(0);
	newBlock = sbrk(totalSize);
	if (newBlock == NULL)
	{
		sprintf(sma_malloc_error, "Error allocating\n");
		return NULL;
	}
	heapEnd = sbrk(0);
	int *blockFront = (int *)newBlock -1;
	((free_block_head_t *)newBlock)->next = NULL;
	((free_block_head_t *)newBlock)->prev = NULL;
	((free_block_head_t *)newBlock)->size = totalSize - headers;
	if (*blockFront %2 == 0 && *blockFront != 0)
	{
		newBlock = frontCoalescence(newBlock, *blockFront);
		excessSize = ((free_block_head_t *)newBlock)->size - newBlockSize;
		int *tailTag = (int *)((char *)newBlock + ((free_block_head_t *)newBlock)->size + BLOCK_HEADER);
		*tailTag = newBlockSize;
		remove_block_freeList(newBlock);
	}else
	{
		int *tailTag = (int *)((char *)newBlock + totalSize + BLOCK_HEADER);
		*tailTag = newBlockSize;
	}
	//	Allocates the Memory Block by sending the whole newBlock to get allocated accordingly.
	allocate_block(newBlock, size, excessSize, 0);
	// Move forward to the free space
	return newBlock + BLOCK_HEADER;
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

	void *ptr = freeListHead;
	excessSize = 0;
	
	while (ptr != NULL)
	{
		int ptrSize = ((free_block_head_t *)ptr)->size;
		if (ptrSize > minSize && ptrSize >= size && (ptrSize-size-2*BLOCK_HEADER) >= excessSize)
		{
			worstBlock = ptr;
			excessSize = ptrSize-size - 2*BLOCK_HEADER;
			blockFound = 1;
		}
		ptr = ((free_block_head_t *)ptr)->next;
	}
	//	Checks if appropriate block is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
		// Move forward to the free space
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

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	Start off with the freeListHead, and keep track of the current position in the free memory list.
	//			The next time you allocate, it should start from the current position.
	//	Checks if appropriate found is found.
	if (nextFitSet == 0)
		nextFitStart = freeListHead;
	nextFitSet = 1;

	puts("Here");
	free_block_head_t *listEntry = (free_block_head_t *)nextFitStart; 
	while (listEntry != NULL && blockFound == 0)
	{
		if (listEntry->size >= size)
		{
			blockFound = 1;
			nextBlock = (void *)listEntry;
			excessSize = listEntry->size - size - 2*BLOCK_HEADER;
		}
		listEntry = listEntry->next;
	}


	if (blockFound)
	{
		//	Allocates the Memory Block
		if (((free_block_head_t *)nextBlock)->next == NULL)
			nextFitStart = freeListHead;
		else
			nextFitStart = (void *)((free_block_head_t *)nextBlock)->next;
		allocate_block(nextBlock, size, excessSize, 1);
		nextBlock = (void *)((char *)nextBlock + BLOCK_HEADER);
		if (excessSize > minFreeSize)
			nextFitStart = (void *)((free_block_head_t *)nextFitStart)->prev;
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}

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
	int addFreeBlock  = excessSize > minFreeSize;
	/** 
	 * TODO: Refractor Code! Need proper casting
	 */
	//	If excess free size is big enough
	if (addFreeBlock)
	{
		// Set the tail
		int *tailTag = (int *)((char *)newBlock + size + BLOCK_HEADER);
		*tailTag = size + 1;

		excessFreeBlock = (void *)((char *)newBlock + size + 2 * BLOCK_HEADER);
		((free_block_head_t *)excessFreeBlock)->size =  excessSize;
		((free_block_head_t *)newBlock)->size = size;
		// Set the split blocks tails
		tailTag = (int *)((char *)excessFreeBlock + excessSize + BLOCK_HEADER);
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
		int *tailTag = (int *)((char *)newBlock + size + BLOCK_HEADER);
		*tailTag = size + 1;
		((free_block_head_t *)newBlock)->size = size;
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
		((free_block_head_t *)newBlock)->size++;

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
	/** 
	 * TODO: Refractor Code! Need proper casting
	 */
	free_block_head_t *pNewBlock = (free_block_head_t *)newBlock;
	free_block_head_t *pOldBlock = (free_block_head_t *)oldBlock;

	pNewBlock->prev = pOldBlock->prev;
	pNewBlock->next = pOldBlock->next;
	
	if (pOldBlock->prev != NULL)
		pOldBlock->prev->next = pNewBlock;
	if (pOldBlock->next != NULL)
		pOldBlock->next->prev = pNewBlock;
	// Clear these in order to give this memory space back to the user
	pOldBlock->prev = NULL;
	pOldBlock->next = NULL;

	if (oldBlock == freeListHead)
	{
		// Swap head of the list if it is the head of the list
		freeListHead = pNewBlock;
	}

	if (oldBlock == freeListTail)
	{
		// Swap tail of the list if it is the tail of the list
		freeListTail = pNewBlock;
	}
	//	Updates SMA info

	void *intOldBlock = (void *)((int *)newBlock + 1);
	totalAllocatedSize += get_blockSize(intOldBlock);
	totalFreeSize -= get_blockSize(intOldBlock);

}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block)
{
	block = block - BLOCK_HEADER; 
	free_block_head_t *pBlock = (free_block_head_t *)block;
	int flag = 0;
	// Coming from free, the block was in use, therefore, we need to reset the length and reset flag
	void *intBlock = (void *)((int *)block + 1);
	if ((pBlock->size % 2) != 0)
	{
		pBlock->size = pBlock->size - 1;
		int *ptr = (int *)((char *)block + BLOCK_HEADER + pBlock->size);
		*ptr = *ptr - 1;
		totalAllocatedSize -= get_blockSize(intBlock);
	}
	//	Updates SMA info
	totalFreeSize += get_blockSize(intBlock);
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
		free_block_head_t *currentBlock = block;
		int blockBefore = *(int *)((char *)block - BLOCK_HEADER);
		int blockAfter = *(int *)((char *)block + 2 * BLOCK_HEADER + currentBlock->size);
		// Should fix these to reflect what they are actually looking at.
		// Looking for being at the start of the heap (heapAbsHead), or sbrk(0)
		if (blockBefore != 0 && blockAfter != 0)
		{
			if (blockBefore % 2 == 0 && blockAfter % 2 == 0)
			{
				block = frontCoalescence(block, blockBefore);
				block = rearCoalescence(block, blockAfter);
			}
			else if (blockBefore % 2 == 0)
			{
				block = frontCoalescence(block, blockBefore);
			} 
			else if (blockAfter % 2 == 0)
			{	
				block = rearCoalescence(block, blockAfter);
			}
			else
			{
				addToSortedList(block);
				//addToTail(block);
			}
		}
		else if(blockBefore % 2 == 0 && blockBefore != 0)
		{
			block = frontCoalescence(block, blockBefore);
		}
		else if (blockAfter != 0 && blockAfter % 2 == 0)
		{
			block = rearCoalescence(block, blockAfter);
		}
		else
		{	
			addToSortedList(block);
			//addToTail(block);
		}

	}
	// Check if the new block is near the top and too large. Trim if so.
	pBlock = (free_block_head_t *)block;
	void *rearBlock = (void *)((char *)block + pBlock->size + 2 * BLOCK_HEADER); 
	if (rearBlock == heapEnd && pBlock->size > MAX_TOP_FREE)
	{
		int extraFree = pBlock->size - MAX_TOP_FREE;
		pBlock->size = MAX_TOP_FREE;
		int *tailTag = (int *)((char *)block + pBlock->size + BLOCK_HEADER);
		*tailTag = pBlock->size;
		sbrk(-extraFree);
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

	void *intBlock = (void *)((int *)block + 1);
	totalFreeSize -= get_blockSize(intBlock);
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
	pSize--;
	//	Returns the deferenced size
	return *(int *)pSize ;
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

	free_block_head_t *ptr = (free_block_head_t *)freeListHead;
	free_block_head_t *largest;
	while (ptr != NULL)
	{
		int ptrSize = ptr->size;
		if (largestBlockSize < ptrSize)
		{
			largestBlockSize = ptrSize;
			largest = ptr;
		}
		ptr = ptr->next;
	}
	return largestBlockSize;
}

void freeListInfo()
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

void *frontCoalescence(void *block, int lengthBefore)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	free_block_head_t *blockInfront = (free_block_head_t *)((char *)block - 2 * BLOCK_HEADER - lengthBefore);
	blockInfront->size += 2 * BLOCK_HEADER + pBlock->size;
	int *blockTail = (int *)((char *)block + BLOCK_HEADER + pBlock->size);
	*blockTail =  blockInfront->size;
	return blockInfront;
}

void *rearCoalescence(void *block, int lengthBehind)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	free_block_head_t *blockBehind = (free_block_head_t *)((char *)block + 2 * BLOCK_HEADER + pBlock->size);
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
		blockBehind->next->prev = pBlock;
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

void addToTail(void *block)
{
	free_block_head_t *pBlock = (free_block_head_t *)block;
	pBlock->next = NULL; 
	((free_block_head_t *)freeListTail)->next = block;
	pBlock->prev = freeListTail;
	freeListTail = block;
}

void addToSortedList(void *block)
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
		addToTail(block);
	}
}