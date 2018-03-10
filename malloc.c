#include "malloc.h"

__thread malloc_arena arena = NULL;

void *malloc(size_t size) {
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
 	return(start->data); // returning the starting address of the block
}
