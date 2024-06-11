#include <bits/stdc++.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "containers.h"

// InputBuffer* new_input_buffer() {
//     InputBuffer* input_buffer = new InputBuffer;
//     input_buffer->buffer = NULL;
//     input_buffer->buffer_length = 0;
//     input_buffer->input_length = 0;

//     return input_buffer;
// }

void print_prompt(){
    std::cout<<"db > ";
}

MetaCommandResult do_meta_ccommand(const std::string& input_buffer, Table& table){

    if(input_buffer==".exit"){
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else if(input_buffer==".btree"){
        std::cout<<"Tree :\n";
        print_tree(*table.pager, 0, 0);
        return META_COMMAND_SUCCESS;
    }
    else if(input_buffer==".clear"){
        system("clear");
        return META_COMMAND_SUCCESS;
    }
    else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_insert(const std::string& input_buffer, Statement& statement){

    std::istringstream raw_cmd(input_buffer);
    std::string keyword,id_string,username,email;
    raw_cmd>>keyword>>id_string>>username>>email;



    if(id_string == "" || username == "" || email == ""){
        return PREPARE_SYNTAX_ERROR;
    }

    int id = stoi(id_string);

    if(username.size() > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    if(email.size() > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }

    statement.row_to_insert.id = id;
    strcpy(statement.row_to_insert.username, username.c_str());
    strcpy(statement.row_to_insert.email, email.c_str());

    return PREPARE_SUCCESS;

}

PrepareResult prepare_statement(const std::string& input_buffer, Statement& statement){
    if(input_buffer.substr(0,15)=="create database"){
        //creates new database file, do later
    }
    if(input_buffer.substr(0,12)=="create table"){
        //create new table
        statement.type =STATEMENT_CREATE;
        // return prepare_create(input_buffer,statement);
    }
    if(input_buffer.substr(0,6)=="insert"){
        statement.type = STATEMENT_INSERT;
        return prepare_insert(input_buffer, statement);
    }
    if(input_buffer.substr(0,6)=="select"){
        statement.type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    if(input_buffer.substr(0,6)=="delete"){
        statement.type = STATEMENT_DELETE;
        // return prepare_delete(input_buffer,statement);
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
        void* root_node = get_page(*pager, 0);//
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
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
    Cursor* cursor = table_find(table,0);

    void* node = get_page(*table.pager,cursor->page_num);
    cursor->end_of_table = (*leaf_node_num_cells(node) == 0);

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
        uint32_t next_page_num = *leaf_node_next_leaf(node);
        if(next_page_num == 0){
            cursor.end_of_table = true;
        }
        else{
            cursor.page_num = next_page_num;
            cursor.cell_num = 0;
        }
    }
}

uint32_t* leaf_node_num_cells(void* node){
    return (uint32_t*)(node + LEAF_NODE_NUM_CELLS_OFFSET);
}

uint32_t* leaf_node_next_leaf(void* node){
    return (uint32_t*)(node + LEAF_NODE_NEXT_LEAF_OFFSET);
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
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
    *leaf_node_next_leaf(node) = 0;
}

void leaf_node_insert(Cursor& cursor , uint32_t key , Row& value){

    void* node = get_page(*cursor.table->pager , cursor.page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);

    if(num_cells >= LEAF_NODE_MAX_CELLS){
        leaf_node_split_and_insert(cursor, key, value);
        return;
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

uint32_t* node_parent(void* node){
    return (uint32_t*)(node + PARENT_POINTER_OFFSET);
}

void leaf_node_split_and_insert(Cursor& cursor, uint32_t key, Row& value){
    void* old_node = get_page(*cursor.table->pager, cursor.page_num);
    uint32_t old_max = get_node_max_key(*cursor.table->pager,old_node);
    uint32_t new_page_num = get_unused_page_num(*cursor.table->pager);
    void* new_node = get_page(*cursor.table->pager, new_page_num);

    initialize_leaf_node(new_node);
    *node_parent(new_node) = *node_parent(old_node);
    *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
    *leaf_node_next_leaf(old_node) = new_page_num;

    for(int32_t i= LEAF_NODE_MAX_CELLS; i>=0; i--){
        void* destination_node;

        if(i >= LEAF_NODE_LEFT_SPLIT_COUNT){
            destination_node = new_node;
        }
        else{
            destination_node = old_node;
        }

        uint32_t index_within_node = i%LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destination = leaf_node_cell(destination_node, index_within_node);

        if(i == cursor.cell_num){
            serialize_row(value, leaf_node_value(destination_node, index_within_node));
            *(leaf_node_key(destination_node, index_within_node)) = key;
        }
        else if(i > cursor.cell_num){
            memcpy(destination, leaf_node_cell(old_node, i-1), LEAF_NODE_CELL_SIZE);
        }
        else{
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if(is_node_root(old_node)){
        return create_new_root(*cursor.table, new_page_num);
    }
    else{
        uint32_t parent_page_num = *node_parent(old_node);
        uint32_t new_max = get_node_max_key(*cursor.table->pager,old_node);
        void* parent = get_page(*cursor.table->pager, parent_page_num);

        update_internal_node_key(parent, old_max, new_max);
        internal_node_insert(*cursor.table, parent_page_num, new_page_num);
        return;
    }
}
void update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key){
    uint32_t old_child_index = internal_node_find_child(node, old_key);
    *internal_node_key(node, old_child_index) = new_key;
}

uint32_t get_unused_page_num(Pager& pager){
    return pager.num_pages;
}

void create_new_root(Table& table, uint32_t right_child_page_number){
    void* root = get_page(*table.pager, table.root_page_num);
    void* right_child = get_page(*table.pager, right_child_page_number);

    uint32_t left_child_page_number = get_unused_page_num(*table.pager);
    void* left_child = get_page(*table.pager, left_child_page_number);

    if(get_node_type(root)==NODE_INTERNAL){
        initialize_internal_node(right_child);
        initialize_internal_node(left_child);
    }

    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    if(get_node_type(left_child)==NODE_INTERNAL){
        void* child;
        for(int i=0;i<*internal_node_num_keys(left_child);i++){
            child = get_page(*table.pager, *internal_node_child(left_child, i));
            *node_parent(child) = left_child_page_number;
        }
        child = get_page(*table.pager, *internal_node_right_child(left_child));
        *node_parent(child) = left_child_page_number;
    }

    initialize_internal_node(root);
    set_node_root(root, true);

    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_number;
    uint32_t left_child_max_key = get_node_max_key(*table.pager,left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_number;
    *node_parent(left_child) = table.root_page_num;
    *node_parent(right_child) = table.root_page_num;

}

uint32_t* internal_node_num_keys(void* node){
    return (uint32_t*)(node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* internal_node_right_child(void* node){
    return (uint32_t*)(node+INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num) {
  return (uint32_t*)(node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* internal_node_child(void* node, uint32_t child_num){
    uint32_t num_keys = *internal_node_num_keys(node);
    if(child_num > num_keys){
        std::cout<<"1\n";
        std::cout<<"Tried to access child_num > num_keys\n";
        exit(EXIT_FAILURE);
    }
    else if(child_num == num_keys){
        // return internal_node_right_child(node);
        uint32_t* right_child = internal_node_right_child(node);
        if(*right_child == INVALID_PAGE_NUM){
            std::cout<<"2\n";
            std::cout<<"Tried to access child_num > num_keys\n";//throwing error now
            exit(EXIT_FAILURE);
        }
        return right_child;
    }
    else{
        uint32_t* child = internal_node_cell(node, child_num);
        if(*child==INVALID_PAGE_NUM){
            std::cout<<"3\n";
            std::cout<<"Tried to access child_num > num_keys\n";
            exit(EXIT_FAILURE);
        }
        return child;
    }
}

uint32_t* internal_node_key(void* node, uint32_t key_num){
    return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

// uint32_t get_node_max_key(void* node){
//     switch(get_node_type(node)){
//         case NODE_INTERNAL:
//             return *internal_node_key(node, *internal_node_num_keys(node) - 1);
//         case NODE_LEAF:
//             return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
//     }
// }
uint32_t get_node_max_key(Pager& pager,void* node){
    if(get_node_type(node)==NODE_LEAF){
        return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
    void* right_child = get_page(pager, *internal_node_right_child(node));
    return get_node_max_key(pager, right_child);
}

bool is_node_root(void* node){
    uint8_t value = *((uint8_t*)(node + IS_ROOT_OFFSET));
    return (bool)value;
}

void set_node_root(void* node, bool is_root){
    uint8_t value = is_root;
    *((uint8_t*)(node + IS_ROOT_OFFSET)) = value;
}

void initialize_internal_node(void* node){
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
    *internal_node_right_child(node) = INVALID_PAGE_NUM;
}

void indent(uint32_t level){
  for (uint32_t i = 0; i < level; i++) {
    printf("  ");
  }
}

void print_tree(Pager& pager, uint32_t page_num, uint32_t indentation_level){
  void* node = get_page(pager, page_num);
  uint32_t num_keys, child;

  switch (get_node_type(node)) {
    case (NODE_LEAF):
      num_keys = *leaf_node_num_cells(node);
      indent(indentation_level);
      printf("- leaf (size %d)\n", num_keys);
      for (uint32_t i = 0; i < num_keys; i++) {
        indent(indentation_level + 1);
        printf("- %d\n", *leaf_node_key(node, i));
      }
      break;
    case (NODE_INTERNAL):
        num_keys = *internal_node_num_keys(node);
        indent(indentation_level);
        printf("- internal (size %d)\n", num_keys);
        
        if(num_keys >0){
            for (uint32_t i = 0; i < num_keys; i++) {
                child = *internal_node_child(node, i);
                print_tree(pager, child, indentation_level + 1);

                indent(indentation_level + 1);
                printf("- key %d\n", *internal_node_key(node, i));
            }
            child = *internal_node_right_child(node);
            print_tree(pager, child, indentation_level + 1);

        }
        break;
  }
}

Cursor* table_find(Table& table , uint32_t key){

    uint32_t root_page_num = table.root_page_num;
    void* root_node = get_page(*table.pager , root_page_num);

    if(get_node_type(root_node) == NODE_LEAF){
        return leaf_node_find(table, root_page_num, key);
    }
    else{
        return internal_node_find(table, root_page_num, key);
    }
}

Cursor* internal_node_find(Table& table, uint32_t page_num,uint32_t key){
    void* node = get_page(*table.pager, page_num);
    uint32_t child_index = internal_node_find_child(node, key);

    uint32_t child_num = *internal_node_child(node, child_index);
    void* child = get_page(*table.pager, child_num);
    switch(get_node_type(child)){
        case NODE_LEAF:
            return leaf_node_find(table, child_num, key);
        case NODE_INTERNAL:
            return internal_node_find(table, child_num, key);
    }
}

uint32_t internal_node_find_child(void* node,uint32_t key){
    uint32_t num_keys = *internal_node_num_keys(node);

    uint32_t min_index = 0;
    uint32_t max_index = num_keys;

    while(min_index != max_index){
        uint32_t index = (min_index + max_index) / 2;
        uint32_t key_to_right = *internal_node_key(node, index);

        if(key_to_right >= key){
            max_index = index;
        }
        else{
            min_index = index + 1;
        }
    }

    return min_index;
}

void internal_node_split_and_insert(Table& table, uint32_t parent_page_num, uint32_t child_page_num){
    uint32_t old_page_num = parent_page_num;
    void* old_node = get_page(*table.pager, parent_page_num);
    uint32_t old_max = get_node_max_key(*table.pager,old_node);

    void* child = get_page(*table.pager, child_page_num);
    uint32_t child_max= get_node_max_key(*table.pager,child);
    uint32_t new_page_num = get_unused_page_num(*table.pager);

    uint32_t splitting_root = is_node_root(old_node);
    void* parent;
    void* new_node;

    if(splitting_root){
        create_new_root(table,new_page_num);
        parent = get_page(*table.pager, table.root_page_num);
        old_page_num = *internal_node_child(parent, 0);
        old_node = get_page(*table.pager, old_page_num);
    }
    else{
        parent = get_page(*table.pager, *node_parent(old_node));
        new_node = get_page(*table.pager, new_page_num);
        initialize_internal_node(new_node);
    }

    uint32_t* old_num_keys = internal_node_num_keys(old_node);
    uint32_t cur_page_num = *internal_node_right_child(old_node);
    void* cur = get_page(*table.pager, cur_page_num);

    internal_node_insert(table,new_page_num,cur_page_num);
    *node_parent(cur) = new_page_num;
    *internal_node_right_child(old_node) = INVALID_PAGE_NUM;

    for(int i=INTERNAL_NODE_MAX_CELLS-1; i> INTERNAL_NODE_MAX_CELLS/2; i--){
        cur_page_num = *internal_node_child(old_node, i);
        cur = get_page(*table.pager, cur_page_num);
        internal_node_insert(table,new_page_num,cur_page_num);
        *node_parent(cur) = new_page_num;
        (*old_num_keys)--;
    }

    *internal_node_right_child(old_node) = *internal_node_child(old_node,*old_num_keys-1);
    (*old_num_keys)--;


    uint32_t max_after_split = get_node_max_key(*table.pager,old_node);
    uint32_t destination_page_num =child_max < max_after_split ? old_page_num : new_page_num;

    internal_node_insert(table, destination_page_num, child_page_num);
    *node_parent(child) = destination_page_num;

    update_internal_node_key(parent, old_max, get_node_max_key(*table.pager,old_node));

    if(!splitting_root){
        internal_node_insert(table, *node_parent(old_node), new_page_num);
        *node_parent(new_node) = *node_parent(old_node);
    }

}

void internal_node_insert(Table& table, uint32_t parent_page_num, uint32_t child_page_num){
    void* parent = get_page(*table.pager, parent_page_num);
    void* child = get_page(*table.pager, child_page_num);

    uint32_t child_max = get_node_max_key(*table.pager,child);
    uint32_t index = internal_node_find_child(parent, child_max);

    uint32_t original_num_keys = *internal_node_num_keys(parent);

    if(original_num_keys >= INTERNAL_NODE_MAX_CELLS){
        internal_node_split_and_insert(table, parent_page_num, child_page_num);
        return;
    }

    uint32_t right_child_page_num = *internal_node_right_child(parent);

    if(right_child_page_num ==INVALID_PAGE_NUM){
        *internal_node_right_child(parent) = child_page_num;
        return;
    }

    void* right_child = get_page(*table.pager, right_child_page_num);
    *internal_node_num_keys(parent) = original_num_keys + 1;

    if(child_max > get_node_max_key(*table.pager,right_child)){
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) = get_node_max_key(*table.pager,right_child);
        *internal_node_right_child(parent) = child_page_num;
    }
    else{
        for(uint32_t i = original_num_keys; i > index; i--){
            void* destination = internal_node_cell(parent, i);
            void* source = internal_node_cell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }

        *internal_node_child(parent, index) = child_page_num;
        *internal_node_key(parent, index) = child_max;
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