#include "malloc.h"

void free(void *p) {
	block b;
  // We have to check if the address is valid i.e. it is a pointer who has been already allocated the memory
	if (arena && valid_address(arena, p)) {
		b = get_block(p); // get the block to free
		pthread_mutex_lock(&arena->lock); //locking the arena before allocation
		deallocate(arena, b);
		pthread_mutex_unlock(&arena->lock); //unlocking the arena
	}
}
