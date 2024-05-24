#include<bits/stdc++.h>
#include<regex>
#include "main.h"

Database::Database(const std::string &filename){
    file_name = filename;
    file_st = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
    // std::cout<<file_st.is_open()<<std::endl;
    LoadMetaData();
}

Database::~Database(){
    SaveMetaData();
    for(auto& table_data : tables){
        // std::cout<<table_data.first<<std::endl;
        SaveTableData(table_data.second);
    }
    file_st.close();
}

void Database::PrintTable(std::string &table_name){
    auto it = tables.find(table_name);
    if(it == tables.end()){
        std::cout<<"Table does not exist\n";
        return;
    }

    Table& table = it->second;
    std::cout<<"Table: "<<table.name<<std::endl;
    for(auto& column : table.columns){
        std::cout<<column.name<<"\t";
    }
    std::cout<<std::endl;

    for(auto& row : table.rows){
        for(auto& val : row){
            std::cout<<val<<"\t";
        }
        std::cout<<std::endl;
    }
}

bool Database::CreateTable(std::string& sql){
    Table table;

    if(!ParseSQLcreateQuery(sql, table)){
        std::cout<<"Problem in the sql query\n";
        return false;
    }

    if(tables.find(table.name) != tables.end()){
        std::cout<<"Table already exists\n";
        return false;
    }

    size_t start_page = 1;
    // for(const auto& table_data : tables){
    //     start_page += table_data.second.num_pages;
    // }

    table.start_page = next_free_page_;
    table.last_page = next_free_page_;
    next_free_page_ ++;
    table.num_pages = 0;
    table.row_size = 0;
    for(const auto& column : table.columns){
        table.row_size += column.size;
    }
    table.rows.clear();

    tables[table.name] = table;

    return true;
}

bool Database::ParseSQLcreateQuery(std::string& sql, Table& table){

    std::regex reg(R"(CREATE TABLE (\w+) \((.+)\);)");
    std::smatch match;

    if(!std::regex_match(sql, match, reg)){
        return false;
    }

    table.name = match[1];

    std::string column_stream = match[2];
    std::istringstream column_ss(column_stream);
    std::string column_def;

    while(std::getline(column_ss, column_def, ',')){
        std::istringstream column_def_ss(column_def);
        Column column;
        column_def_ss >> column.name >> column.type;

        // std::cout<<column.type<<std::endl;
        if(column.type == "INT"){
            column.size = sizeof(int);
        }
        else if(column.type == "CHAR"){
            // might fuck you because saving done is only of column type and the size is not written anywhere
            column_def_ss >> column.size;
        }
        else{
            std::cout<<"Error in the column type\n";
            return false;
        }

        table.columns.push_back(column);
    }

    return true;
}

bool Database::InsertIntoTable(std::string sql){
    std::string table_name;
    std::vector<std::string> values;

    if(!ParseInsertSQL(sql, table_name, values)){
        std::cout<<"Cannot parse the insert query\n";
        return false;
    }

    auto it = tables.find(table_name);
    if(it == tables.end()){
        std::cout<<"Table "<<table_name<<" does not exist mf"<<std::endl;
        return false;
    }

    Table& table = tables[table_name];

    if(values.size() != table.columns.size()){
        std::cout<<"Incorrect number of columns given"<<std::endl;
        return false;
    }

    size_t rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / table.row_size;

    if(!table.rows.empty() && table.rows.size() % rows_per_page == 0){
        table.last_page = next_free_page_;
        next_free_page_ ++;
        table.num_pages ++;
    }

    table.rows.push_back(values);
    SaveTableData(table);

    return true;
}

bool Database::ParseInsertSQL(std::string sql, std::string& table_name, std::vector<std::string>& values){
    std::regex reg(R"(INSERT INTO (\w+) \((.+)\) VALUES \((.+)\);)");
    std::smatch match;

    if(!std::regex_match(sql, match, reg)){
        return false;
    }

    table_name = match[1];

    std::string column_str = match[2];
    std::string values_str = match[3];

    std::istringstream column_ss(column_str);
    std::istringstream values_ss(values_str);

    std::string column_name;
    std::string value;

    while(std::getline(column_ss, column_name, ',')){
        std::getline(values_ss, value, ',');
        values.push_back(value);
    }

    // for(auto x:values){
    //     std::cout<<x<<std::endl;
    // }

    return true;
}

