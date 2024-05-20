#include <bits/stdc++.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

template <typename Struct, typename AttributeType>
size_t size_of_attribute(AttributeType Struct::* attribute) {
    return sizeof(attribute);
}

const uint32_t ID_SIZE = size_of_attribute(&Row::id);
const uint32_t USERNAME_SIZE = size_of_attribute(&Row::username);
const uint32_t EMAIL_SIZE = size_of_attribute(&Row::email);

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;


typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT 
} StatementType;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL 
} ExecuteResult;

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;



typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;

typedef struct {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
} Table;

void print_prompt();

InputBuffer* new_input_buffer();

void close_input_buffer(InputBuffer&);

void read_input(InputBuffer&);

MetaCommandResult do_meta_ccommand(InputBuffer& , Table&);

PrepareResult prepare_statement(InputBuffer& , Statement&);

void serialize_row(Row& , void*);

void deserialize_row(void* , Row&);

void* row_slot(Table& , uint32_t);

ExecuteResult execute_insert(Statement& , Table&);

ExecuteResult execute_select(Statement& , Table&);

ExecuteResult execute_statement(Statement& , Table&);

Table* new_table();

void free_table(Table&);