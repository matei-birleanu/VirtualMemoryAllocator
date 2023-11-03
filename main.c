#include "vma.h"
#include <stdio.h>
int main(void)
{
	char command[15] = {0};
	uint64_t maxsize, size, start;
	arena_t *arena;
	while (1) {
		scanf("%s", command);
		if (strcmp(command, "DEALLOC_ARENA") == 0)
			break;
		if (strcmp(command, "ALLOC_ARENA") == 0) {
			scanf("%ld", &maxsize);
			arena = alloc_arena(maxsize);
			continue;
		}
		if (strcmp(command, "ALLOC_BLOCK") == 0) {
			scanf("%ld", &start);
			scanf("%ld", &size);
			if (start >= maxsize) {///rezolvat sa nu mai faca alloc block
				printf("The allocated address is outside the size of arena\n");
				continue;
			}
			if (start + size > maxsize) {
				printf("The end address is past the size of the arena\n");
				continue;
			}
			alloc_block(arena, start, size);
			continue;
		}
		if (strcmp(command, "PMAP") == 0) {
			pmap(arena);
			continue;
		}
		if (strcmp(command, "FREE_BLOCK") == 0) {
			scanf("%ld", &start);
			if (arena->alloc_list->size == 0) {
				printf("Invalid address for free.\n");
				continue;
			}
			if (start > maxsize) {
				printf("The allocated address is outside the size of arena\n");
				continue;
			} else {
				free_block(arena, start);
				}
			continue;
		}
		if (strcmp(command, "WRITE") == 0) {
			char *cuv;
			if (scanf("%ld%ld", &start, &size) != 2) {
				printf("Invalid command. Please try again.\n");
				continue;
			}
			cuv = calloc(size, sizeof(char));
			char dump;
			fread(&dump, sizeof(char), 1, stdin);
			fread(cuv, sizeof(char), size, stdin);
			write(arena, start, size, (int8_t *)cuv);
			free(cuv);
			scanf(" ");
			continue;
		}
		if (strcmp(command, "READ") == 0) {
			if (scanf("%ld%ld", &start, &size) != 2) {
				printf("Invalid command. Please try again.\n");
				continue;
			}
			if (size > maxsize) {
				printf("The allocated address is outside the size of arena\n");
				continue;
			}
			read(arena, start, size);
			continue;
		}
	   printf("Invalid command. Please try again.\n");
	}
	dealloc_arena(arena);
	return 0;
}
