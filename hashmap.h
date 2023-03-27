#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "list.h"

struct hashmap {
	int size;
	struct list** lists;
};

int init_hashmap(struct hashmap** m, int size);

void delete_hashmap(struct hashmap** m, void (*free_cell_t)(void*));

int add_entry(struct hashmap* m, void* data, int (*f) (void*));

void delete_entry(struct hashmap *m, void* data, int (*hash_f)(void*), int (*is_equal)(void*, void*), void (*free_function)(void*));

void* already_exists(struct hashmap *m, void* data, int (*hash_f)(void*), int (*is_equal)(void*, void*));

void free_hashmap(struct hashmap* m, void (*free_function)(void*));