bool Database::SelectFromTable(std::string sql){
    
    std::string table_name;
    std::vector<std::string> columns;

    if(!ParseSelectSQL(sql, table_name, columns)){
        std::cout<<"Cannot parse the select query\n";
        return false;
    }

    Table& table = tables[table_name];
    std::vector<size_t> column_indices;

    for(auto col : columns){
        bool found = false;
        for(size_t i = 0; i < table.columns.size(); i ++){
            if(table.columns[i].name == col){
                column_indices.push_back(i);
                found = true;
                break;
            }
        }
        if(!found){
            std::cout<<"Column "<<col<<" does not exist in the table "<<table_name<<std::endl;
            return false;
        }
    }

    std::cout<<"Table: "<<table_name<<std::endl;
    for(auto col : columns){
        std::cout<<col<<"\t";
    }
    std::cout<<std::endl;

    for(auto row : table.rows){
        for(auto idx : column_indices){
            std::cout<<row[idx]<<"\t";
        }
        std::cout<<std::endl;
    }

    return true;
}


bool Database::ParseSelectSQL(std::string sql, std::string& table_name, std::vector<std::string>& columns){
    
    std::regex reg(R"(SELECT (.+) FROM (\w+);)");
    std::smatch match;

    if(!std::regex_match(sql, match, reg)){
        return false;
    }

    std::string column_str = match[1];
    table_name = match[2];

    std::istringstream column_ss(column_str);
    std::string column_name;

    while(std::getline(column_ss, column_name, ',')){
        columns.push_back(column_name);
    }

    return true;
}
void Database::LoadMetaData(){
    file_st.seekg(0, std::ios::beg);

    char buffer[PAGE_SIZE] = {0};
    file_st.read(buffer, PAGE_SIZE);

    size_t offset = 0;
    size_t num_tables = 0;

    std::memcpy(&num_tables, buffer + offset, sizeof(size_t));
    offset += sizeof(size_t);

    // std::cout<<num_tables<<std::endl;

    for(size_t i = 0; i < num_tables; i ++){
        Table table;

        size_t table_name_len = 0;
        std::memcpy(&table_name_len, buffer + offset, sizeof(size_t));
        offset += sizeof(size_t);

        table.name = std::string(buffer + offset, table_name_len);
        offset += table_name_len;

        std::memcpy(&table.start_page, buffer + offset, sizeof(size_t));
        offset += sizeof(size_t);

        std::memcpy(&table.num_pages, buffer + offset, sizeof(size_t));
        offset += sizeof(size_t);

        std::memcpy(&table.row_size, buffer + offset, sizeof(size_t));
        offset += sizeof(size_t);

        size_t num_columns = 0;
        std::memcpy(&num_columns, buffer + offset, sizeof(size_t));
        offset += sizeof(size_t);

        for(size_t j = 0; j < num_columns; j ++){
            Column column;

            size_t column_name_len = 0;
            std:memcpy(&column_name_len, buffer + offset, sizeof(size_t));
            offset += sizeof(size_t);

            column.name = std::string(buffer + offset, column_name_len);
            offset += column_name_len;

            size_t column_type_len = 0;
            std::memcpy(&column_type_len, buffer + offset, sizeof(size_t));
            offset += sizeof(size_t);

            column.type = std::string(buffer + offset, column_type_len);
            offset += column_type_len;

            std::memcpy(&column.size, buffer + offset, sizeof(size_t));
            offset += sizeof(size_t);

            table.columns.push_back(column);
        }

        tables[table.name] = table;
        //table me rows bhr
        LoadTableData(table);
    }

    std::memcpy(&next_free_page_, buffer, sizeof(size_t));
    offset += sizeof(size_t);
}

void Database::LoadTableData(Table& table){
    size_t current_page = table.start_page;
    size_t rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / table.row_size;
    table.rows.clear();

    while(current_page != 0){
        file_st.seekg(current_page * PAGE_SIZE, std::ios::beg);
        char buffer[PAGE_SIZE] = {0};
        file_st.read(buffer, PAGE_SIZE);
        size_t offset = sizeof(PageHeader);

        PageHeader header;
        std::memcpy(&header, buffer, sizeof(PageHeader));
        current_page = header.next_page;

        for(size_t rowidx = 0; rowidx < rows_per_page; rowidx ++){
            if(offset + table.row_size > PAGE_SIZE){
                break;
            }

            std::vector<std::string> row;
            for(auto &column : table.columns){
                if(column.type == "INT"){
                    int value;
                    std::memcpy(&value, buffer + offset, sizeof(int));
                    row.push_back(std::to_string(value));
                }
                else{
                    row.push_back(std::string(buffer + offset, column.size));
                }
                offset += column.size;
            }

            table.rows.push_back(row);
        }
    }
}

