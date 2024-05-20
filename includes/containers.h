#include <bits/stdc++.h>

typedef struct{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

void print_prompt();

InputBuffer* new_input_buffer();

void close_input_buffer(InputBuffer*);

void read_input(InputBuffer*);