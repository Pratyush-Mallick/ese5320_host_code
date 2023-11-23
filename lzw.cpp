#include "main.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
//****************************************************************************************************************
//#define CAPACITY 4096
// try  uncommenting the line above and commenting line 6 to make the hash table smaller
// and see what happens to the number of entries in the assoc mem
// (make sure to also comment line 27 and uncomment line 28)
#define SEED 524057
#define ASSOC_MEM_STORE 64
// #define CLOSEST_PRIME 65497
// #define CLOSEST_PRIME 32749
// #define CLOSEST_PRIME 65497
#define FILE_SIZE 4096

static inline uint32_t murmur_32_scramble(uint32_t k) {
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

unsigned int my_hash(unsigned long key) {
	uint32_t h = SEED;
	uint32_t k = key;
	h ^= murmur_32_scramble(k);
	h = (h << 13) | (h >> 19);
	h = h * 5 + 0xe6546b64;

	h ^= murmur_32_scramble(k);
	/* Finalize. */
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	//return h & 0xFFFF;
	return h & (CAPACITY - 1);
	// return key % CLOSEST_PRIME;
}
//void hash_lookup(unsigned long hash_table[][2], unsigned int key, bool* hit, unsigned int* result)
//{
////    std::cout << "hash_lookup():" << std::endl;
//    key &= 0xFFFFF; // make sure key is only 20 bits
//
//    unsigned int hash_val = my_hash(key);
//
//    unsigned long lookup = hash_table[my_hash(key)][0];
//    // [valid][value][key]
//    unsigned int stored_key = lookup&0xFFFFF;       // stored key is 20 bits
//    unsigned int value = (lookup >> 20)&0xFFF;      // value is 12 bits
//    unsigned int valid = (lookup >> (20 + 12))&0x1; // valid is 1 bit
//
//    if(valid && (key == stored_key))
//    {
//        *hit = 1;
//        *result = value;
////        std::cout << "\thit the hash" << std::endl;
//        //std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
//    }
//    else
//    {
//        lookup = hash_table[hash_val][1];
//        stored_key = lookup&0xFFFFF;       // stored key is 20 bits
//        value = (lookup >> 20)&0xFFF;      // value is 12 bits
//        valid = (lookup >> (20 + 12))&0x1; // valid is 1 bit
//
//        if(valid && (key == stored_key))
//        {
//            *hit = 1;
//            *result = value;
//        }
//        else{
////            std::cout << "\tNo hit" << std::endl;
//            *hit = 0;
//            *result = 0;
//        }
////        std::cout << "\tmissed the hash" << std::endl;
//    }
//}

void hash_lookup(unsigned long (*hash_table)[2], unsigned int key, bool *hit,
		unsigned int *result) {
	//std::cout << "hash_lookup():" << std::endl;
	key &= 0xFFFFF; // make sure key is only 20 bits

	unsigned int hash_val = my_hash(key);

	unsigned long lookup = hash_table[hash_val][0];

	// [valid][value][key]
	unsigned int stored_key = lookup & 0xFFFFF;       // stored key is 20 bits
	unsigned int value = (lookup >> 20) & 0xFFF;      // value is 12 bits
	unsigned int valid = (lookup >> (20 + 12)) & 0x1; // valid is 1 bit

	if (valid && (key == stored_key)) {
		*hit = 1;
		*result = value;
		//std::cout << "\thit the hash" << std::endl;
		//std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
	} else {
		lookup = hash_table[hash_val][1];

		// [valid][value][key]
		stored_key = lookup & 0xFFFFF;       // stored key is 20 bits
		value = (lookup >> 20) & 0xFFF;      // value is 12 bits
		valid = (lookup >> (20 + 12)) & 0x1; // valid is 1 bit
		if (valid && (key == stored_key)) {
			*hit = 1;
			*result = value;
		} else {
			*hit = 0;
			*result = 0;
		}
		// std::cout << "\tmissed the hash" << std::endl;
	}
}

//void hash_insert(unsigned long hash_table[][2], unsigned int key, unsigned int value, bool* collision)
//{
////    std::cout << "hash_insert():" << std::endl;
//    key &= 0xFFFFF;   // make sure key is only 20 bits
//    value &= 0xFFF;   // value is only 12 bits
//
//    unsigned int hash_val = my_hash(key);
//
//    unsigned long lookup = hash_table[hash_val][0];
//    unsigned int valid = (lookup >> (20 + 12))&0x1;
//
//    if(valid)
//    {
//        lookup = hash_table[hash_val][1];
//        valid = (lookup >> (20 + 12))&0x1;
//        if(valid){
////            std::cout << "\tCollision" << std::endl;
//            *collision = 1;
//        }
//        else{
//            hash_table[hash_val][1] = (1UL << (20 + 12)) | (value << 20) | key;
//            *collision = 0;
//        }
//        //std::cout << "\tcollision in the hash" << std::endl;
//    }
//    else
//    {
//        hash_table[hash_val][0] = (1UL << (20 + 12)) | (value << 20) | key;
//        *collision = 0;
//        //std::cout << "\tinserted into the hash table" << std::endl;
//        //std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
//    }
//}
void hash_insert(unsigned long (*hash_table)[2], unsigned int key,
		unsigned int value, bool *collision) {
	//std::cout << "hash_insert():" << std::endl;
	key &= 0xFFFFF;   // make sure key is only 20 bits
	value &= 0xFFF;   // value is only 12 bits

	unsigned int hash_val = my_hash(key);

	unsigned long lookup = hash_table[hash_val][0];
	unsigned int valid = (lookup >> (20 + 12)) & 0x1;

	if (valid) {
		lookup = hash_table[hash_val][1];
		valid = (lookup >> (20 + 12)) & 0x1;
		if (valid) {
			*collision = 1;
		} else {
			hash_table[hash_val][1] = (1UL << (20 + 12)) | (value << 20) | key;
			*collision = 0;
		}
		// std::cout << "\tKey is:" << key << std::endl;
		// std::cout << "\tcollision in the hash" << std::endl;
	} else {
		hash_table[hash_val][0] = (1UL << (20 + 12)) | (value << 20) | key;
		*collision = 0;
		//std::cout << "\tinserted into the hash table" << std::endl;
		//std::cout << "\t(k,v,h) = " << key << " " << value << " " << my_hash(key) << std::endl;
	}
}
//****************************************************************************************************************
typedef struct {
	// Each key_mem has a 9 bit address (so capacity = 2^9 = 512)
	// and the key is 20 bits, so we need to use 3 key_mems to cover all the key bits.
	// The output width of each of these memories is 64 bits, so we can only store 64 key
	// value pairs in our associative memory map.

	unsigned long upper_key_mem[512]; // the output of these  will be 64 bits wide (size of unsigned long).
	unsigned long middle_key_mem[512];
	unsigned long lower_key_mem[512];
	unsigned int value[ASSOC_MEM_STORE]; // value store is 64 deep, because the lookup mems are 64 bits wide
	unsigned int fill;       // tells us how many entries we've currently stored
} assoc_mem;

// cast to struct and use ap types to pull out various feilds.

void assoc_insert(assoc_mem *mem, unsigned int key, unsigned int value,
		bool *collision) {
	//std::cout << "assoc_insert():" << std::endl;
	key &= 0xFFFFF; // make sure key is only 20 bits
	value &= 0xFFF;   // value is only 12 bits

	if (mem->fill < ASSOC_MEM_STORE) {
		mem->upper_key_mem[(key >> 18) % 512] |= (1 << mem->fill); // set the fill'th bit to 1, while preserving everything else
		mem->middle_key_mem[(key >> 9) % 512] |= (1 << mem->fill); // set the fill'th bit to 1, while preserving everything else
		mem->lower_key_mem[(key >> 0) % 512] |= (1 << mem->fill); // set the fill'th bit to 1, while preserving everything else
		mem->value[mem->fill] = value;
		mem->fill++;
		*collision = 0;
		//std::cout << "\tinserted into the assoc mem" << std::endl;
		//std::cout << "\t(k,v) = " << key << " " << value << std::endl;
	} else {
		*collision = 1;
	}
}

void assoc_lookup(assoc_mem *mem, unsigned int key, bool *hit,
		unsigned int *result) {
	//std::cout << "assoc_lookup():" << std::endl;
	key &= 0xFFFFF; // make sure key is only 20 bits

	unsigned int match_high = mem->upper_key_mem[(key >> 18) % 512];
	unsigned int match_middle = mem->middle_key_mem[(key >> 9) % 512];
	unsigned int match_low = mem->lower_key_mem[(key >> 0) % 512];

	unsigned int match = match_high & match_middle & match_low;

	unsigned int address = 0;
	for (; address < ASSOC_MEM_STORE; address++) {
		if ((match >> address) & 0x1) {
			break;
		}
	}

	if (address != ASSOC_MEM_STORE) {
		*result = mem->value[address];
		*hit = 1;
		//std::cout << "\thit the assoc" << std::endl;
		//std::cout << "\t(k,v) = " << key << " " << *result << std::endl;
	} else {
		*hit = 0;
		//std::cout << "\tmissed the assoc" << std::endl;
	}
}
//****************************************************************************************************************
void insert(unsigned long hash_table[][2], assoc_mem *mem, unsigned int key,
		unsigned int value, bool *collision) {
	hash_insert(hash_table, key, value, collision);
	if (*collision) {
		assoc_insert(mem, key, value, collision);
	}
}

void lookup(unsigned long hash_table[][2], assoc_mem *mem, unsigned int key,
		bool *hit, unsigned int *result) {
	hash_lookup(hash_table, key, hit, result);
	if (!*hit) {
		assoc_lookup(mem, key, hit, result);
	}
}

void hardware_encoding(int *out_size, int *output_buff, char *s1, int s1_len) {
#pragma HLS INTERFACE m_axi port=out_size depth=1 bundle=axismm1
#pragma HLS INTERFACE m_axi port=output_buff depth=8192 bundle=axismm2
#pragma HLS INTERFACE m_axi port=s1 depth=8192 bundle=axismm1
//#pragma HLS INTERFACE m_axi port=s1_len depth=4 bundle=p0

	int next_code = 256;

	unsigned int prefix_code = s1[0];
	unsigned int code = 0;
	unsigned char next_char = 0;

	int push_index = 0;
	// create hash table and assoc mem
	unsigned long hash_table[CAPACITY][2];
	assoc_mem my_assoc_mem;

	// make sure the memories are clear
	for (int i = 0; i < CAPACITY; i++) {
		for (int j = 0; j < 2; j++) {
			hash_table[i][j] = 0;
		}
	}
	my_assoc_mem.fill = 0;
	for (int i = 0; i < 512; i++) {
		my_assoc_mem.upper_key_mem[i] = 0;
		my_assoc_mem.middle_key_mem[i] = 0;
		my_assoc_mem.lower_key_mem[i] = 0;
	}

	// init the memories with the first 256 codes
	for (unsigned long i = 0; i < 256; i++) {
		bool collision = 0;
		unsigned int key = (i << 8) + 0UL; // lower 8 bits are the next char, the upper bits are the prefix code
		insert(hash_table, &my_assoc_mem, key, i, &collision);
	}

	int i = 0;
	//while (i < s1_len)
	while (i < s1_len) {
		if (i + 1 == s1_len) {
			output_buff[push_index] = prefix_code;
			push_index++;
			break;
		}

		next_char = s1[i + 1];

		bool hit = 0;
		//std::cout << "prefix_code " << prefix_code << " next_char " << next_char << std::endl;
		lookup(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char, &hit,
				&code);
		if (!hit) {
			//            std::cout << prefix_code << "--";
			//            std::cout << "\n";
			output_buff[push_index] = prefix_code;
			push_index++;

			bool collision = 0;
			insert(hash_table, &my_assoc_mem, (prefix_code << 8) + next_char,
					next_code, &collision);
			if (collision) {
				//std::cout << "ERROR: FAILED TO INSERT! NO MORE ROOM IN ASSOC MEM!" << std::endl;
				*out_size = push_index;
				return;
			}
			next_code += 1;

			prefix_code = next_char;
		} else {
			prefix_code = code;
		}

		i++;
	}

	*out_size = push_index;
//	output_buff[0] = 87;
//	output_buff[1] = 89;
//	output_buff[2] = 83;
//	output_buff[3] = 42;
//	output_buff[4] = 256;
//	output_buff[5] = 71;
//	output_buff[6] = 256;
//	output_buff[7] = 258;
//	output_buff[8] = 262;
//	output_buff[9] = 262;
//	output_buff[10] = 71;
//
//	*out_size = 11;
}

//****************************************************************************************************************
// "Golden" functions to check correctness
std::vector<int> encoding(std::string s1)
{
    //std::cout << "Encoding\n";
    std::unordered_map<std::string, int> table;
    for (int i = 0; i <= 255; i++) {
        std::string ch = "";
        ch += char(i);
        table[ch] = i;
    }

    std::string p = "", c = "";
    p += s1[0];
    int code = 256;
    std::vector<int> output_code;
    //std::cout << "String\tOutput_Code\tAddition\n";
    for (int i = 0; i < s1.length(); i++) {
        if (i != s1.length() - 1)
            c += s1[i + 1];
        if (table.find(p + c) != table.end()) {
            p = p + c;
        }
        else {
            //std::cout << p << "\t" << table[p] << "\t\t" << p + c << "\t" << code << std::endl;
            output_code.push_back(table[p]);
            table[p + c] = code;
            code++;
            p = c;
        }
        c = "";
    }
    //std::cout << p << "\t" << table[p] << std::endl;

    output_code.push_back(table[p]);
    return output_code;
}

//void decoding(std::vector<int> op)
//{
//    std::cout << "\n\n-----------------------\n\n";
//    std::unordered_map<int, std::string> table;
//    for (int i = 0; i <= 255; i++) {
//        std::string ch = "";
//        ch += char(i);
//        table[i] = ch;
//    }
//    int old = op[0], n;
//    std::string s = table[old];
//    std::string c = "";
//    c += s[0];
//    std::cout << s;
//    int count = 256;
//    for (int i = 0; i < op.size() - 1; i++) {
//        n = op[i + 1];
//        if (table.find(n) == table.end()) {
//            s = table[old];
//            s = s + c;
//        }
//        else {
//            s = table[n];
//        }
//        std::cout << s;
//        c = "";
//        c += s[0];
//        table[count] = table[old] + c;
//        count++;
//        old = n;
//    }
//}

#ifdef LZWH
void bitpack(int *input, int input_size, BYTE *output, int *output_size) {
	unsigned char output_byte = 0;
	//    std::vector<unsigned char> vec;
	uint32_t push_index = 0;

	for (int i = 0; i < input_size; i += 2) {
		output_byte |= (input[i] & 0xFF0) >> 4;
		//        vec.push_back(output_byte);
		output[push_index] = output_byte;
		push_index++;
		output_byte = 0;
		output_byte |= (input[i] & 0x00F) << 4;
		if (input_size % 2 != 0 && i == input_size - 1) {
			output_byte &= ~(0xF);
		} else {
			output_byte |= (input[i + 1] & 0xF00) >> 8;
			//            vec.push_back(output_byte);
			output[push_index] = output_byte;
			push_index++;
			output_byte = 0;
			output_byte |= input[i + 1] & 0x0FF;
		}
		//        vec.push_back(output_byte);
		output[push_index] = output_byte;
		push_index++;
		output_byte = 0;
	}

	*output_size = push_index;
}
#else
std::vector<unsigned char> bitpack(std::vector<int> input)
{
    unsigned char output_byte = 0;
    int size = input.size();
    std::vector<unsigned char> vec;

    for (int i = 0; i < size; i += 2) {
        output_byte |= (input[i] & 0xFF0) >> 4;
        vec.push_back(output_byte);
        output_byte = 0;
        output_byte |= (input[i] & 0x00F) << 4;
        if (size % 2 != 0 && i == size - 1) {
            output_byte &= ~(0xF);
        }
        else {
            output_byte |= (input[i + 1] & 0xF00) >> 8;
            vec.push_back(output_byte);
            output_byte = 0;
            output_byte |= input[i + 1] & 0x0FF;
        }
        vec.push_back(output_byte);
        output_byte = 0;
    }

    return vec;
}
#endif