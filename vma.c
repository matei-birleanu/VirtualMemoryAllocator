#include "vma.h"
#include <stdio.h>
dll_node_t *dll_remove_nth_node(doubly *list, unsigned int n)
{
	if (list->size == 0)
		return NULL;
	if (n >= list->size - 1)
		n = list->size - 1;
	if (n == 0) {
		dll_node_t *removed = list->head;
		if (list->size > 1) {
			list->head = list->head->next;
			list->head->prev = NULL;
		} else {
			list->head = NULL;
		}
		list->size--;
		return removed;
	}
	dll_node_t *current = list->head;
	while (n--)
		current = current->next;
	dll_node_t *removed = current;
	if (!current->next) {
		dll_node_t *prev_node = current->prev;
		prev_node->next = NULL;
		list->size--;
		return removed;
	}
	dll_node_t *prev_node = current->prev;
	dll_node_t *next_node = current->next;
	prev_node->next = current->next;
	next_node->prev = current->prev;
	list->size--;
	return removed;
}

dll_node_t *dll_get(doubly *list, unsigned int n)
{
	dll_node_t *node;
	if (!list || !list->size)
		return NULL;
	n = n % list->size;
	node = list->head;
	for (unsigned int i = 0; i < n; i++)
		node = node->next;
	return node;
}

void dll_free(doubly **pp_list)
{
	dll_node_t *node;
	if (!pp_list || !*pp_list)
		return;
	while ((*pp_list)->size) {
		node = dll_remove_nth_node(*pp_list, 0);
		free(node->data);
		free(node);
	}
	free(*pp_list);
	*pp_list = NULL;
}

void dll_free_mini(doubly **pp_list)
{
	dll_node_t *node;
	if (!pp_list || !*pp_list)
		return;
	while ((*pp_list)->size) {
		node = dll_remove_nth_node(*pp_list, 0);
		free(((miniblock_t *)node->data)->rw_buffer);
		free(node->data);
		free(node);
	}
	free(*pp_list);
	*pp_list = NULL;
}

doubly *dll_create(unsigned int data_size)
{
	doubly *list;
	list = malloc(sizeof(doubly));
	list->data_size = data_size;
	list->size = 0;
	list->head = NULL;
	return list;
}

void add_node(doubly *list, unsigned int n, const void *new_data)
{
	dll_node_t *new_node = malloc(sizeof(dll_node_t));
	new_node->data = malloc(list->data_size);
	memcpy(new_node->data, new_data, list->data_size);
	if (list->size == 0)
		n = 0;
	if (n == 0) {
		new_node->prev = NULL;
		new_node->next = list->head;
		if (list->head)
			list->head->prev = new_node;
		list->head = new_node;
		list->size++;
		return;
	} else if (n >= list->size) {
		new_node->next = NULL;
		dll_node_t *curr = list->head;
		while (curr->next)
			curr = curr->next;
		curr->next = new_node;
		new_node->prev = curr;
		list->size++;
		return;
	}
	dll_node_t *curr = list->head;
	while (--n)
		curr = curr->next;
	dll_node_t *next_node = curr->next;
	new_node->next = curr->next;
	new_node->prev = curr;
	curr->next = new_node;
	next_node->prev = new_node;
	list->size++;
}

arena_t *alloc_arena(const uint64_t size)
{
	doubly *list;
	arena_t *arena;
	arena = (arena_t *)malloc(sizeof(arena_t));
	list = (doubly *)dll_create(sizeof(block_t));
	arena->alloc_list = list;
	arena->arena_size = size;
	return arena;
}

void dealloc_arena(arena_t *arena)
{
	doubly *list;
	list = arena->alloc_list;
	dll_node_t *node;
	int n = list->size;
	node = list->head;
	for (int i = 0; i < n; i++) {
		dll_node_t *head;
		head = ((block_t *)node->data)->miniblock_list->head;
		unsigned int k = (((block_t *)node->data)->miniblock_list)->size;
		while (k) {
			head = dll_get(((block_t *)node->data)->miniblock_list, k);
			if (((miniblock_t *)head->data)->rw_buffer)
				free(((miniblock_t *)head->data)->rw_buffer);
			k--;
		}
		dll_free(&(((block_t *)node->data)->miniblock_list));
		node = node->next;
	}
	dll_free(&arena->alloc_list);
	free(arena);
}

