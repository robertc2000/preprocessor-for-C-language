#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <errno.h>

#include "misc.h"
#define SUCCESS 1

void get_constant_name(char* buffer, char* constant_name);
void get_value_name(char* buffer, char* value_name);
int is_uppercase(char c);
int is_lowercase(char c);
int is_digit(char c);
int is_constant_start(char* value, int i);
int is_char_allowed_in_constant(char* buffer, int index);
char* get_constant(char* buffer, int* alloc_status);
int find_constant_end(char* value, int start_index);
int jump_define(char* buffer);
int starts_with(const char* buffer, const char* target);
char* jump_quotes(char* start, char** end, int* status);
char* get_filename(char* start, int* status);