#include "malloc.h"

__thread malloc_arena arena = NULL;

void *malloc(size_t size) {
	if (!max_arena_size) {
		long int page_size = sysconf(_SC_PAGESIZE);
		max_arena_size = page_size * page_size;
		long int i = 0;
		for(i = 0; i < 1024000; i++)
			max_arena_size += page_size;
	}
	if (!max_arenas) {
		max_arenas = sysconf(_SC_NPROCESSORS_ONLN);
	}
	block start;
	size_t s;
	s = align8(size); // 8-byte alignment for every size
	if (arena) {
		pthread_mutex_lock(&arena->lock); //locking the arena before allocation
		start = insert_block(arena, s);
		pthread_mutex_unlock(&arena->lock); //unlocking the arena

		if (!start) {
			errno = ENOMEM; //Error if no more memory can be allocated
 			return(NULL);
	 	}
	} else {
		arena = create_arena(s);
		if (!arena) { // extending the heap for the first time
			errno = ENOMEM;
			return (NULL);
		}

		pthread_mutex_lock(&arena->lock); //locking the arena before allocation
		start = insert_block(arena, s);
		pthread_mutex_unlock(&arena->lock); //unlocking the arena
		if (!start) {
			errno = ENOMEM; // Error if no more memory can be allocated
	 	 	return(NULL);
	 	}
 	}
	arena->allocation_req += 1;
	arena->ordblks += 1;
	arena->hblkhd += s;
	arena->hblks += 1;
	if (s > arena->usmblks) {
		arena->usmblks = s;
	}
	arena->uordblks += s;
	arena->fordblks -= s;

 	return(start->data); // returning the starting address of the block
}
