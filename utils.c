#include "utils.h"

size_t max_arena_size = 0;
long int max_arenas = 0;
long int current_arenas = 0;
void *arena_head = NULL;
const int SMALLEST_BLOCK = 128;

int is_two_power(size_t s) {
	while (s > 1) {
		if (s % 2 != 0) {
			return -1;
		}
		s = s / 2;
	}
	return 0;
}

int get_two_power(size_t s) {
	int power = 0;
	while (s > 1) {
		s = s / 2;
		power++;
	}
	return power;
}

malloc_arena find_arena(size_t size) {
	malloc_arena start = arena_head;

	if(current_arenas <= max_arenas) {
		while(start->next != NULL) {
			start = start->next;
		}
		return start;
	} else {
		return NULL;
	}
}

block join_free_chunks(malloc_arena arena, block b) {
	if (b->next && b->next->free) {
		b->size += b->next->size + block_size();
		b->next = b->next->next;
		if (b->next) {
			b->next->prev = b;
		}
		b->buddy_order = get_buddy_order(b->size);
	}
	return (b);
}

int get_buddy_order(size_t s) {
	size_t lower = SMALLEST_BLOCK;
	int order = 0;
	while (lower < s) {
		lower = lower * 2;
		order++;
	}
	return order;
}

void buddy_split(malloc_arena arena, block b) {
	b->size = b->size / 2;
	block newb = (block)(b->data + b->size);
	newb->size = b->size - block_size();
	newb->next = b->next;
	newb->prev = b;
	b->next = newb;
	b->buddy_order = b->buddy_order - 1;
	newb->buddy_order = b->buddy_order;
	newb->free = 1;
	b->ptr = arena->data;
  newb->ptr = arena->data;
}

void allocate(malloc_arena arena, block b, size_t s, block last) {
	b->size = s;
	b->prev = last;
	b->next = NULL;
	b->free = 1;
	b->buddy_order = get_buddy_order(s);
	b->ptr = arena->data;
}

void deallocate(malloc_arena arena, block b) {
	if(!b->next) {
		if (b->prev) {
			b = buddy_join(arena, b);
		} else {
			if(!arena->prev) {
				if(arena->next) {
					arena->next->prev = NULL;
					arena_head = (void *)arena->next;
				} else {
					arena_head = NULL;
				}
				void *addr = (void *)arena;
				size_t length = arena->size;
				arena = NULL;
				munmap(addr, length);
				current_arenas -= 1;
			}
		}
	} else {
		b = buddy_join(arena, b);
	}
}

block buddy_join(malloc_arena arena, block b) {
	while (b->prev && b->prev->free) {
		b = b->prev;
		void *p = (void *)b->prev;
		if (p < (void *)arena->start || p > (void *)(arena->data + arena->size)) {
			b->prev = NULL;
			break;
		}
	}
	void *p = (void *)b->next;
	if (p < (void *)arena->start || p > (void *)(arena->data + arena->size)) {
		b->next = NULL;
		return (b);
	}
	while(b->next && b->next->free) {
		b = join_free_chunks(arena, b);
		arena->ordblks -= 1;
	}
	return b;
}

size_t alignn(size_t s, size_t alignment) {
	int power = get_two_power(alignment);
	return (((((s) -1) >> power) << power) + alignment);
}

size_t align8(size_t s) {
	return alignn(s, 8);
}

size_t block_size() {
	// Since its a 64 bit machine
	return 40;
}

size_t arena_size() {
	// Since its a 64 bit machine
	return 96;
}

/* Get the block from and addr */
block get_block (void *p) {
	char *tmp;
	tmp = p;
	return (p = tmp -= block_size());
}

block insert_block(malloc_arena arena, size_t s) {
	block start, last;
	if(!arena->start) {
		start = (block)arena->data;
		arena->start = start;
		allocate(arena, arena->start, arena->size, NULL);
		arena->ordblks = 1;
		arena->fordblks = arena->size;
	} else {
		last = arena->start;
		start = find_free_block(arena, &last, s);
	}

	if(!start) {
		printf("No block for size %zd\n", s);
		return NULL;
	}

	split_block(arena, start, s);
	start->free = 0;
	return start;
}

block find_free_block(malloc_arena arena, block *last, size_t size) {
	int order = get_buddy_order(size);
	block start = arena->start;
	while(start && !(start->free && start->buddy_order == order && start->size >= size)) {
		void *p = (void *)start->next;
		if (p < (void *)arena->start || p > (void *)(arena->data + arena->size)) {
			start->next = NULL;
			*last = start;
			start = NULL;
			break;
		}
		*last = start;
		start = start->next;
	}

	if (!start) {
		start = arena->start;
		while(start && !(start->free && start->buddy_order > order && (start->size)/2 >= size)) {
			void *p = (void *)start->next;
			if (p < (void *)arena->start || p > (void *)(arena->data + arena->size)) {
				start->next = NULL;
				*last = start;
				start = NULL;
				break;
			}
			*last = start;
			start = start->next;
		}
	}

	return start;
}

void copy_block(block src, block dest) {
	int *src_data, *dest_data;
	size_t i;
	src_data = src->ptr;
	dest_data = dest->ptr;

	for(i = 0; i * 8 > src->size && i * 8 < dest->size; i++) {
		dest_data[i] = src_data[i];
	}
}

malloc_arena create_arena(size_t size) {
	malloc_arena arena;
	size_t request_memory = max_arena_size;
	while (request_memory < size) {
		request_memory += max_arena_size;
	}

	if (!arena_head) {
		void *addr = mmap(NULL, request_memory, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED) {
			printf("MMAP failed\n");
			return NULL;
		}
		arena = (malloc_arena)addr;
		arena->size = request_memory;
		arena->next = NULL;
		arena->prev = NULL;
		arena_head = (void *)arena;
	} else {
		malloc_arena start = find_arena(size);
		if (!start) {
			printf("No more arena available\n");
			return NULL;
		}
		void *addr = mmap(start->data, request_memory, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED) {
			printf("MMAP failed\n");
			return NULL;
		}
		arena = (malloc_arena)addr;
		start->next = arena;
		arena->prev = start;
		arena->size = request_memory;
	}
	arena->ordblks = 0;
	arena->hblkhd = 0;
	arena->hblks = 0;
	arena->usmblks = 0;
	arena->uordblks = 0;
	arena->fordblks = arena->size;
	arena->allocation_req = 0;
	arena->free_req = 0;
	current_arenas += 1;
	return arena;
}

void split_block(malloc_arena arena, block b, size_t s) {
	int required_order = get_buddy_order(s);
	while (b->buddy_order != required_order) {
		buddy_split(arena, b);
		arena->ordblks += 1;
	}
}

int valid_address(malloc_arena arena, void *p) {
	if(arena) {
		if(p > (void *)arena->start && p < (void *)(arena->data + arena->size)) { // if the pointer is between the arena start and arena end, then it is a valid
			return (arena->data == get_block(p)->ptr); // we have field ptr pointing to the arena data, if b->ptr == arena->data, then b is probably (very probably) a valid block
		}
	}
	return (0);
}
