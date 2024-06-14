#include<bits/stdc++.h>

using namespace std;



int main(){
    const char *file = "data.txt";
    fstream file_st = fstream(file, ios::in | ios::out | ios::binary);
    cout<<file_st.is_open()<<endl;

    size_t v = 10;
    size_t v1 = 200;
    char buffer[100] = {1};
    size_t offset = 0;

    file_st.seekg(offset, ios::beg);


    memcpy(buffer, &v, sizeof(size_t));
    offset += sizeof(size_t);
    memcpy(buffer + offset, &v1, sizeof(size_t));

    file_st.write(buffer, sizeof(size_t) * 2);

    // read
    offset = 0;
    char buffer2[100] = {0};
    file_st.seekg(offset, ios::beg);
    file_st.read(buffer2, sizeof(size_t)*2);
    size_t v2;
    memcpy(&v2, buffer2 + offset, sizeof(size_t));
    offset += sizeof(size_t);
    size_t v3;
    memcpy(&v3, buffer2 + offset, sizeof(size_t));
    // memcpy(&v3, buffer2 + sizeof(size_t), sizeof(size_t));
    cout<<v2<<endl;
    cout<<v3<<endl;

}