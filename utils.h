#ifndef UTILS_H_
#define UTILS_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

typedef struct memory_block *block;
typedef struct my_malloc_arena *malloc_arena;

struct memory_block {
	size_t size; 				/* Block size */
	block next;					/* Next block */
	block prev;					/* Previous block */
	int free;						/* Block is free or not */
	int buddy_order;		/* Block's buddy order */
	void *ptr;					/* Block's pointer pointing to the arena */
	char data [1];
};

struct my_malloc_arena {
	block start;					/* Starting block of the arena */
	size_t size;					/* Size of the arena */
	malloc_arena next;    /* Next arena */
	malloc_arena prev;    /* Previous arena */
	int ordblks;      		/* Number of free chunks */
	int hblks;     				/* Number of mmapped regions */
	int hblkhd;    				/* Space allocated in mmapped regions (bytes) */
	int usmblks;   				/* Maximum total allocated space (bytes) */
	int uordblks;       	/* Total allocated space (bytes) */
	int fordblks;       	/* Total free space (bytes) */
	pthread_mutex_t lock;
	char data[1];
};

/* The function returns a fitting chunk, or NULL if none where found. After the execution, the
   argument last points to the last visited chunk. */
int is_two_power(size_t s);
int get_two_power(size_t s);
malloc_arena find_arena(size_t size);
block join_free_chunks(malloc_arena arena, block b);
int get_buddy_order(size_t s);
void buddy_split(malloc_arena arena, block b);
void allocate(malloc_arena arena, block b, size_t s, block last);
void deallocate(malloc_arena arena, block b);
block buddy_join(malloc_arena arena, block b);
size_t alignn(size_t s, size_t alignment);
size_t align8(size_t s);
size_t block_size();
size_t arena_size();
block get_block (void *p);
block insert_block(malloc_arena arena, size_t s);
block find_free_block(malloc_arena arena, block *last, size_t size);
void copy_block(block src, block dest);
malloc_arena create_arena(size_t size);
void split_block(malloc_arena arena, block b, size_t s);
int valid_address(malloc_arena arena, void *p);

extern size_t max_arena_size;
extern void *arena_head;
extern long int max_arenas;
extern long int current_arenas;
extern const int SMALLEST_BLOCK;
extern size_t current_mmap_allocation;

#endif
