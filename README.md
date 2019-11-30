# Overview
 A malloc library that provides the following dynamic memory allocation routines (as defined in man 3 malloc):

- void *malloc(size_t size);
- void free(void *ptr);
- void *calloc(size_t nmemb, size_t size);
- void *realloc(void *ptr, size_t size);

- int posix_memalign(void **memptr, size_t alignment, size_t size);
- void *memalign(size_t alignment, size_t size);
- struct mallinfo mallinfo();
- void malloc_stats(void);

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
- utils.h contains all the helper functions like aligning the size, splitting blocks etc.
- malloc.h contains the functions of malloc library malloc, calloc, realloc, free, memalign, posix_memalign
- Each of the malloc functions are in separate .c files

## Struct to hold the block information
```c
typedef struct memory_block *block;

struct memory_block {
	size_t size; 			/* Block size */
	block next;			/* Next block */
	block prev;			/* Previous block */
	int free;			/* Block is free or not */
	int buddy_order;		/* Block's buddy order */
	void *ptr;			/* Block's pointer pointing to the arena */
	char data [1];
};
```

## Struct to hold the arena information
```c
typedef struct my_malloc_arena *malloc_arena;

struct my_malloc_arena {
	block start;			/* Starting block of the arena */
	size_t size;			/* Size of the arena */
	malloc_arena next;    		/* Next arena */
	malloc_arena prev;    		/* Previous arena */
	int ordblks;      		/* Number of free chunks */
	int hblks;     			/* Number of mmapped regions */
	int hblkhd;    			/* Space allocated in mmapped regions (bytes) */
	int usmblks;   			/* Maximum total allocated space (bytes) */
	int uordblks;       		/* Total allocated space (bytes) */
	int fordblks;       		/* Total free space (bytes) */
	int allocation_req;
        int free_req;
	pthread_mutex_t lock;
	char data[1];
};
```

## Mallinfo struct
```c
struct mallinfo {
	int arenas; 		        /* Total arenas */
	int ordblks;   			/* Number of free chunks */
	int hblks;    			/* Number of mmapped regions */
	int hblkhd;   			/* Space allocated in mmapped regions (bytes) */
	int usmblks;   			/* Maximum total allocated space (bytes) */
	int uordblks;  			/* Total allocated space (bytes) */
	int fordblks;  			/* Total free space (bytes) */
};
```

## Data Structure
- Memory Block Struct
	- I used doubly linked list to store the meta information about each blocks
	- Keeping track of next and previous block helped me join free blocks in O(n) runtime.
	- My struct stores the buddy order which makes the check for required order block O(1)
	- My struct stores also stores free status of each block

- Memory Arena Struct
	- My struct is also a doubly linked list node
	- My struct also has a pthread_mutex_t lock, which is used to lock the arena in case of allocation and deallocation
	- It also contains fields required for malloc stats

## Malloc Logic
- If arena is null, create arena using mmap by requesting size in multiple of page size
- First 8-byte align the requested size
- If arena->start is initialized, search for a free chunk wide enough.
- If arena->start is not initialized, that means its the first request for arena and new block will become start of the arena.
- While searching, keep splitting the blocks using the buddy logic. Read Buddy Allocation [here](https://en.wikipedia.org/wiki/Buddy_memory_allocation)
- Assign the requested memory to the block wide enough.
– If new chunk is found,
  - Try to split the block using Buddy
	- Mark the chunk as used (b->free=0;)
	– if no fitting block found according to buddy, current implementation returns NULL indicating no more blocks can be allocated in the arena.

	Note while finding the new block I put the pointer to the last visited chunk in
	last, so I can access it during the extension without traversing the whole list
	again.

## Free logic
- Validate the address using the block's pointer. If the pointer is pointing to the arena in which it was allocated, it is a valid pointer
- If valid, get the block pointed by pointer. Free the block.
- Join the buddy blocks if they are also free and merge them to a single block. Increase the order and size accordingly.

## Important Design Decisions
- I am making sure that my block is always aligned so that while allocating memory I have to only align the requested size.
- While traversing the linked list to find the free block I am keeping track of last visited node so the malloc function can easily extends the end of the heap if no fitting chunk found
- the split_block function in [utils.c](utils.c) cut the block passed in argument to make data block of the wanted size and all splitted blocks become part of linked list, so that next time there wont be need to split if any of the existing can satisfy the requested memory.
- There is at least one malloc arena per CPU core. This library will detect the total number of CPU cores and create that many arenas.
- Each thread allocates/deallocates from one of these per-cpu arenas.
- To support fork and thread safe allocation/deallocation, I assign thread specific arena to each thread using __thread malloc_arena arena;
