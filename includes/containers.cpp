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

    if(strcmp(input_buffer.buffer,".exit") == 0){
        close_input_buffer(input_buffer);
        db_close(table);
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
    }

    return pager.pages[page_num];
}

void* row_slot(Table& table, uint32_t row_num){
    uint32_t page_num = row_num / ROWS_PER_PAGE;

    void* page = get_page(*table.pager, page_num);

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
    // std::cout<<table.num_rows<<std::endl;
    table.num_rows++;
    // std::cout<<table.num_rows<<std::endl;

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

Table* db_open(const char* filename){

    Pager* pager = pager_open(filename);
    uint32_t num_rows = (pager->file_length) / ROW_SIZE;

    Table* table = new Table;
    table->pager = pager;
    table->num_rows = num_rows;

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

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i ++){
        pager->pages[i] = nullptr;
    }

    return pager;
}

void pager_flush(Pager& pager , uint32_t page_num , uint32_t size){
    
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

    ssize_t bytes_written = write(pager.file_descriptor , pager.pages[page_num] , size);

    // std::cout<<"Size is : "<<size<<std::endl;

    if(bytes_written == -1){
        std::cout<<"Error writing to the file : "<<errno<<std::endl;
        exit(EXIT_FAILURE);
    }
}

void db_close(Table& table){
    Pager* pager = table.pager;
    uint32_t num_full_pages = table.num_rows / ROWS_PER_PAGE;
    // std::cout<<table.num_rows<<std::endl;
    // std::cout<<num_full_pages<<std::endl;

    for(uint32_t i = 0; i < num_full_pages; i ++){
        if(pager->pages[i] == nullptr){
            continue;
        }
        pager_flush(*pager , i , PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = nullptr;
    }

    uint32_t num_additional_rows = table.num_rows % ROWS_PER_PAGE;
    // std::cout<<num_additional_rows<<std::endl;

    if(num_additional_rows > 0){
        uint32_t page_num = num_full_pages;
        if(pager->pages[page_num] != nullptr){
            pager_flush(*pager , page_num , num_additional_rows * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = nullptr;
        }
    }

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