# Overview
 A malloc library that provides the following dynamic memory allocation routines (as defined in man 3 malloc):

- void *malloc(size_t size);
- void free(void *ptr);
- void *calloc(size_t nmemb, size_t size);
- void *realloc(void *ptr, size_t size);

- int posix_memalign(void **memptr, size_t alignment, size_t size);
- void *memalign(size_t alignment, size_t size);

# How to run
- Existing tests
  - There are two tests
      - [Simple test](test1.c). run "make check"
      - [Benchmark test](t-test1.c). run "make check1"

- Create your test
  - Include the my malloc header file (# include "malloc.h")
  - Use the way you use standard malloc
  - Run the "make" to create the shared library i.e. (.so) file
  - run your test program using LD_PRELOAD=`pwd`/libmalloc.so <your output file>. For eg: LD_PRELOAD=`pwd`/libmalloc.so ./test1

# Design
## Code Strucure
- There are two header files utils.h and malloc.h
- Utils.h contains all the helper functions like aligning the size, splitting blocks etc.
- malloc.h contains the functions of malloc library malloc, calloc, realloc, free, memalign, posix_memalign
- Each of the malloc functions are in separate .c files

## Struct to hold the block information
```c
typedef struct memory_block *block;

struct memory_block {
	size_t size;     // size of the block (8-byte aligned)
	block next;  	   // Address of the next block. It is important because we want to
              	   // traverse from one block to another to find the free block and also join the
            		   // free blocks if they are adjacent to each other to decrease fregmentation
	block prev; 	   // Address of the previous block. This is very important during free to combine adjacent free blocks
	int free; 	     // To check if the block is free
  int buddy_order; // Order of the block according to buddy allocation
	void *ptr; 	     // This pointer stores the address of the memory block's data i.e. end of block's address.
           		     // It is useful to validate the block's address as it tells us that the
			             // block is alloced to through this malloc library.
	char data [1]; 	 // Meta data for each block. The pointer to this mark the end of block.
};
```

## Struct to hold the arena information
```c
typedef struct my_malloc_arena *malloc_arena;

struct my_malloc_arena {
	block start;
	size_t size;
	malloc_arena next;
	malloc_arena prev;
	pthread_mutex_t lock;
	char data[1];
};
```

## Data Structure
- I used doubly linked list to store the meta information about each blocks
- Keeping track of next and previous block helped me join free blocks in O(n) runtime.
- My struct stores the buddy order which makes the check for required order block O(1)
- My struct stores also stores free status of each block

## Malloc Logic
- First 8-byte align the requested size
- If heap_start is initialized, search for a free chunk wide enough. This will become start of the heap
- While searching, keep splitting the blocks using the buddy logic. Read Buddy Allocation [here](https://en.wikipedia.org/wiki/Buddy_memory_allocation)
- Assign the requested memory to the block wide enough.
– If new chunk is found,
  - Try to split the block using Buddy
	- Mark the chunk as used (b->free=0;)
	– if no fitting block found according to buddy, I extend the heap.

	Note while finding the new block I put the pointer to the last visited chunk in
	last, so I can access it during the extension without traversing the whole list
	again. Otherwise I extend the heap (which is empty at that point.)
	Note that the function extend heap works here with last=NULL.

## Free logic
- Validate the address using the block's pointer. If the pointer is pointing to end of meta block, it is a valid pointer
- If valid, get the block pointed by pointer. Free the block.
- Join the buddy blocks if they are also free and merge them to a single block. Increase the order and size accordingly.

## Important Design Decisions
- I am making sure that my block is always aligned so that while allocating memory I have to only align the requested size.
- While traversing the linked list to find the free block I am keeping track of last visited node so the malloc function can easily extends the end of the heap if no fitting chunk found
- the split_block function in [utils.c](utils.c) cut the block passed in argument to make data block of the wanted size and all splitted blocks become part of linked list, so that next time there wont be need to split if any of the existing can satisfy the requested memory.
- There is at least one malloc arena per CPU core. This library will detect the total number of CPU cores and create that many arenas.
- Each thread allocates/deallocates from one of these per-cpu arenas.