miniblock_t
*alloc_miniblock(const uint64_t address, const uint64_t size,  uint8_t perm)
{
	miniblock_t *minibloc = malloc(sizeof(miniblock_t));
	if (perm == '6')
		minibloc->perm = '6';
	minibloc->start_address = address;
	minibloc->size = size;
	minibloc->rw_buffer = NULL;
	return minibloc;
}

void lipire(doubly *l_mini, doubly **lista, miniblock_t *mini)
{
	doubly *sters = *lista;
	dll_node_t *node;
	unsigned int poz;
	poz = l_mini->size;
	///printf("Lipesc miniul nou pe poz %d in lista din stanga\n",poz);
	add_node(l_mini, poz, mini);
	node = l_mini->head;
	while (node->next)
		node = node->next;
	node->next = sters->head;
	sters->head->prev = node;
	free(sters);
	sters = NULL;
}

void ajutor3(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t bloc;
	bloc.size = size;
	bloc.start_address = address;
	doubly *l_mini;
	miniblock_t *minibloc = alloc_miniblock(address, size, '6');
	l_mini = dll_create(sizeof(miniblock_t));
	bloc.miniblock_list = l_mini;
	add_node(l_mini, 0, minibloc);
	add_node(arena->alloc_list, 0, &bloc);
	free(minibloc);
}

void ajutor2(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t bloc;
	doubly *l_mini;
	bloc.size = size;
	bloc.start_address = address;
	dll_node_t *node = dll_get(arena->alloc_list, arena->alloc_list->size - 1);
	uint64_t adr = ((block_t *)node->data)->start_address;
	size_t sz = ((block_t *)node->data)->size;
	if (address < adr + sz) {
		printf("This zone was already allocated.\n");
		return;
	}
	///este in continuarea miniblock urilor ultimului nod
	if (address == adr + sz) {
		l_mini = ((block_t *)node->data)->miniblock_list;
		miniblock_t *minibloc = alloc_miniblock(address, size, '6');
		add_node(l_mini, l_mini->size, minibloc);
		sz += size;
		free(minibloc);
		return;
	}
	l_mini = dll_create(sizeof(miniblock_t));
	miniblock_t *minibloc = alloc_miniblock(address, size, '6');
	bloc.miniblock_list = l_mini;
	add_node(l_mini, 0, minibloc);
	add_node(arena->alloc_list, arena->alloc_list->size, &bloc);
	free(minibloc);
}

