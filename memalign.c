#include "malloc.h"

void *memalign(size_t alignment, size_t size) {
	void *ptr = NULL;
	if (posix_memalign((void **)&ptr, alignment, size) == 0) {
		return (ptr);
	}
	errno = ENOMEM;
	return (NULL);
}
