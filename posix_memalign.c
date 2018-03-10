#include "malloc.h"

#define VOID_SIZE (sizeof(void *))

int is_two_power(size_t s) {
	while (s > 1) {
		if (s % 2 != 0) {
			return -1;
		}
		s = s / 2;
	}
	return 0;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
	if (is_two_power(size) != 0 && alignment % VOID_SIZE != 0) {
		return EINVAL;
	}
	size_t s = alignn(size, alignment);

	*memptr = malloc(s);
	if (memptr) {
 		return (0); // returning the starting address of the block
	}
	return ENOMEM;
}
