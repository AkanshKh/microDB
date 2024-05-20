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

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

void read_input(InputBuffer * input_buffer){
    ssize_t bytes_read = getline(&(input_buffer->buffer),&(input_buffer->buffer_length),stdin);

    if(bytes_read <= 0){
        std::cout<<"Error reading input\n";
        exit(EXIT_FAILURE);
    }


    // Remove the newline character 
    input_buffer->buffer_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
    // std::cout<<input_buffer->buffer<<" "<<input_buffer->buffer_length<<std::endl;
}