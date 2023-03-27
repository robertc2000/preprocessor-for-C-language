#include "hashmap.h"
#include "string_ops.h"

char* path;
struct list* directories;

int is_define(const char* buffer);
int is_if(const char* buffer);
int is_undef(const char* buffer);
int is_include(const char* buffer);
int is_ifdef(const char* buffer);

char* get_constant_value(struct hashmap* map, char* constant);
void free_pair(void* pair);

int hash_function(void* pair);
char* read_input(char* in_file, int* alloc_status);

int parse_define(struct hashmap* map, char* start, char** end);
void parse_undefine(struct hashmap* map, char* start, char** end);
char* parse_if(struct hashmap* map, char* start, char** end, int* alloc_status);
char* parse_include(struct hashmap* map, struct list* dirs, char* start, char** end, int* status);
char* process_ifdef(struct hashmap* map, char* start, char** end, int* alloc_status);
void parse(char** current_index_in, char* out_buffer, int* current_index_out,
					int* alloc_status, struct hashmap* map);
int process_file(char* in_file, char* out_file, struct hashmap* map, char* out_buffer);