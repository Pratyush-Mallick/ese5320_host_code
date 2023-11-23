// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "main.h"
#include "server.h"


#define PORT	  8080
#define CHUNKSIZE BLOCKSIZE // tuneable but must match client
#define HEADER    2
#define DONE_BIT (1 << 7)

unsigned char table[TOTAL_PACKETS][CHUNKSIZE+HEADER];
FILE* fp;

/*
* checks for a system call error if so exits the application
* code supplied by Andre Dehon from previous homeworks
*/
void Check_error(int Error, const char * Message)
{
  if (Error)
  {
    //fputs(Message, stderr);
    perror(Message);
    exit(EXIT_FAILURE);
  }
}

/*
* checks for a sys call error if so then exits
* code supplied by Andre Dehon from previous homeworks
*/
void Exit_with_error(void)
{
  perror(NULL);
  exit(EXIT_FAILURE);
}

/*
* Frees alloc'd or malloc'd data
* code supplied by Andre Dehon from previous homeworks
*/
void Free(unsigned char * Frame)
{
  free(Frame);
}

/*
* loads data from sd card or file
* code supplied by Andre Dehon from previous homeworks
*/
uint32_t Load_data(unsigned char* Input_data, uint32_t in_size)
{
  //unsigned int Bytes_read;

  // FIle name must be small please note this!!!
//  FILE *File = fopen(INPUT_FILE, "wb");
//  Bytes_read = fread(Input_data, 1, in_size, File);
//
//  fclose(File);

	fp = fopen(FILE_PATH INPUT_FILE, "r");
	if (fp == NULL) {
		perror("invalid file");
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0, SEEK_END); // seek to end of file
	int file_size = ftell(fp); // get current file pointer
	std::cout << file_size << std::endl;
	fseek(fp, 0, SEEK_SET); // seek back to beginning of fil

	//int bytes_read = fread(&Input_data[0], sizeof(unsigned char), file_size, fp);

	return file_size;

	//std::cout << bytes_read << std::endl;

	//return bytes_read;
}

/*
* Stores an output buffer to sd card or file name
* code supplied by Andre Dehon from previous homeworks
*/
void Store_data(const char * Filename,unsigned char * Data, uint32_t Size)
{
  FILE * File = fopen(Filename, "wb");
  if (File == NULL)
    Exit_with_error();

  if (fwrite(Data, 1, Size, File) != Size)
    Exit_with_error();

  if (fclose(File) != 0)
    Exit_with_error();
}

