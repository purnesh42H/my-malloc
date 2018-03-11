#include "malloc.h"

void *realloc(void *p, size_t size) {
	size_t s;
	block b, new;
	void *newp = NULL;
	if (!p || !arena) // if pointer is null just allocate the memory to current break
		return (malloc(size));
	if (valid_address (arena, p)) { // Reallocate only if its a valid address, as we need to free the old address
		s = align8(size);
 		b = get_block(p);
		if (b->size >= s) {
			if (mlock(b, s) == 0) {
				if (b->buddy_order > get_buddy_order(s)) { // Same as malloc, split if chunk is bigger than required memory
	 				split_block (arena, b, s);
				}
				b->free = 0;
				munlock(b, s);
			} else {
				printf("Failed to lock\n");
				errno = ENOMEM; // Error if no more memory can be allocated
 	 	 	 	return (NULL);
			}
		} else {
			if (b->next && b->next->free && (b->size + block_size() + b->next->size) >= s) { // try joining the next free chunk
				if (mlock(b, s) == 0) {
					b = buddy_join(arena, b);
					if (b->buddy_order > get_buddy_order(s)) {
						split_block (arena, b, s);
					}
					b->free = 0;
					munlock(b, s);
				}
			} else {
					newp = malloc(s); // allocate the memory
					if (!newp) {
						printf("No block\n");
						errno = ENOMEM;
			 			return (NULL);
					}
					new = get_block(newp);
					if(mlock(new, s) == 0) {
						copy_block(b, new); // copy data from previous to new block
						munlock(new, s);
					} else {
						printf("Failed to lock\n");
						errno = ENOMEM; // Error if no more memory can be allocated
		 	 	 	 	return(NULL);
					}
					free(p); // free the old one
					return (newp);
				}
    	}
 			return (p); // return the old one
		}
		printf("Invalid pointer\n");
		errno = ENOMEM;
		return (NULL);
}
