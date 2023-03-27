#include "hashmap.h"

int init_hashmap(struct hashmap** m, int size) {
	*m = malloc(sizeof(struct hashmap));

	if (!(*m)) {
		return ENOMEM;
	}

	(*m)->size = size;
	(*m)->lists = calloc(size, sizeof(struct list*));

	if (!((*m)->lists)) {
		free(*m);
		return ENOMEM;
	}

	return SUCCESS;
}

void delete_hashmap(struct hashmap** m, void (*free_cell_t)(void*)) {
	int i;
	for (i = 0; i < (*m)->size; i++) {
		free_list(&((*m)->lists[i]), free_cell_t);
	}

	free((*m)->lists);
	free(*m);
}

int add_entry(struct hashmap* m, void* data, int (*hash_function) (void*)) {
	int index = hash_function(data);
	return cons(&(m->lists[index]), data);
}

void delete_entry(struct hashmap *m, void* data, int (*hash_f)(void*),
		int (*is_equal)(void*, void*), void (*free_function)(void*)) {

	del_element(&(m->lists[hash_f(data)]), data, is_equal, free_function);
}

void* already_exists(struct hashmap *m, void* data, int (*hash_f)(void*), int (*is_equal)(void*, void*)) {
	struct list* l = m->lists[hash_f(data)];

	while (l) {
		if (is_equal(data, l->data)) {
			return l->data;
		}

		l = l->next;
	}

	return NULL;
}

void free_hashmap(struct hashmap* m, void (*free_function)(void*)) {
	int i;
	for (i = 0; i < m->size; i++) {
		if (m->lists[i]) {
			free_list(&(m->lists[i]), free_function);
		}
	}

	free(m->lists);
	free(m);
}