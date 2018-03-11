#include "malloc.h"

struct mallinfo mallinfo() {
	struct mallinfo info;
	info.arenas = current_arenas;
	info.ordblks = 0;
	info.hblkhd = 0;
	info.hblks = 0;
	info.usmblks = 0;
	info.uordblks = 0;
	info.fordblks = 0;

	malloc_arena start = arena_head;
	while(start != NULL) {
		info.ordblks += arena->ordblks;
		info.hblks += arena->hblks;
		info.hblkhd += arena->hblks;
		if (arena->usmblks > info.usmblks)
			info.usmblks += arena->usmblks;
		info.uordblks += arena->uordblks;
		info.fordblks += arena->fordblks;
		start = start->next;
	}

	return info;
}
