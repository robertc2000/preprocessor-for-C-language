#include "parse.h"

struct constant_value_pair* alloc_const_val_pair(int* status) {
	struct constant_value_pair* new_pair = calloc(1, sizeof(struct constant_value_pair));

	if (!new_pair) {
		*status = ENOMEM;
		return NULL;
	} else {
		*status = SUCCESS;
	}

	return new_pair;
}

int hash_function(void* pair) {
	int result = strlen(((struct constant_value_pair*)pair)->constant);
	return result;
}

char* get_constant_value(struct hashmap* map, char* constant) {
	int constant_size = strlen(constant);
	struct list* l = map->lists[constant_size];

	while (l) {
		struct constant_value_pair* current_cell = (struct constant_value_pair*) l->data;
		if (strncmp(current_cell->constant, constant, constant_size) == 0)
			return current_cell->value;

		l = l->next;
	}

	return NULL;
}

int process_constants(struct hashmap* map, char* value) {
	// replaces all the constants in the value of a constant
	// e.g #define A B + 3 => B will be replaced by its value
	int len, aux_index, status;
	int current_index = 0;
	char* aux;

	if (strlen(value) == 0) {
		return SUCCESS;
	}

	len = strlen(value);
	aux = calloc(MAX_CONSTANT_NAME, sizeof(char));
	if (!aux) {
		return ENOMEM;
	}

	aux_index = 0;
	status = 0;

	while (current_index < len) {
		if (is_constant_start(value, current_index)) {
			char* const_value;

			char* constant = get_constant(value + current_index, &status);
			if (!constant) {
				free(aux);
				return ENOMEM;
			}

			const_value = get_constant_value(map, constant);

			if (const_value) {
				strcat(aux, const_value);
				aux_index += strlen(const_value);
			} else {
				strcat(aux, constant);
				aux_index += strlen(constant);
			}

			current_index += strlen(constant);

			free(constant);
		} else {
			aux[aux_index] = value[current_index];
			current_index++;
			aux_index++;
		}
	}

	memset(value, 0, MAX_VALUE_NAME);
	memcpy(value, aux, MAX_VALUE_NAME);

	free(aux);

	return 1;
}

int parse_define(struct hashmap* map, char* start, char** end) {
	int index = 8; // strlen("#define")
	int alloc_status;
	char* constant_name, *value;
	struct constant_value_pair* pair;

	// get constant name
	constant_name = calloc(MAX_CONSTANT_NAME, sizeof(char));
	if (!constant_name) {
		return ENOMEM;
	}

	get_constant_name(start + index, constant_name);

	// get value
	value = calloc(MAX_VALUE_NAME, sizeof(char));
	if (!value) {
		free(constant_name);
		return ENOMEM;
	}

	index += strlen(constant_name);
	get_value_name(start + index, value);

	// if the value contains constants, they will be replaced
	alloc_status = process_constants(map, value);
	if (alloc_status == ENOMEM) {
		free(constant_name);
		free(value);
		return ENOMEM;
	}

	pair = alloc_const_val_pair(&alloc_status);
	if (alloc_status == ENOMEM) {
		free(constant_name);
		free(value);
		return ENOMEM;
	}
	pair->constant = constant_name;
	pair->value = value;

	alloc_status = add_entry(map, pair, hash_function);
	if (alloc_status == ENOMEM) {
		free(constant_name);
		free(value);
		free(pair);
		return ENOMEM;
	}

	*end = start + jump_define(start);

	return SUCCESS;
}

int cmp_constants(void* pair_1, void* pair_2) {
	struct constant_value_pair* p1 = (struct constant_value_pair*) pair_1;
	struct constant_value_pair* p2 = (struct constant_value_pair*) pair_2;

	int result = !strcmp(p1->constant, p2->constant);

	return result;
}

void free_pair(void* pair) {
	struct constant_value_pair* p = (struct constant_value_pair*) pair;

	free(p->value);
	free(p->constant);
	free(p);
}

void parse_undefine(struct hashmap* map, char* start, char** end) {
	char cst[MAX_CONSTANT_NAME];
	struct constant_value_pair temp;

	*end = start;
	for (; **end != '\n'; (*end)++);
	(*end)++;

	memset(cst, 0, sizeof(cst));
	start += strlen(UNDEF);

	strncpy(cst, start, *end - start - 1);

	temp.constant = cst;
	temp.value = NULL;
	delete_entry(map, &temp, hash_function, cmp_constants, free_pair);
}

static char* process_branch(struct hashmap* map, char* start, char** end, int* alloc_status) {
	int output_buf_index = 0;

	char* code = calloc(MAX_CODE_SIZE, sizeof(char));
	if (!code) {
		*alloc_status = ENOMEM;
		return NULL;
	}

	while (1) {
		parse(&start, code, &output_buf_index, alloc_status, map);

		if (*alloc_status == ENOMEM) {
			free(code);
			return NULL;
		}

		if (starts_with(start, ENDIF) || starts_with(start, ELSE)) {
			break;
		}
	}

	*end = start;

	return code;
}

static char* jump_to_endif(char* start) {
	int n = 1;

	while (1) {
		if (is_ifdef(start) || is_if(start)) {
			n++;
		}

		if (starts_with(start, ENDIF)) {
			n--;

			if (n == 0) {
				while (*start != '\0' && *start != '\n') {
					start++;
				}

				break;
			}
		}

		start++;
	}

	return start;
}

static char* jump_to_else(char* start, int* else_exists) {
	while (1) {
		if (starts_with(start, ELSE)) {
			start += strlen(ELSE) + 1;
			*else_exists = 1;
			break;
		}

		if (starts_with(start, ENDIF)) {
			*else_exists = 0;
			break;
		}

		start++;
	}

	return start;
}

char* process_ifdef(struct hashmap* map, char* start, char** end, int* alloc_status) {
	int ifdef, take_if_branch;
	char cst[MAX_CONSTANT_NAME];
	struct constant_value_pair temp;
	void* exists;
	char* code;

	ifdef = is_ifdef(start); // 1 -> is ifdef; 2 -> is ifndef

	memset(cst, 0, sizeof(cst));

	if (ifdef == IFDEF_V) {
		start += strlen(IFDEF);
	} else {
		start += strlen(IFNDEF);
	}

	*end = start;
	for (; **end != '\n'; (*end)++);

	strncpy(cst, start, *end - start);
	temp.constant = cst;
	temp.value = NULL;
	
	exists = already_exists(map, &temp, hash_function, cmp_constants);

	if (exists != NULL && ifdef == IFDEF_V) {
		take_if_branch = 1;
	} else if (exists == NULL && ifdef == IFNDEF_V) {
		take_if_branch = 1;
	} else {
		take_if_branch = 0;
	}

	(*end)++;
	start = *end;
	code = NULL;

	if (take_if_branch) {
		code = process_branch(map, start, end, alloc_status);
		if (*alloc_status == ENOMEM) {
			return NULL;
		}
		*end = jump_to_endif(start);
	} else {
		// take else branch
		int else_exists = 0;
		start = jump_to_else(start, &else_exists);

		if (else_exists) {
			code = process_branch(map, start, end, alloc_status);
			if (*alloc_status == ENOMEM) {
				return NULL;
			}

		}

		*end = jump_to_endif(start);
	}

	return code;
}