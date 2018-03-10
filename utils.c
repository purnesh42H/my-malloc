#include "utils.h"

size_t max_arena_size = 0;
int max_arenas = 0;
int current_arenas = 0;
void *arena_head = NULL;
const int SMALLEST_BLOCK = 128;

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

	if(current_arenas < max_arenas) {
		while(start->next != NULL) {
			start = start->next;
		}
		return start;
	} else {
		return NULL;
	}
}

block join_free_chunks(block b) {
	if (b->next && b->next->free) {
		b->size += b->next->size + block_size();
		b->next = b->next->next;
		if (b->next) {
			b->next->prev = b;
		}
		b->buddy_order += 1;
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
			buddy_join(b);
		} else {
			if(arena->prev) {
				arena->prev->next = arena->next;
			}
			void *addr = (void *)arena;
			size_t length = arena->size;
			arena = NULL;
			munmap(addr, length);
			current_arenas -= 1;
		}
	} else {
		buddy_join(b);
	}
}

block buddy_join(block b) {
	while (b->prev && b->prev->free) {
		b = b->prev;
	}
	while(b->next && b->next->free) {
		b = join_free_chunks(b);
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
	return 72;
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
	} else {
		last = arena->start;
		start = find_free_block(arena, &last, s);
	}

	if(!start) {
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
		long long int diffr = (void *)start->next - (void *)(arena->data + arena->size);
		if (start->next && diffr >= 0) {
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
			long long int diffr = (void *)start->next - (void *)(arena->data + arena->size);
			if (start->next && diffr >= 0) {
				start->next = NULL;
				start = NULL;
				*last = start;
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
	if (!max_arena_size) {
		max_arena_size = sysconf(_SC_PAGESIZE) * sysconf(_SC_PAGESIZE);
	}
	if (!max_arenas) {
		max_arenas = sysconf(_SC_NPROCESSORS_ONLN);
	}
	size_t request_memory = max_arena_size;
	while (request_memory < size) {
		request_memory += max_arena_size;
	}

	if (!arena_head) {
		void *addr = mmap(NULL, request_memory, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED) {
			return NULL;
		}
		arena_head = addr;
		arena = (malloc_arena)addr;
		arena->size = request_memory;
		arena->next = NULL;
		arena->prev = NULL;
	} else {
		malloc_arena start = find_arena(size);
		if (!start) {
			return NULL;
		}
		void *addr = mmap(start->data, request_memory, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED) {
			return NULL;
		}
		arena = (malloc_arena)addr;
		start->next = arena;
		arena->prev = start;
		arena->size = request_memory;
	}
	current_arenas += 1;
	return arena;
}

void split_block(malloc_arena arena, block b, size_t s) {
	int required_order = get_buddy_order(s);
	while (b->buddy_order != required_order) {
		buddy_split(arena, b);
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
