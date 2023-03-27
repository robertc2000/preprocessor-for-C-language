#include "input.h"

#define HASHMAP_SIZE 30

struct hashmap* map;
char* input_file = NULL;
char* out_file = NULL;
char* out_buffer = NULL;

void get_file_path(char* filename, int* status) {
	char* end;

	path = calloc(MAX_FILENAME_SIZE, sizeof(char));
	if (!path) {
		*status = ENOMEM;
		return;
	}

	if (!filename) {
		return;
	}

	end = filename + strlen(filename) - 1;

	while (*end != '\\' && *end != '/') {
		end--;

		if (end == filename) {
			return;
		}
	}

	strncpy(path, filename, end - filename + 1);
	*status = SUCCESS;
}

void free_everything() {
	if (out_file) {
		free(out_file);
	}

	free(out_buffer);

	if (path) {
		free(path);
	}
	
	free_hashmap(map, free_pair);
	free_list(&directories, free);

	if (input_file) {
		free(input_file);
	}
}

int main(int argc, char** argv) {
	int status = 0, result;

	path = NULL;
	directories = NULL;

	status = init_hashmap(&map, HASHMAP_SIZE);
	if (status == ENOMEM) {
		exit(ENOMEM);
	}

	result = parse_input(argv, argc, map, &directories, &input_file, &out_file, &status);
	if (result == ERROR) {
		printf("Error parsing input\n");
		free_everything();
		exit(1);
	} else if (result == ENOMEM) {
		free_everything();
		exit(ENOMEM);
	}

	get_file_path(input_file, &status);
	if (status == ENOMEM) {
		free_everything();
		exit(ENOMEM);
	}

	out_buffer = calloc(MAX_READ_SIZE, sizeof(char));
	if (!out_buffer) {
		free_everything();
		exit(ENOMEM);
	}

	result = process_file(input_file, out_file, map, out_buffer);
	if (result == FILE_ERROR) {
		free_everything();
		exit(1);
	} else if (result == ENOMEM) {
		free_everything();
		exit(ENOMEM);
	}

	if (out_file) {
		FILE* fout = fopen(out_file, "w");
		if (!fout) {
			free_everything();
			return FILE_ERROR;
		}

		fwrite(out_buffer, sizeof(char), strlen(out_buffer), fout);
		fclose(fout);
	} else {
		printf("%s", out_buffer);
	}

	free_everything();
}
