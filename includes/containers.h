#include <bits/stdc++.h>

#define COLUMN_USERNAME_SIZE 31
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

struct Row {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
};


template <typename Struct, typename AttributeType>
const size_t size_of_attribute(AttributeType Struct::* attribute) {
    return sizeof(AttributeType);
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


// Common Node Header Layout

const uint32_t NODE_TYPE_SIZE = sizeof(uint32_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint32_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint32_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;


// Leaf Node Header Layout

const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

// Leaf Node Body Layout

const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;



typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT 
} StatementType;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL ,
    EXECUTE_DUPLICATE_KEY
} ExecuteResult;

typedef enum {
    NODE_INTERNAL,
    NODE_LEAF
} NodeType;

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
    int file_descriptor;
    uint32_t num_pages;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    Pager* pager;
    uint32_t root_page_num;
} Table;

typedef struct {
    Table* table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;
} Cursor;

InputBuffer* new_input_buffer();

void print_prompt();

void close_input_buffer(InputBuffer&);

void read_input(InputBuffer&);

MetaCommandResult do_meta_ccommand(InputBuffer& , Table&);

PrepareResult prepare_insert(InputBuffer& , Statement&);

PrepareResult prepare_statement(InputBuffer& , Statement&);

void serialize_row(Row& , void*);

void deserialize_row(void* , Row&);

void* get_page(Pager& , uint32_t);

void* cursor_value(Cursor&);

void print_row(Row&);

ExecuteResult execute_insert(Statement& , Table&);

ExecuteResult execute_select(Statement& , Table&);

ExecuteResult execute_statement(Statement& , Table&);

Table* db_open(const char* filename);

Pager* pager_open(const char* filename);

void pager_flush(Pager &, uint32_t);

void db_close(Table &);

// void signalHandler();

Cursor* table_start(Table&);

Cursor* table_end(Table&);

void cursor_advance(Cursor&);

uint32_t* leaf_node_num_cells(void*);

void* leaf_node_cell(void* , uint32_t);

uint32_t* leaf_node_key(void* , uint32_t);

void* leaf_node_value(void* , uint32_t);

void initialize_leaf_node(void*);

void* leaf_node_insert(Cursor& , uint32_t , Row&);

void print_leaf_node(void*);

Cursor* table_find(Table& , uint32_t);

NodeType get_node_type(void*);

void set_node_type(void*, NodeType);

Cursor* leaf_node_find(Table& , uint32_t , uint32_t);