void ajutor(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t bloc;
	doubly *l_mini;
	bloc.size = size;
	bloc.start_address = address;
	uint64_t adr;
	adr = ((block_t *)arena->alloc_list->head->data)->start_address;
	size_t sz = ((block_t *)arena->alloc_list->head->data)->size;
	if (address > adr + sz) {
		l_mini = dll_create(sizeof(miniblock_t));
		miniblock_t *minibloc = alloc_miniblock(address, size, '6');
		add_node(l_mini, 0, minibloc);
		bloc.miniblock_list = l_mini;
		add_node(arena->alloc_list, 1, &bloc);
		free(minibloc);
		return;
	}
	if (address < adr && address + size < adr) {
		l_mini = dll_create(sizeof(miniblock_t));
		miniblock_t *minibloc = alloc_miniblock(address, size, '6');
		add_node(l_mini, 0, minibloc);
		bloc.miniblock_list = l_mini;
		add_node(arena->alloc_list, 0, &bloc);
		free(minibloc);
		return;
	}
	if (address + size == adr) {
		l_mini = ((block_t *)arena->alloc_list->head->data)->miniblock_list;
		miniblock_t *minibloc = alloc_miniblock(address, size, '6');
		add_node(l_mini, 0, minibloc);
		((block_t *)arena->alloc_list->head->data)->start_address = address;
		((block_t *)arena->alloc_list->head->data)->size += size;
		free(minibloc);
		return;
	}
	if (adr + sz == address) {
		///printf("am intrat\n");
		///printf("%ld + %ld = %ld",adr, sz, address);
		l_mini = ((block_t *)arena->alloc_list->head->data)->miniblock_list;
		miniblock_t *minibloc = alloc_miniblock(address, size, '6');
		add_node(l_mini, sz, minibloc);
		((block_t *)arena->alloc_list->head->data)->size += size;
		free(minibloc);
		return;
	}
	printf("This zone was already allocated.\n");
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t bloc;
	bloc.size = size;
	bloc.start_address = address;
	doubly *l_mini;
	if (arena->alloc_list->size == 0) {
		l_mini = dll_create(sizeof(miniblock_t));
		miniblock_t *minibloc = alloc_miniblock(address, size, '6');
		add_node(l_mini, 0, minibloc);
		bloc.miniblock_list = l_mini;
		add_node(arena->alloc_list, arena->alloc_list->size, &bloc);
		free(minibloc);
		return;
	}
	if (arena->alloc_list->size == 1) {
		ajutor(arena, address, size);
		return;
	}
	dll_node_t *aux, *curr, *sters;
	unsigned int poz = 0;
	curr = arena->alloc_list->head;
	aux = curr->next;
	unsigned long adr1, adr2;
	dll_node_t *node = dll_get(arena->alloc_list, arena->alloc_list->size - 1);
	uint64_t adr = ((block_t *)node->data)->start_address;
	if (address > adr) {
		ajutor2(arena, address, size);
		return;
	}
	if (address < ((block_t *)curr->data)->start_address) {
		ajutor3(arena, address, size);
		return;
	}
	while (1) {
		adr1 = ((block_t *)curr->data)->start_address;
		adr2 = ((block_t *)aux->data)->start_address;
		if (address > adr1 && address < adr2) {
			size_t size1;
			size1 = ((block_t *)curr->data)->size;
			if (adr1 + size1 - 1 < address - 1 &&
				address + size - 1 < adr2 - 1) {
				miniblock_t *minibloc = alloc_miniblock(address, size, '6');
				l_mini = dll_create(sizeof(miniblock_t));
				bloc.miniblock_list = l_mini;
				add_node(l_mini, 0, minibloc);
				add_node(arena->alloc_list, poz + 1, &bloc);
				free(minibloc);
				break;
			}
			if (adr1 + size1 == address && address + size == adr2) {
				miniblock_t *mini = alloc_miniblock(address, size, '6');
				size_t size_block = ((block_t *)aux->data)->size;
				int size_mini = ((block_t *)aux->data)->miniblock_list->size;
				l_mini = ((block_t *)curr->data)->miniblock_list;
				lipire(l_mini, &((block_t *)aux->data)->miniblock_list, mini);
				((block_t *)curr->data)->size += size_block + size;
				((block_t *)curr->data)->miniblock_list->size += size_mini;
				sters = dll_remove_nth_node(arena->alloc_list, poz + 1);
				free(sters->data);
				free(sters);
				free(mini);
				break;
			}
			if (adr1 + size1 - 1 == address - 1) {///lipire stanga
				miniblock_t *minibloc = alloc_miniblock(address, size, '6');
				int pozitie = ((block_t *)curr->data)->miniblock_list->size;
				l_mini = ((block_t *)curr->data)->miniblock_list;
				add_node(l_mini, pozitie - 1, minibloc);
				((block_t *)curr->data)->size += size;
				free(minibloc);
				return;
			}
			printf("This zone was already allocated.\n");
			return;
		}
			poz++;
			curr = curr->next;
			aux = curr->next;
	}
}

