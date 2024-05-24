#include<bits/stdc++.h>
#include "main.h"

void print_prompt(){
    std::cout<<"db > ";
}

int main(int argc,char* argv[]){
    if(argc < 2){
        std::cout<<"Must supply a database filename. \n";
        exit(EXIT_FAILURE);
    }

    char* filename = argv[1];
    Database db(filename);

    while(true){
        print_prompt();
        std::string sql;
        std::getline(std::cin, sql);

        if(sql == ".exit"){
            std::cout<<"Bye Bye\n";
            break;
        }
        if(sql == "clear"){
            system("clear");
            continue;
        }

        if(sql.substr(0, 12) == "CREATE TABLE"){
            db.CreateTable(sql);
        }
        if(sql.substr(0, 11) == "INSERT INTO"){
            db.InsertIntoTable(sql);
        }
        if(sql.substr(0, 6) == "SELECT"){
            db.SelectFromTable(sql);
        }
    }

}