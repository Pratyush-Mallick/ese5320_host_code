#include "main.h"

void dedup(std::vector<hashtable_t> &hashTable) {
    for(int i=0; i<hashTable.size(); i++) {
        for(int j=i+1; j<hashTable.size(); j++) {
            if(memcmp(&hashTable[i].hashval, &hashTable[j].hashval, SHA256_BLOCK_SIZE) == 0) {
                hashTable[j].id = hashTable[i].id;
                hashTable[j].seen = 1;
            }
        }
    }
}
