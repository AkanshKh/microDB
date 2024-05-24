#ifndef MAIN_H
#define MAIN_H

#define PAGE_SIZE 4096
struct Column{
    std::string name;
    std::string type;
    size_t size;
};

struct Table {
    std::string name;
    std::vector<Column> columns;
    size_t start_page;
    size_t last_page;
    size_t num_pages;
    size_t row_size;
    std::vector<std::vector<std::string>> rows;
};

struct PageHeader {
    size_t next_page;
};


class Database{
    private:

        std::string file_name;
        std::fstream file_st;
        void LoadMetaData();
        void SaveMetaData();
        void LoadTableData(Table& table);
        void SaveTableData(Table& table);
        bool ParseSQLcreateQuery(std::string& sql, Table& table);
        size_t next_free_page_;
        void PrintTable(std::string& table_name);  
        bool ParseInsertSQL(std::string sql, std::string& table_name, std::vector<std::string>& values);
        bool ParseSelectSQL(std::string sql, std::string& table_name, std::vector<std::string>& columns);

    public:
        std::unordered_map<std::string, Table> tables;
        Database(const std::string &filename);
        ~Database();
        bool CreateTable(std::string& sql);
        bool InsertIntoTable(std::string sql);
        bool SelectFromTable(std::string sql);

}; 

#endif // MAIN_H