// basic
int ESE532_Server::setup_server()
{
	//input_data = (unsigned char*)malloc( sizeof(unsigned char) * 72000);
//	if(input_data == NULL)
//	{
//		std::cout << "aborting packet" <<std::endl;
//		return 1;
//	}

	//unsigned char* packet = (unsigned char*)malloc( sizeof(unsigned char) * (HEADER + CHUNKSIZE));

	//std::cout << "setting up sever.." << std::endl;
	// read data from file
	int bytes_read = Load_data(input_data, 72000);

	// reconcile how to break up the file
	int loops = bytes_read / CHUNKSIZE;
	int remainder = bytes_read % CHUNKSIZE;
	int n;

	int table_size = loops;
	// account for remainder
	if(remainder)
	{
		table_size++;
	}

	//printf("table constructed of %d packets and %d bytes \n",table_size, bytes_read);
    // send chunk size bytes
	for( n = 0; n < loops-1; n++)
	{
		int bytes_read = fread(&table[n][0], sizeof(unsigned char), CHUNKSIZE, fp);
		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

		table[n][0] = low;
		table[n][1] = high;
	}

	// if no remainder send the last chunk with done bit
	if(remainder == 0){

		//unsigned char* packet = (unsigned char*)malloc( sizeof(unsigned char) * (HEADER + CHUNKSIZE) );
		//unsigned char* packet = &table[loops-1][0];

		int bytes_read = fread(&table[n][0], sizeof(unsigned char), CHUNKSIZE, fp);

		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

		table[n][0] = low;
		table[n][1] = high | DONE_BIT;

		n++;
	}
	// if remainder then send last chunk as normal
	else{

		int bytes_read = fread(&table[n][0], sizeof(unsigned char), CHUNKSIZE, fp);

		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

		//
		//packet[0] = low;
		//packet[1] = high;

		table[n][0] = low;
		table[n][1] = high;

		//table[n] = packet;

		n++;
	}

	// send remaining pieces
	if(remainder){
		//unsigned char* packet =  (unsigned char*)malloc( sizeof(unsigned char) * (CHUNKSIZE + HEADER) );

		//unsigned char* packet = &table[loops-1][0];

		int bytes_read = fread(&table[n][0], sizeof(unsigned char), CHUNKSIZE, fp);

//		if(packet == NULL)
//		{
//			std::cout << "aborting packet" <<std::endl;
//			return 1;
//		}
		//
		//memcpy(&packet[HEADER],&input_data[loops*CHUNKSIZE],remainder);

		//
		char high = CHUNKSIZE >> 8;
		char low = CHUNKSIZE & 0xFF;

//		packet[0] = low;
//		packet[1] =  high | DONE_BIT;

		table[n][0] = low;
		table[n][1] =  high | DONE_BIT;


		//table[n] = packet;
		n++;
	}

	fclose(fp);
	//free(input_data);
	packet_counter = 0;
	printf("server setup complete!\n");

	return 0;

//	// make our pointer table
//	table = (unsigned char**)malloc( sizeof(unsigned char*) * table_size);
//	printf("table constructed of %d packets and %d bytes \n",table_size, bytes_read);
//	if(table == NULL)
//	{
//		std::cout << "aborting packet" <<std::endl;
//		return 1;
//	}
//
//    // send chunk size bytes
//	for( n = 0; n < loops-1; n++)
//	{
//		unsigned char* packet = (unsigned char*)malloc( sizeof(unsigned char) * (HEADER + CHUNKSIZE) );
//		if(packet == NULL)
//		{
//			std::cout << "aborting packet" <<std::endl;
//			return 1;
//		}
//		//
//		memcpy(&packet[HEADER],&input_data[n*CHUNKSIZE],CHUNKSIZE);
//
//		//
//		char high = CHUNKSIZE >> 8;
//		char low = CHUNKSIZE & 0xFF;
//
//		//
//		packet[0] = low;
//		packet[1] = high;
//
//		table[n] = packet;
//	}
//
//	// if no remainder send the last chunk with done bit
//	if(remainder == 0){
//
//		unsigned char* packet = (unsigned char*)malloc( sizeof(unsigned char) * (HEADER + CHUNKSIZE) );
//		if(packet == NULL)
//		{
//			std::cout << "aborting packet" <<std::endl;
//			return 1;
//		}
//		//
//		memcpy(&packet[HEADER],&input_data[n*CHUNKSIZE],CHUNKSIZE);
//
//		//
//		char high = CHUNKSIZE >> 8;
//		char low = CHUNKSIZE & 0xFF;
//
//		//
//		packet[0] = low;
//		packet[1] = high | DONE_BIT;
//
//		table[n] = packet;
//		n++;
//	}
//	// if remainder then send last chunk as normal
//	else{
//
//		unsigned char* packet = (unsigned char*)malloc( sizeof(unsigned char) * (HEADER + CHUNKSIZE) );
//		if(packet == NULL)
//		{
//			std::cout << "aborting packet" <<std::endl;
//			return 1;
//		}
//		//
//		memcpy(&packet[HEADER],&input_data[n*CHUNKSIZE],CHUNKSIZE);
//
//		//
//		char high = CHUNKSIZE >> 8;
//		char low = CHUNKSIZE & 0xFF;
//
//		//
//		packet[0] = low;
//		packet[1] = high;
//
//		table[n] = packet;
//
//		n++;
//	}
//
//	// send remaining pieces
//	if(remainder){
//		unsigned char* packet =  (unsigned char*)malloc( sizeof(unsigned char) * (CHUNKSIZE + HEADER) );
//		if(packet == NULL)
//		{
//			std::cout << "aborting packet" <<std::endl;
//			return 1;
//		}
//		//
//		memcpy(&packet[HEADER],&input_data[loops*CHUNKSIZE],remainder);
//
//		//
//		char high = CHUNKSIZE >> 8;
//		char low = CHUNKSIZE & 0xFF;
//
//		//
//		packet[0] = low;
//		packet[1] =  high | DONE_BIT;
//
//		table[n] = packet;
//		n++;
//	}
//
//	free(input_data);
//	packet_counter = 0;
//	printf("server setup complete!\n");
//
//	return 0;
}

int ESE532_Server::get_packet(unsigned char* buffer)
{
	// get next packet and copy into passed in buffer

	memcpy(buffer, &table[packet_counter][0], CHUNKSIZE + HEADER);
	packet_counter++;
	return (CHUNKSIZE + HEADER);
}
