#include "malloc.h"

#define VOID_SIZE (sizeof(void *))

int posix_memalign(void **memptr, size_t alignment, size_t size) {
	if (size == 0 || is_two_power(alignment) != 0 || alignment % VOID_SIZE != 0) {
		*memptr = NULL;
		return EINVAL;
	}
	size_t s = alignn(size, alignment);
	*memptr = malloc(s);
	if (memptr) {
 		return (0); // returning the starting address of the block
	}
	return ENOMEM;
}
