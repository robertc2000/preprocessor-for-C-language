#include "parse.h"

int is_define(const char* buffer) {
	// returns 1 if buffer starts with "#define "
	return starts_with(buffer, DEFINE);
}

int is_if(const char* buffer) {
	// returns 1 if buffer starts with "#define "
	return starts_with(buffer, IF);
}

int is_undef(const char* buffer) {
	return starts_with(buffer, UNDEF);
}

int is_include(const char* buffer) {
	return starts_with(buffer, INCLUDE);
}

int is_ifdef(const char* buffer) {
	if (starts_with(buffer, IFDEF)) {
		return IFDEF_V;
	} else if (starts_with(buffer, IFNDEF)) {
		return IFNDEF_V;
	}
	
	return NO_IFDEF;
}

char* parse_include(struct hashmap* map, struct list* dirs, char* start, char** end, int* status) {
	char* code, *full_file_path;
	char* filename = get_filename(start, status);
	if (*status == ENOMEM) {
		return NULL;
	}

	for (; **end != '\n'; (*end)++);
	(*end)++;
	
	code = calloc(MAX_READ_SIZE, sizeof(char));
	if (!code) {
		*status = ENOMEM;
		free(filename);
		return NULL;
	}

	full_file_path = calloc(MAX_FILENAME_SIZE, sizeof(char));
	if (!full_file_path) {
		*status = ENOMEM;
		free(filename);
		free(code);
		return NULL;
	}

	strcat(full_file_path, path);
	strcat(full_file_path, filename);

	*status = process_file(full_file_path, NULL, map, code);
	if (*status == ENOMEM) {
		free(filename);
		free(code);
		free(full_file_path);
		return NULL;
	}

	if (*status == FILE_ERROR) {
		int file_found = 0;

		while (dirs) {
			char* new_path = calloc(MAX_FILENAME_SIZE, sizeof(char));
			if (!new_path) {
				*status = ENOMEM;

				free(filename);
				free(code);
				free(full_file_path);

				return NULL;
			}

			// form file path
			strcat(new_path, (char*) dirs->data);
			strcat(new_path, "/");
			strcat(new_path, filename);

			*status = process_file(new_path, NULL, map, code);
			free(new_path);

			if (*status == ENOMEM) {
				free(filename);
				free(code);
				free(full_file_path);
				return NULL;
			}

			if (*status != FILE_ERROR) {
				file_found = 1;
				break;
			}

			dirs = dirs->next;
		}

		if (!file_found) {
			printf("Error opening file\n");

			free(filename);
			free(code);
			free(full_file_path);

			return NULL;
		}
	}

	free(full_file_path);
	free(filename);

	return code;
}

void parse(char** current_index_in, char* out_buffer, int* current_index_out, int* status, struct hashmap* map) {
	char* constant;
	*status = SUCCESS;

	if (**current_index_in == '\"') {
		char* quotes_string = jump_quotes(*current_index_in, current_index_in, status);
		if (*status == ENOMEM) {
			return;
		}

		strcat(out_buffer, quotes_string);
		*current_index_out += strlen(quotes_string);

		free(quotes_string);
		return;
	}

	if (is_ifdef(*current_index_in)) {
		char* code = process_ifdef(map, *current_index_in, current_index_in, status);
		if (*status == ENOMEM) {
			return;
		}

		if (code) {
			strncat(out_buffer, code, strlen(code));
			*current_index_out += strlen(code);
			free(code);
		}

		return;
	}

	if (is_define(*current_index_in)) {
		*status = parse_define(map, *current_index_in, current_index_in);
		return;
	}

	if (is_undef(*current_index_in)) {
		parse_undefine(map, *current_index_in, current_index_in);
		return;
	}

	if (is_if(*current_index_in)) {
		char* code = parse_if(map, *current_index_in, current_index_in, status);
		if (*status == ENOMEM) {
			return;
		}

		if (code) {
			strncat(out_buffer, code, strlen(code));
			*current_index_out += strlen(code);
			free(code);
		}

		return;
	}

	if (is_include(*current_index_in)) {
		char* code = parse_include(map, directories, *current_index_in, current_index_in, status);
		if (*status == ENOMEM || *status == FILE_ERROR) {
			return;
		}

		strncat(out_buffer, code, strlen(code));
		*current_index_out += strlen(code);
		free(code);

		return;
	}

	constant = get_constant(*current_index_in, status);
	if (*status == ENOMEM) {
		return;
	}

	if (constant) {
		char* value = get_constant_value(map, constant);

		if (value) {
			int i;				
			for (i = 0; i < strlen(value); i++) {
				out_buffer[*current_index_out] = value[i];
				(*current_index_out)++;
			}

			*current_index_in += strlen(constant);
			free(constant);
			return;
		}
	}

	free(constant);

	out_buffer[*current_index_out] = **current_index_in;
	(*current_index_in)++;
	(*current_index_out)++;
}

int process_file(char* in_file, char* out_file, struct hashmap* map, char* out_buffer) {
	int status = SUCCESS;
	char* current_index_in;
	int current_index_out;

	char* in_buffer = read_input(in_file, &status);
	if (status == ENOMEM) {
		return ENOMEM;
	} else if (in_buffer == NULL) {
		return FILE_ERROR;
	}

	current_index_in = in_buffer;
	current_index_out = strlen(out_buffer);

	while (*current_index_in != '\0') {
		parse(&current_index_in, out_buffer, &current_index_out, &status, map);
		if (status == ENOMEM) {
			free(in_buffer);
			return ENOMEM;
		}

		if (status == FILE_ERROR) {
			free(in_buffer);
			return FILE_ERROR;
		}
	}

	free(in_buffer);
	return SUCCESS;
}