#ifndef SERVER_H_
#define SERVER_H_

#include <sys/types.h>
#include <unistd.h>
#include "main.h"

#define BLOCKSIZE 8192 // Tuneable. must match the transmitter side
#define HEADER 2  // max number of elements we can get from ethernet
#define NUM_ELEMENTS 16384

#ifdef  HOST
#define FILE_PATH         "/media/sd-mmcblk0p1/"
#else 
#define FILE_PATH         "/mnt/castor/seas_home/p/pmallick/Documents/project/HLS_p3/"
#endif
#define INPUT_FILE        "LittlePrince.txt"
#define ENCODER_OUT_FILE  "en_out.txt"
#define DECODER_OUT_FILE  "decode_out.txt"

class ESE532_Server{
public:
    //
	int setup_server();

	//
	int get_packet(unsigned char* buffer);


    // how many packets have we distributed
    int packet_counter;

    // how many packets did we break our file into?
    int table_size;

    // data read in from a file
    unsigned char* input_data;

    // table of pointers that points to an array where your packets live
    unsigned char** table;

protected:

};
#endif