void free_block(arena_t *arena, const uint64_t address)
{
	doubly *list = arena->alloc_list, *lista_mini;
	int poz = 0, ok = 0, pozitie;
	dll_node_t *node, *mini, *new_head;
	size_t size1, sum1 = 0, sz;
	node = list->head;
	uint64_t adr;
	while (node) {
		adr = ((block_t *)node->data)->start_address;
		sz  = ((block_t *)node->data)->size;
		if (address >= adr && address < adr + sz) {
			ok = 1; break;
		}
		node = node->next; poz++;
	}
	if (ok == 0) {
		printf("Invalid address for free.\n");
		return;
	}
	lista_mini = ((block_t *)node->data)->miniblock_list;
	if (lista_mini->size == 1) {
		if (address != ((miniblock_t *)lista_mini->head->data)->start_address) {
			puts("Invalid address for free."); return;
		}
		dll_free(&lista_mini);
		dll_node_t *removed = dll_remove_nth_node(list, poz);
		free(removed->data); free(removed); return;
	}
	ok = 0;
	mini = lista_mini->head;
	for (unsigned int i = 0; i < lista_mini->size; i++) {
		if (address == ((miniblock_t *)mini->data)->start_address) {
			pozitie = i; ok = 1; break;
		}
		mini = mini->next;
	}
	if (ok == 0) {
		printf("Invalid address for free.\n"); return;
	}
	if (pozitie == 0 || (unsigned int)pozitie == lista_mini->size - 1) {
		((block_t *)node->data)->size -= ((miniblock_t *)mini->data)->size;
		dll_node_t *removed = dll_remove_nth_node(lista_mini, pozitie);
		if (((miniblock_t *)removed->data)->rw_buffer)
			free(((miniblock_t *)removed->data)->rw_buffer);
		free(removed->data); free(removed);
		void *y = ((block_t *)node->data)->miniblock_list->head->data;
		uint64_t x = ((miniblock_t *)y)->start_address;
		if (pozitie == 0)
			((block_t *)node->data)->start_address = x;
		return;
	}
	mini = lista_mini->head;
	for (int i = 0 ; i < pozitie; i++) {
		sum1 += ((miniblock_t *)mini->data)->size; mini = mini->next;
	}
	((block_t *)node->data)->size = sum1;
	new_head = mini->next;
	dll_node_t *copie = dll_get(lista_mini, pozitie - 1);
	dll_node_t *removed = dll_remove_nth_node(lista_mini, pozitie);
	copie->next->prev = NULL;
	copie->next = NULL;
	lista_mini->size = pozitie;
	if (((miniblock_t *)removed->data)->rw_buffer)
		free(((miniblock_t *)removed->data)->rw_buffer);
	free(removed->data); free(removed);
	block_t bloc;
	sum1 = 0; size1 = 0;
	dll_node_t *aux = new_head;
	while (aux) {
		sum1 += ((miniblock_t *)aux->data)->size;
		aux = aux->next;
		size1++;
	}
	bloc.miniblock_list = dll_create(sizeof(miniblock_t));
	bloc.size = sum1;
	bloc.start_address = ((miniblock_t *)new_head->data)->start_address;
	bloc.miniblock_list->data_size = sizeof(miniblock_t);
	bloc.miniblock_list->head = new_head;
	bloc.miniblock_list->size = size1;
	add_node(list, poz + 1, &bloc);
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	doubly *lista_miniblocuri, *lista_bloc;
	lista_bloc = arena->alloc_list;
	uint64_t ramas = size, capacity = 0;
	int ok = 0;
	dll_node_t *node;
	block_t *bloc_gasit;
	node = lista_bloc->head;
	uint64_t adr;
	size_t sz;
	while (node) {
		adr = ((block_t *)node->data)->start_address;
		sz = ((block_t *)node->data)->size;
		if (address >= adr  && address < adr + sz) {
			ok = 1;
			break;
		}
		node = node->next;
	}
	if (ok == 0) {
		printf("Invalid address for read.\n");
		return;
	}
	bloc_gasit = ((block_t *)node->data);
	lista_miniblocuri = bloc_gasit->miniblock_list;
	if (address + size > bloc_gasit->start_address + bloc_gasit->size) {
		size = bloc_gasit->start_address + bloc_gasit->size - address;
		printf("Warning: size was bigger than the block size.");
		printf(" Reading %ld characters.\n", size);
	}
	dll_node_t *mini = lista_miniblocuri->head;
	while (mini) {
		adr = ((miniblock_t *)mini->data)->start_address;
		sz = ((miniblock_t *)mini->data)->size;
		if (address >= adr && address <= adr + sz)
			break;
		mini = mini->next;
	}
	uint64_t x = ((miniblock_t *)mini->data)->start_address;
	capacity = x + ((miniblock_t *)mini->data)->size - address;
	if (capacity >= size) {
		uint64_t x = ((miniblock_t *)mini->data)->start_address;
		char *m = (char *)((miniblock_t *)mini->data)->rw_buffer;
		printf("%.*s\n", (int)size, m + address - x);
		return;
	}
	char *b = (char *)((miniblock_t *)mini->data)->rw_buffer;
	printf("%.*s", (int)capacity, b);
	ramas = size;
	ramas = ramas - capacity;
	mini = mini->next;
	while (ramas) {
		capacity = ((miniblock_t *)mini->data)->size;
		if (capacity >= ramas) {
			char *m = (char *)((miniblock_t *)mini->data)->rw_buffer;
			printf("%.*s\n", (int)ramas, m);
			return;
		}
		char *u = (char *)((miniblock_t *)mini->data)->rw_buffer;
		printf("%.*s", (int)capacity, u);
		ramas = ramas - capacity;
		mini = mini->next;
	}
}

