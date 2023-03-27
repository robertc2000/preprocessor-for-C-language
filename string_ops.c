#include "string_ops.h"

void get_constant_name(char* buffer, char* constant_name) {
	int index = 0;
	while(buffer[index] != ' ' && buffer[index] != '\n') {
		constant_name[index] = buffer[index];
		index++;
	}
}

void get_value_name(char* buffer, char* value_name) {
	int buffer_index = 0;
	int value_index = 0;

	if (buffer[buffer_index] == '\n') {
		// value for constant does not exist ("#define ABC")
		return;
	}

	buffer_index++;

	while (buffer[buffer_index] != '\n') {
		if (buffer[buffer_index] == '\\' && buffer[buffer_index + 1] == '\n') {
			buffer_index += 2;
		}

		if (buffer[buffer_index] == '\t') {
			buffer_index++;
			continue;
		}

		value_name[value_index] = buffer[buffer_index];
		buffer_index++;
		value_index++;
	}
}

int is_uppercase(char c) {
	if (c >= 'A' && c <= 'Z') {
		return 1;
	}
	return 0;
}

int is_lowercase(char c){
	if (c >= 'a' && c <= 'z') {
		return 1;
	}
	return 0;
}

int is_digit(char c) {
	if (c >= '0' && c <= '9') {
		return 1;
	}
	return 0;
}

int is_constant_start(char* value, int i) {
	// returns if index i is the start of the constant
	char c = value[i];

	if (is_uppercase(c) || is_lowercase(c) || (c == '_')) {
		if (i == 0 || value[i - 1] == ' ') {
			return 1;
		}
	}

	return 0;
}

int is_char_allowed_in_constant(char* buffer, int index) {
	char c = buffer[index];

	if (index == 0) {
		return is_uppercase(c) || (c == '_') || is_lowercase(c);
	}

	return is_uppercase(c) || is_lowercase(c) || is_digit(c) || (c == '_');
}

char* get_constant(char* buffer, int* alloc_status) {
	char* constant = NULL;
	int index = 0;

	if (!is_constant_start(buffer, 0)) {
		return NULL;
	}

	constant = calloc(MAX_CONSTANT_NAME, sizeof(char));
	if (!constant) {
		*alloc_status = ENOMEM;
		return NULL;
	}

	*alloc_status = SUCCESS;

	while (is_char_allowed_in_constant(buffer, index)) {
		constant[index] = buffer[index];
		index++;
	}

	return constant;
}

int find_constant_end(char* value, int start_index) {
	int i = start_index;

	while (is_char_allowed_in_constant(value, i)) {
		i++;
	}

	return i;
}

int jump_define(char* buffer) {
	int index = 0;

	while (buffer[index] != '\n') {
		if (buffer[index] == '\\') {
			index += 2;
		} else {
			index++;
		}
	}

	return index + 1;
}

int starts_with(const char* buffer, const char* target) {
	// returns 1 if buffer starts with target string
	return !strncmp(buffer, target, strlen(target));
}

char* jump_quotes(char* start, char** end, int* status) {
	// start[0] must be '\"'
	// end is a pointer to the first char after the closing quote
	// returns the string inside the quotation marks

	int max_quote_string_size = 40;
	char* quotes = calloc(max_quote_string_size, sizeof(char));
	if (!quotes) {
		*status = ENOMEM;
		return NULL;
	}

	*end = start + 1;
	for (; **end != '\"'; (*end)++);
	(*end)++;

	strncpy(quotes, start, *end - start);

	return quotes;
}

char* get_filename(char* start, int* status) {
	// "start" starts with "#include"
	char* sep = " \n", *tok;
	char* filename = calloc(MAX_FILENAME_SIZE, sizeof(char));
	if (!filename) {
		*status = ENOMEM;
		return NULL;
	}


	tok = strtok(start, sep);
	tok = strtok(NULL, sep);

	strncpy(filename, tok + 1, strlen(tok) - 2);

	return filename;
}