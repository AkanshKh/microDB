#include <bits/stdc++.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "containers.h"

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = new InputBuffer;
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt(){
    std::cout<<"db > ";
}

void close_input_buffer(InputBuffer& input_buffer) {
    free(input_buffer.buffer);
    free(&input_buffer);
}

void read_input(InputBuffer& input_buffer){
    ssize_t bytes_read = getline(&(input_buffer.buffer),&(input_buffer.buffer_length),stdin);

    if(bytes_read <= 0){
        std::cout<<"Error reading input\n";
        exit(EXIT_FAILURE);
    }


    // Remove the newline character 
    input_buffer.buffer_length = bytes_read - 1;
    input_buffer.buffer[bytes_read - 1] = 0;

    // std::cout<<input_buffer.buffer<<" "<<input_buffer.buffer_length<<std::endl;
}

MetaCommandResult do_meta_ccommand(InputBuffer& input_buffer, Table& table){

    // std::cout<<input_buffer.buffer<<std::endl;
    // std::cout<<strcmp(input_buffer.buffer,".exit")<<std::endl;

    if(strcmp(input_buffer.buffer , ".exit") == 0){
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else if(strcmp(input_buffer.buffer, ".btree") == 0){
        std::cout<<"Tree :\n";
        print_leaf_node(get_page(*table.pager , 0));
        return META_COMMAND_SUCCESS;
    }
    else if(strcmp(input_buffer.buffer , ".clear") == 0){
        system("clear");
        return META_COMMAND_SUCCESS;
    }
    else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_insert(InputBuffer& input_buffer, Statement& statement){

    char* keyword = strtok(input_buffer.buffer, " ");
    char* id_string = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if(id_string == NULL || username == NULL || email == NULL){
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);

    if(strlen(id_string) > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    if(strlen(id_string) > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    statement.row_to_insert.id = id;
    strcpy(statement.row_to_insert.username, username);
    strcpy(statement.row_to_insert.email, email);

    return PREPARE_SUCCESS;

}

PrepareResult prepare_statement(InputBuffer& input_buffer, Statement& statement){
    if(strncmp(input_buffer.buffer, "insert", 6) == 0){
        statement.type = STATEMENT_INSERT;
        return prepare_insert(input_buffer, statement);
    }
    if(strcmp(input_buffer.buffer, "select") == 0){
        statement.type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void serialize_row(Row& source, void* destination){
    memcpy(destination + ID_OFFSET, &source.id, ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &source.username, USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &source.email, EMAIL_SIZE);
}

void deserialize_row(void* source, Row& destination){
    memcpy(&destination.id, source + ID_OFFSET, ID_SIZE);
    memcpy(&destination.username, source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&destination.email, source + EMAIL_OFFSET, EMAIL_SIZE);
}

void* get_page(Pager& pager, uint32_t page_num){

    if(page_num > TABLE_MAX_PAGES){
        std::cout<<"Tried to fetch page that is out of bounds\n";
        exit(EXIT_FAILURE);
    }

    if(pager.pages[page_num] == nullptr){
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager.file_length;
        
        if(pager.file_length % PAGE_SIZE){
            num_pages++;
        }

        if(page_num <= num_pages){
            lseek(pager.file_descriptor , page_num * PAGE_SIZE , SEEK_SET);
            ssize_t bytes_read = read(pager.file_descriptor , page , PAGE_SIZE);

            if(bytes_read == -1){
                std::cout<<"Error reading the file\n";
                exit(EXIT_FAILURE);
            }
        }

        pager.pages[page_num] = page;

        if(page_num >= pager.num_pages){
            pager.num_pages = page_num + 1;
        }
    }

    return pager.pages[page_num];
}

void* cursor_value(Cursor& cursor){

    uint32_t page_num = cursor.page_num;

    void* page = get_page(*cursor.table->pager , page_num);

    return leaf_node_value(page, cursor.cell_num);
}

void print_row(Row& row){
    std::cout<<row.id<<" "<<row.username<<" "<<row.email<<std::endl;
}

ExecuteResult execute_insert(Statement& statement, Table& table){

    void* node = get_page(*table.pager , table.root_page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);

    if(num_cells >= LEAF_NODE_MAX_CELLS){
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &statement.row_to_insert;

    uint32_t key_to_insert = row_to_insert->id;
    Cursor* cursor = table_find(table, key_to_insert);

    if(cursor->cell_num < num_cells){
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if(key_at_index == key_to_insert){
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    leaf_node_insert(*cursor, key_to_insert, *row_to_insert);

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement& statement, Table& table){

    Cursor* cursor = table_start(table);

    Row row;

    while(!cursor->end_of_table){
        deserialize_row(cursor_value(*cursor), row);
        print_row(row);
        cursor_advance(*cursor);
    }

    free(cursor);
    
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement& statement, Table& table){
    if(statement.type == STATEMENT_INSERT){
        return execute_insert(statement,table);
    }
    if(statement.type == STATEMENT_SELECT){
        return execute_select(statement,table);
    }

    std::cout<<"GAY GAY"<<std::endl;
}

Table* db_open(const char* filename){

    Pager* pager = pager_open(filename);

    Table* table = new Table;
    table->pager = pager;
    table->root_page_num = 0;

    if(pager->num_pages == 0){
        void* root_node = get_page(*pager, 0);
        initialize_leaf_node(root_node);
    }

    return table;
}

Pager* pager_open(const char* filename){

    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);

    if(fd == -1){
        std::cout<<"Unable to locate the file\n";
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);

    Pager* pager = new Pager;
    
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    pager->num_pages = file_length / PAGE_SIZE;
    
    if(file_length % PAGE_SIZE != 0){
        std::cout<<"Corrupt database file\n";
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i ++){
        pager->pages[i] = nullptr;
    }

    return pager;
}

void pager_flush(Pager& pager , uint32_t page_num){
    
    if(pager.pages[page_num] == nullptr){
        std::cout<<"Tried to flush null page\n";
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager.file_descriptor , page_num * PAGE_SIZE , SEEK_SET);
    // std::cout<<"Offset is : "<<offset<<std::endl;

    if(offset == -1){
        std::cout<<"Error seeking the file : "<<errno<<std::endl;
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager.file_descriptor , pager.pages[page_num] , PAGE_SIZE);

    // std::cout<<"Size is : "<<size<<std::endl;

    if(bytes_written == -1){
        std::cout<<"Error writing to the file : "<<errno<<std::endl;
        exit(EXIT_FAILURE);
    }
}

void db_close(Table& table){
    Pager* pager = table.pager;
    // std::cout<<table.num_rows<<std::endl;
    // std::cout<<num_full_pages<<std::endl;

    for(uint32_t i = 0; i < pager->num_pages; i ++){
        if(pager->pages[i] == nullptr){
            continue;
        }
        pager_flush(*pager , i);
        free(pager->pages[i]);  
        pager->pages[i] = nullptr;
    }

    // uint32_t num_additional_rows = table.num_rows % ROWS_PER_PAGE;
    // // std::cout<<num_additional_rows<<std::endl;

    // if(num_additional_rows > 0){
    //     uint32_t page_num = num_full_pages;
    //     if(pager->pages[page_num] != nullptr){
    //         pager_flush(*pager , page_num);
    //         free(pager->pages[page_num]);
    //         pager->pages[page_num] = nullptr;
    //     }
    // }

    int res = close(pager->file_descriptor);

    if(res == -1){
        std::cout<<"Error closing the file\n";
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i ++){
        void* page = pager->pages[i];
        if(page){
            free(page);
            pager->pages[i] = nullptr;
        }
    }

    free(pager);
    free(&table);
}

// void signalHandler(int signum){
//     std::cout<<"Interrupt signal ("<<signum<<") received.\n";
//     exit(signum);
// }

Cursor* table_start(Table& table){

    Cursor* cursor = new Cursor;
    cursor->table = &table;
    cursor->page_num = table.root_page_num;
    cursor->cell_num = 0;

    void* root_node = get_page(*table.pager , table.root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->end_of_table = (num_cells == 0);

    return cursor;
}

Cursor* table_end(Table& table){

    Cursor* cursor = new Cursor;
    cursor->table = &table;
    cursor->page_num = table.root_page_num;

    void* root_node = get_page(*table.pager , table.root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->cell_num = num_cells;
    cursor->end_of_table = true;

    return cursor;
}

void cursor_advance(Cursor& cursor){
    uint32_t page_num =  cursor.page_num;

    void* node = get_page(*cursor.table->pager , page_num);

    cursor.cell_num += 1;
    if(cursor.cell_num >= (*leaf_node_num_cells(node))){
        cursor.end_of_table = true;
    }
}

uint32_t* leaf_node_num_cells(void* node){
    return (uint32_t*)(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

void* leaf_node_cell(void* node, uint32_t cell_num){
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num){
    return (uint32_t*)leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num){
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void* node){
    set_node_type(node, NODE_LEAF);
    *leaf_node_num_cells(node) = 0;
}

void* leaf_node_insert(Cursor& cursor , uint32_t key , Row& value){

    void* node = get_page(*cursor.table->pager , cursor.page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);

    if(num_cells >= LEAF_NODE_MAX_CELLS){
        std::cout<<"Need to implement splitting a leaf node\n";
        exit(EXIT_FAILURE);
    }

    if(cursor.cell_num < num_cells){
        for(uint32_t i = num_cells; i > cursor.cell_num; i--){
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor.cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor.cell_num));

}

void print_leaf_node(void* node){

    uint32_t num_cells = *leaf_node_num_cells(node);
    std::cout<<"leaf (size "<<num_cells<<") : \n";

    for(uint32_t i = 0; i < num_cells; i++){
        uint32_t key = *leaf_node_key(node, i);
        std::cout<<i<<" - "<<key<<std::endl;
    }
}

Cursor* table_find(Table& table , uint32_t key){

    uint32_t root_page_num = table.root_page_num;
    void* root_node = get_page(*table.pager , root_page_num);

    if(get_node_type(root_node) == NODE_LEAF){
        return leaf_node_find(table, root_page_num, key);
    }
    else{
        std::cout<<"Need to implement searching an internal node\n";
        exit(EXIT_FAILURE);
    }
}

NodeType get_node_type(void* node){
    uint32_t value = *((uint32_t*)(node + NODE_TYPE_OFFSET));

    return (NodeType)value;
}

void set_node_type(void* node, NodeType type){
    uint32_t value = type;

    *((uint32_t*)(node + NODE_TYPE_OFFSET)) = value;
}

Cursor* leaf_node_find(Table& table , uint32_t page_num , uint32_t key){

    void* node = get_page(*table.pager , page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    Cursor* cursor = new Cursor;
    cursor->table = &table;
    cursor->page_num = page_num;

    //Binary search

    uint32_t min_index = 0;
    uint32_t one_past_max_index = num_cells;

    while(one_past_max_index != min_index){
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = *leaf_node_key(node, index);

        if(key == key_at_index){
            cursor->cell_num = index;
            return cursor;
        }

        if(key < key_at_index){
            one_past_max_index = index;
        }
        else{
            min_index = index + 1;
        }
    }

    cursor->cell_num = min_index;
    return cursor;
}