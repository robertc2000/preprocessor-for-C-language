#include "input.h"

void read_from_stdin(char* in_buffer) {
	char buffer[MAX_READ_SIZE];

	memset(buffer, 0, sizeof(buffer));
	fgets(buffer, MAX_READ_SIZE - 1, stdin);
	strcat(in_buffer, buffer);
}

int read_from_file(char* in_buffer, char* in_file) {
	FILE* fin = fopen(in_file, "r");
	if (!fin) {
		return FILE_ERROR;
	}

	fread(in_buffer, sizeof(char), MAX_READ_SIZE, fin);
	fclose(fin);

	return SUCCESS;
}

char* read_input(char* in_file, int* alloc_status) {
	char* in_buffer = calloc(MAX_READ_SIZE, sizeof(char));
	if (!in_buffer) {
		*alloc_status = ENOMEM;
		return NULL;
	}

	if (in_file == NULL) {
		read_from_stdin(in_buffer);
	} else {
		int result = read_from_file(in_buffer, in_file);
		if (result == FILE_ERROR) {
			free(in_buffer);
			return NULL;
		}
	}

	*alloc_status = SUCCESS;
	return in_buffer;
}

int parse_define_arg(char** argv, int index, int argc, struct hashmap* map, int* alloc_status) {
	char* current = argv[index];
	char* end, *value;
	struct constant_value_pair* pair;
	char* constant = calloc(MAX_CONSTANT_NAME, sizeof(char));
	if (!constant) {
		*alloc_status = ENOMEM;
		return -1;
	}

	value = calloc(MAX_VALUE_NAME, sizeof(char));

	if (!value) {
		*alloc_status = ENOMEM;
		free(constant);
		return -1;
	}

	// retrieve constant
	if (strlen(argv[index]) > 2) {
		// "-DSYMBOL"
		current += 2;
	} else {
		index++;
		current = argv[index];
	}

	end = current;

	while (*end != '\0' && *end != '=') {
		end++;
	}

	if (*end == '=') {
		strncpy(constant, current, end - current);
		current = end + 1;
	} else {
		strncpy(constant, current, end - current);
		index++;

		if (index < argc) {
			current = argv[index];

			if (current[0] == '=') {
				index++;
				current = argv[index];
			} else {
				current = NULL;
			}
		} else {
			current = NULL;
		}
	}

	// retrieve value
	if (current != NULL) {
		// value exists
		strcpy(value, current);
		index++;
	}

	pair = alloc_const_val_pair(alloc_status);
	if (*alloc_status == ENOMEM) {
		free(constant);
		free(value);
		return -1;
	}
	pair->constant = constant;
	pair->value = value;

	//printf("define_3\n");

	*alloc_status = add_entry(map, pair, hash_function);
	if (*alloc_status == ENOMEM) {
		//printf("define_2\n");
		free(constant);
		free(value);
		free(pair);
		return -1;
	}

	//printf("define_1\n");

	return index;
}

int parse_directory_arg(char** argv, int index, struct list** directories, int* alloc_status) {
	char* current = argv[index];
	int max_directory_name = 50;
	char* directory;

	if (strlen(argv[index]) > 2) {
		current += 2;
	} else {
		index++;
		current = argv[index];
	}

	max_directory_name = 50;
	directory = calloc(max_directory_name, sizeof(char));
	if (!directory) {
		*alloc_status = ENOMEM;
		return -1;
	}

	strcpy(directory, current);
	cons(directories, directory);

	*alloc_status = SUCCESS;
	return index + 1;
}

int parse_output_arg(char** argv, int current, char** out_file, int* alloc_status) {
	char* filename_p;

	*out_file = calloc(MAX_FILENAME_SIZE, sizeof(char));
	if (!out_file) {
		*alloc_status = ENOMEM;
		return -1;
	}

	if (strlen(argv[current]) > 2) {
		filename_p = argv[current] + 2;
	} else {
		current++;
		filename_p = argv[current];
	}

	strcpy(*out_file, filename_p);
	return current + 1;
}

int parse_input(char** argv, int argc, struct hashmap* map, struct list** directories, char** input_file, char** out_file, int* alloc_status) {
	int i;
	int read_input_file = 0;

	for (i = 1; i < argc;) {
		if (starts_with(argv[i], "-D")) {
			i = parse_define_arg(argv, i, argc, map, alloc_status);
			if (*alloc_status == ENOMEM) {
				return ENOMEM;
			}
			continue;
		}

		if (starts_with(argv[i], "-I")) {
			i = parse_directory_arg(argv, i, directories, alloc_status);
			if (*alloc_status == ENOMEM) {
				return ENOMEM;
			}
			continue;
		}

		if (starts_with(argv[i], "-o")) {
			i = parse_output_arg(argv, i, out_file, alloc_status);
			continue;
		}

		if (starts_with(argv[i], "-")) {
			return ERROR;
		}

		// input file
		if (read_input_file) {
			if (*out_file) {
				free(*out_file);
				exit(1);
			}

			*out_file = calloc(MAX_FILENAME_SIZE, sizeof(char));
			if (!(*out_file)) {
				return ENOMEM;
			}

			strcpy(*out_file, argv[i]);
			i++;
		} else {
			*input_file = calloc(MAX_FILENAME_SIZE, sizeof(char));
			if (!(*input_file)) {
				return ENOMEM;
			}

			strcpy(*input_file, argv[i]);
			i++;
			read_input_file = 1;
		}
	}

	return SUCCESS;
}