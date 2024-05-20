#include<bits/stdc++.h>
#include "./includes/containers.h"
using namespace std;

int main(int argc,char* argv[]){

    Table* table = new_table();

    InputBuffer* input_buffer = new_input_buffer();

    while(true){
        print_prompt();
        read_input(*input_buffer);

        if(input_buffer->buffer[0] == '.'){
            MetaCommandResult got = do_meta_ccommand(*input_buffer, *table);

            if(got == META_COMMAND_SUCCESS){
                std::cout<<"Successfully Executed"<<std::endl;
                continue;
            }
            if(got == META_COMMAND_UNRECOGNIZED_COMMAND){
                std::cout<<"Unrecognized Command "<<input_buffer->buffer<<std::endl;
                continue;
            }
        }

        Statement statement;

        PrepareResult prepared = prepare_statement(*input_buffer,statement);

        if(prepared == PREPARE_SYNTAX_ERROR){
            std::cout<<"Cannot parse the query due to syntax errors\n";
            continue;
        }
        if(prepared == PREPARE_UNRECOGNIZED_STATEMENT){
            std::cout<<"Unrecognized Command initials "<<input_buffer->buffer<<std::endl;
            continue;
        }

        ExecuteResult executed = execute_statement(statement, *table);
        
        if(executed == EXECUTE_SUCCESS){
            std::cout<<"SUCCESSSSSSSSSS HEHE YOU GAY\n";
        }
        if(executed == EXECUTE_TABLE_FULL){
            std::cout<<"TABLE IS FULL GAY MAN\n";
        }
    }
}   