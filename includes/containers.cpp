#include <bits/stdc++.h>
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

    if(strcmp(input_buffer.buffer,".exit") == 0){
        close_input_buffer(input_buffer);
        free_table(table);
        exit(EXIT_SUCCESS);
    }
    else if(strcmp(input_buffer.buffer,".clear") == 0){
        system("clear");
        return META_COMMAND_SUCCESS;
    }
    else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer& input_buffer, Statement& statement){
    if(strncmp(input_buffer.buffer, "insert", 6) == 0){
        statement.type = STATEMENT_INSERT;
        int args_assigned = sscanf(input_buffer.buffer, "insert %d %s %s", &statement.row_to_insert.id, statement.row_to_insert.username, statement.row_to_insert.email);

        // std::cout<<statement.row_to_insert.id<<std::endl;
        // std::cout<<statement.row_to_insert.username<<std::endl;
        // std::cout<<statement.row_to_insert.email<<std::endl;


        if(args_assigned < 3){
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
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

void* row_slot(Table& table, uint32_t row_num){
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table.pages[page_num];

    if(page == NULL){
        page = table.pages[page_num] = malloc(PAGE_SIZE);
    }

    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    // std::cout<<page<<std::endl;
    // std::cout<<byte_offset<<std::endl;
    return page + byte_offset;
}

void print_row(Row& row){
    std::cout<<row.id<<" "<<row.username<<" "<<row.email<<std::endl;
}

ExecuteResult execute_insert(Statement& statement, Table& table){
    if(table.num_rows >= TABLE_MAX_ROWS){
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &statement.row_to_insert;

    serialize_row(*row_to_insert, row_slot(table,table.num_rows));
    table.num_rows++;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement& statement, Table& table){
    Row row;
    for(uint32_t i = 0; i < table.num_rows; i ++){
        deserialize_row(row_slot(table, i), row);
        print_row(row);
    }
    
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

Table* new_table(){
    Table* table = new Table;

    table->num_rows = 0;
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i ++){
        table->pages[i] = nullptr;
    }

    return table;
}

void free_table(Table& table){
    for(uint32_t i = 0; i < table.num_rows; i ++){
        free(table.pages[i]);
    }

    free(&table);
}