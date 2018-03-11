#include "malloc.h"

void malloc_stats(void) {
	struct mallinfo info = mallinfo();
	printf("Malloc Statistics:\n");
	printf("Total arenas: %d\nNumber of free chunks: %d\nNumber of mmapped regions: %d\nSpace allocated in mmapped regions (bytes): %u\nMaximum total allocated space (bytes): %u\nTotal allocated space (bytes): %u\nTotal free space (bytes): %u\n\n", info.arenas, info.ordblks, info.hblks, info.hblkhd, info.usmblks, info.uordblks, info.fordblks);

	printf("Arena Wise Statistics:\n");
	malloc_arena start = arena_head;
	int i = 0;
	while(start != NULL) {
		printf("Arena: %d\n", i);
		printf("Number of free chunks: %d\nNumber of mmapped regions: %d\nSpace allocated in mmapped regions (bytes): %u\nMaximum total allocated space (bytes): %u\nTotal allocated space (bytes): %u\nTotal free space (bytes): %u\n\n", start->ordblks, start->hblks, start->hblkhd, start->usmblks, start->uordblks, start->fordblks);
		i++;
		start = start->next;
	}
}
