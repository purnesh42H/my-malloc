#include "malloc.h"

void *memalign(size_t alignment, size_t size) {
	if (is_two_power(alignment) != 0) {
		errno = EINVAL;
		return (NULL);
	}
	size_t s = alignn(size, alignment);
	void *ptr = malloc(s);
	if(!ptr) {
		errno = ENOMEM;
		return (NULL);
	}
	return ptr;
}
