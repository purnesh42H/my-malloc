#include "malloc.h"

void *calloc(size_t count, size_t size) {
	size_t *new;
	size_t s8;
	new = malloc(count * size); // allocate memory using malloc
	if (new) {
		s8 = align8(count * size) >> 2;
		memset(new, 0, s8); // initialize the memory region with zero. Each integer value is assumed to be 4 bytes
	}

	return (new);
}