void
write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	int ok = 0;
	doubly *lista_miniblocuri, *lista_bloc;
	uint64_t capacity = 0, copie_size = size;
	uint64_t ramas = size;
	lista_bloc = arena->alloc_list;
	dll_node_t *node;
	node = lista_bloc->head;
	block_t *bloc_gasit;
	uint64_t adr;
	size_t sz;
	while (node) {
		adr = ((block_t *)node->data)->start_address;
		sz = ((block_t *)node->data)->size;
		if (address >= adr && address <= adr + sz) {
			ok = 1;
			break;
		}
		node = node->next;
	}
	if (ok == 0) {
		printf("Invalid address for write.\n");
		return;
	}
	bloc_gasit = ((block_t *)node->data);
	lista_miniblocuri = bloc_gasit->miniblock_list;
	if (address + size > bloc_gasit->start_address + bloc_gasit->size) {
		copie_size = bloc_gasit->start_address + bloc_gasit->size - address;
		printf("Warning: size was bigger than the block size.");
		printf(" Writing %ld characters.\n", copie_size);
	}
	uint64_t index = 0;
	dll_node_t *mini;
	mini = lista_miniblocuri->head;
	uint64_t h;
	size_t g;
	while (mini) {
		h = ((miniblock_t *)mini->data)->start_address;
		g = ((miniblock_t *)mini->data)->size;
		if (address >= h && address < h + g)
			break;
		mini = mini->next;
	}
	h = ((miniblock_t *)mini->data)->start_address;
	g = ((miniblock_t *)mini->data)->size;
	capacity = h + g - address;
	if (capacity >= copie_size) {
		((miniblock_t *)mini->data)->rw_buffer = calloc(size, sizeof(char));
		strncpy(((miniblock_t *)mini->data)->rw_buffer, (char *)data, size);
		return;
	}
	((miniblock_t *)mini->data)->rw_buffer = malloc(capacity * sizeof(char));
	strncpy(((miniblock_t *)mini->data)->rw_buffer, (char *)data, capacity);
	index = index + capacity;
	ramas = copie_size - capacity;
	mini = mini->next;
	while (ramas) {
		capacity = ((miniblock_t *)mini->data)->size;
		if (capacity >= ramas) {
			uint64_t p = ramas;
			((miniblock_t *)mini->data)->rw_buffer = calloc(p, sizeof(char));
			char *mem;
			mem = (char *)data + index;
			strncpy(((miniblock_t *)mini->data)->rw_buffer, mem, ramas);
			ramas = 0;
			return;
		}
		uint64_t r = capacity;
		((miniblock_t *)mini->data)->rw_buffer = malloc(r * sizeof(char));
		char *yu = (char *)data + index;
		strncpy(((miniblock_t *)mini->data)->rw_buffer, yu, capacity);
		index = index + capacity;
		ramas = ramas - capacity;
		mini = mini->next;
	}
	printf("\n");
}

void pmap(const arena_t *arena)
{
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);
	dll_node_t *node;
	int i;
	size_t sum = 0;
	unsigned int nr_miniblock = 0;
	for (i = 0; (unsigned int)i < arena->alloc_list->size; i++) {
		node = dll_get(arena->alloc_list, i);
		sum += ((block_t *)node->data)->size;
		unsigned int r = ((block_t *)node->data)->miniblock_list->size;
		nr_miniblock = nr_miniblock + r;
	}
	char permis[4] = {"RW-"};
	printf("Free memory: 0x%lX bytes\n", arena->arena_size - sum);
	printf("Number of allocated blocks: %d\n", arena->alloc_list->size);
	printf("Number of allocated miniblocks: %d\n", nr_miniblock);
	if (nr_miniblock)
		puts("");
	for (i = 0; (unsigned int)i < arena->alloc_list->size; i++) {
		node = dll_get(arena->alloc_list, i);
		printf("Block %d begin\n", i + 1);
		block_t *bloc = ((block_t *)node->data);
		uint64_t a = bloc->start_address;
		printf("Zone: 0x%lX - 0x%lX\n", a, bloc->start_address + bloc->size);
		for (int k = 0; (unsigned int)k < bloc->miniblock_list->size; k++) {
			dll_node_t *aux;
			aux = dll_get(bloc->miniblock_list, k);
			miniblock_t *minibloc = ((miniblock_t *)aux->data);
			if (minibloc->perm == '6')
				strcpy(permis, "RW-");
			uint64_t end = minibloc->start_address + (uint64_t)minibloc->size;
			uint64_t rdr = minibloc->start_address;
			printf("Miniblock %d:\t\t0x%lX\t\t-", k + 1, rdr);
			printf("\t\t0x%lX\t\t| %s\n", end, permis);
		}
		printf("Block %d end\n", i + 1);
		if ((unsigned int)i != arena->alloc_list->size - 1)
			puts("");
	}
}

/*void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{

}*/
