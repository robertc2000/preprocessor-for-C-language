#define MAX_READ_SIZE 1024
#define FILE_ERROR -1

#define ERROR -1
#define IF_DIR_V 0
#define ELIF_DIR_V 1
#define ELSE_DIR_V 2

#define NO_IFDEF 0
#define IFDEF_V 1
#define IFNDEF_V 2

#define DEFINE "#define "

#define IF "#if "
#define ELIF "#elif "
#define ELSE "#else"
#define ENDIF "#endif"

#define UNDEF "#undef "

#define IFDEF "#ifdef "
#define IFNDEF "#ifndef "

#define INCLUDE "#include "

#define MAX_CODE_SIZE 50
#define MAX_FILENAME_SIZE 50
#define MAX_CONSTANT_NAME 40
#define MAX_VALUE_NAME 40

struct constant_value_pair {
	char* constant;
	char* value;
};

struct constant_value_pair* alloc_const_val_pair(int* status);