void Database::SaveMetaData(){
    file_st.seekp(0, std::ios::beg);
    char buffer[PAGE_SIZE] = {0};
    size_t offset = 0;
    size_t num_tables = tables.size();
    std::memcpy(buffer + offset, &num_tables, sizeof(size_t));

    offset += sizeof(size_t);

    for(const auto& table_data : tables){
        const Table& table = table_data.second;

        // std::cout<<table.name<<" "<<table.start_page<<" "<<table.num_pages<<" "<<table.row_size<<std::endl;
        size_t table_name_len = table.name.size();
        std::memcpy(buffer + offset, &table_name_len, sizeof(size_t));
        offset += sizeof(size_t);

        // might go wrong dont know c_str wala cheez (copilot suggested dunno why)
        std::memcpy(buffer + offset, table.name.data(), table_name_len);
        offset += table_name_len;

        std::memcpy(buffer + offset, &table.start_page, sizeof(size_t));
        offset += sizeof(size_t);

        std::memcpy(buffer + offset, &table.num_pages, sizeof(size_t));
        offset += sizeof(size_t);

        std::memcpy(buffer + offset, &table.row_size, sizeof(size_t));
        offset += sizeof(size_t);

        size_t num_columns = table.columns.size();
        std::memcpy(buffer + offset, &num_columns, sizeof(size_t));
        offset += sizeof(size_t);

        for(auto& column : table.columns){
            size_t column_name_len = column.name.size();
            std::memcpy(buffer + offset, &column_name_len, sizeof(size_t));
            offset += sizeof(size_t);

            std::memcpy(buffer + offset, column.name.data(), column_name_len);
            offset += column_name_len;

            size_t column_type_len = column.type.size();
            std::memcpy(buffer + offset, &column_type_len, sizeof(size_t));
            offset += sizeof(size_t);

            std::memcpy(buffer + offset, column.type.data(), column_type_len);
            offset += column_type_len;

            std::memcpy(buffer + offset, &column.size, sizeof(size_t));
            offset += sizeof(size_t);
        }
    }

    std::memcpy(buffer, &next_free_page_, sizeof(size_t));
    offset += sizeof(size_t);

    file_st = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

    file_st.write(buffer, PAGE_SIZE);

    if (!file_st) {
        std::cerr << "Write operation failed." << std::endl;
    }
}


void Database::SaveTableData(Table& table){
    size_t current_page = table.start_page;
    size_t rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / table.row_size;
    // size_t num_pages = (table.rows.size() + rows_per_page - 1) / rows_per_page;
    size_t current_row_idx = 0;
    size_t total_rows = table.rows.size();

    while(current_row_idx < total_rows){
        file_st.seekp(current_page * PAGE_SIZE, std::ios::beg);
        char buffer[PAGE_SIZE] = {0};
        size_t offset = sizeof(PageHeader); 

        size_t end_row = std::min(current_row_idx + rows_per_page, table.rows.size());

        for(size_t rowidx = current_row_idx; rowidx < end_row; rowidx ++){
            const auto& row = table.rows[rowidx];

            for(size_t colidx = 0; colidx < row.size(); colidx ++){
                const auto& column = table.columns[colidx];
                const std::string& row_val = row[colidx];

                if(column.type == "INT"){
                    int value = std::stoi(row_val);
                    std::memcpy(buffer + offset, &value, column.size);
                }
                else{
                    std::memcpy(buffer + offset, row_val.data(), column.size);
                }

                offset += column.size;
            }
        }

        PageHeader header;

        if(end_row < total_rows){
            header.next_page = next_free_page_;
            next_free_page_ ++;
        }
        else{
            header.next_page = 0;
        }
        std::memcpy(buffer, &header, sizeof(PageHeader));

        file_st = std::fstream(file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

        file_st.write(buffer, PAGE_SIZE);

        if(!file_st){
            std::cerr << "Write operation failed." << std::endl;
        }
        current_page = header.next_page;
        current_row_idx = end_row;
    }
}