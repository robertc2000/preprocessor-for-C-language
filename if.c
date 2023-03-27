#include "parse.h"

static int get_directive(char* start) {
	if (starts_with(start, IF)) {
		return IF_DIR_V;
	}

	if (starts_with(start, ELIF)) {
		return ELIF_DIR_V;
	}

	if (starts_with(start, ELSE)) {
		return ELSE_DIR_V;
	}

	return ERROR;
}

static int eval_expression(struct hashmap* map, char* start) {
	// start has the form: #if COND / #elif COND / #else
	int directive = get_directive(start);

	if (directive == IF_DIR_V || directive == ELIF_DIR_V) {
		char* dir, *current, *end, *value, *cond;
		char condition[MAX_CONSTANT_NAME];

		if (directive == IF_DIR_V) {
			dir = IF;
		} else {
			dir = ELIF;
		}

		current = start + strlen(dir);
		end = current;

		while (*end != '\n') {
			end++;
		}

		memset(condition, 0, MAX_CONSTANT_NAME);

		strncpy(condition, current, end - current);

		// condition is either a macro or a number
		value = get_constant_value(map, condition);
		cond = NULL;

		if (!value) {
			// condition is a number
			cond = condition;
		} else {
			cond = value;
		}

		return atoi(cond);
	}

	// else case is taken; else will always execute if it is reached there
	return 1;
}

static char* get_if_code(struct hashmap* map, char* start, int* alloc_status) {
	// returns the section of the code (in the "#if / #endif" directive) which will be executed
	// "start" starts with #if / #elif or #else
	int c_index = 0;

	char* code = calloc(MAX_CODE_SIZE, sizeof(char));
	if (!code) {
		*alloc_status = ENOMEM;
		return NULL;
	}

	for (; *start != '\n'; start++); // reach code section
	start++;
	
	do {
		parse(&start, code, &c_index, alloc_status, map);
		if (*alloc_status == ENOMEM) {
			free(code);
			return NULL;
		}

	} while (!starts_with(start, ELIF) && !starts_with(start, ELSE) && 
				!starts_with(start, ENDIF));
	
	*alloc_status = SUCCESS;
	return code;
}

static char* jump_to_endif(char* start) {
	while (strncmp(start, ENDIF, strlen(ENDIF))) {
		start++;
	}

	start += strlen(ENDIF);

	return start;
}

static char* jump_to_next_cond(char* start) {
	// jumps 2 newlines
	int i;
	for (i = 0; i < 2; start++) {
		if (*start == '\n') {
			i++;
		}
	}

	return start;
}

char* parse_if(struct hashmap* map, char* start, char** end, int* alloc_status) {
	int cond_true = 0;

	while (!cond_true) {
		cond_true = eval_expression(map, start);

		if (cond_true) {
			char* code = get_if_code(map, start, alloc_status);
			if (*alloc_status == ENOMEM) {
				return NULL;
			}

			*end = jump_to_endif(start);
			return code;
		} else {
			start = jump_to_next_cond(start);
			if (starts_with(start, ENDIF)) {
				*end = jump_to_endif(start);
				return NULL;
			}
		}
	}

	return NULL;
}