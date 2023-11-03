#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/* TODO : add your implementation for doubly-linked list */
typedef struct dll_node_t dll_node_t;
struct dll_node_t {
	void *data;
	dll_node_t *prev, *next;
};

typedef struct doubly_linked_list_t doubly;
struct doubly_linked_list_t {
	dll_node_t *head;
	unsigned int data_size;
	unsigned int size;
	unsigned int max_size;
};

typedef struct {
	uint64_t start_address;
	size_t size;
	doubly *miniblock_list;
} block_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
} miniblock_t;

typedef struct {
	uint64_t arena_size;
	doubly *alloc_list;
} arena_t;

dll_node_t *dll_get(doubly *list, unsigned int n);
void dll_free(doubly **pp_list);
doubly *dll_create(unsigned int data_size);
void add_node(doubly *list, unsigned int n, const void *new_data);

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void
write
(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);
