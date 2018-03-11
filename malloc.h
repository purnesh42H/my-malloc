#ifndef MALLOC_H_
#define MALLOC_H_
#include "utils.h"

struct mallinfo {
	int arenas; 				/* Total arenas */
	int ordblks;   			/* Number of free chunks */
	int hblks;    			/* Number of mmapped regions */
	int hblkhd;   			/* Space allocated in mmapped regions (bytes) */
	int usmblks;   			/* Maximum total allocated space (bytes) */
	int uordblks;  			/* Total allocated space (bytes) */
	int fordblks;  			/* Total free space (bytes) */
};

/* The function returns a fitting chunk, or NULL if none where found. After the execution, the
   argument last points to the last visited chunk. */
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void free(void *ptr);
void *realloc(void *p, size_t size);
int posix_memalign(void **memptr, size_t alignment, size_t size);
void *memalign(size_t alignment, size_t size);
struct mallinfo mallinfo();
void malloc_stats(void);

extern __thread malloc_arena arena;

#endif
