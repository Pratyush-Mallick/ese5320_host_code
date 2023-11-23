#include "main.h"
#include <math.h>
#include <climits>

uint64_t hash_func(unsigned char* input, unsigned int pos)
{
    // put your hash function implementation here
    uint64_t hash = 0;
    for (int i = 0; i < WIN_SIZE; i++)
        hash += input[pos + WIN_SIZE - 1 - i] * pow(PRIME, i + 1);
    return hash;
}

int cdc(std::vector<int>& chunk_indices, unsigned char* buff, unsigned int buff_size)
{
    int counter = 0;
    int local_max = INT_MIN;
    bool flag = 1;
    int diff;
    uint64_t hash = hash_func(buff, WIN_SIZE);
    for (unsigned int i = WIN_SIZE; i < buff_size - WIN_SIZE; i++) {
        counter++;
        if (hash % MODULUS == TARGET) {
            if (flag == 1) {
                diff = i - 0;
                flag = 0;
            }
            else {
                diff = i - chunk_indices.back();
            }
            if (local_max < diff)
                local_max = diff;
            chunk_indices.push_back(i);
        }
        else if (i == buff_size - WIN_SIZE - 1) {
            chunk_indices.push_back(i);
        }
        else if (counter == 4096 - WIN_SIZE) {
            chunk_indices.push_back(i);
            counter = 0;
        }
        hash = (hash * PRIME) - (buff[i] * pow(PRIME, WIN_SIZE + 1)) + (buff[i + WIN_SIZE] * PRIME);
    }

    if (flag == 1) {
        local_max = buff_size;
    }

    //chunk_indices.push_back(buff_size);
    return local_max;
}

//#include "main.h"
//#include <math.h>
//#include <climits>
//
//uint64_t hash_func(unsigned char* input, unsigned int pos)
//{
//    // put your hash function implementation here
//    uint64_t hash = 0;
//    for (int i = 0; i < WIN_SIZE; i++)
//        hash += input[pos + WIN_SIZE - 1 - i] * pow(PRIME, i + 1);
//    return hash;
//}
//
//int cdc(std::vector<int>& chunk_indices, unsigned char* buff, unsigned int buff_size)
//{
//    int local_max = 0;
//    bool flag = 1;
//    int diff;
//    uint64_t hash = hash_func(buff, WIN_SIZE);
//    for (unsigned int i = WIN_SIZE; i < buff_size - WIN_SIZE; i++) {
//        if (hash % MODULUS == TARGET) {
//            if (flag == 1) {
//                diff = i - 0;
//                flag = 0;
//            }
//            else {
//                diff = i - chunk_indices.back();
//            }
//
//            if (local_max < diff)
//                local_max = diff;
//            chunk_indices.push_back(i);
//        }
//        else if (i == buff_size - WIN_SIZE - 1) {
//            chunk_indices.push_back(i);
//        }
//        hash = (hash * PRIME) - (buff[i] * pow(PRIME, WIN_SIZE + 1)) + (buff[i + WIN_SIZE] * PRIME);
//    }
//    if (flag == 1) {
//        local_max = buff_size;
//    }
//
//    chunk_indices.push_back(buff_size);
//    return local_max;
//}
