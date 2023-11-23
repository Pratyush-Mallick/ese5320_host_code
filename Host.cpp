//#define CL_HPP_CL_2_0_DEFAULT_BUILD
//#define CL_HPP_TARGET_OPENCL_VERSION 200
//#define CL_HPP_MINIMUM_OPENCL_VERSION 0
//#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
//#define CL_API_SUFFIX__VERSION_1_3
//#define CL_USE_OPENCL_2_0_APIS

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>
#include "Utilities.h"
#include "stopwatch.h"

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "stopwatch.h"
#include <iterator>
#include <fstream>
#include "main.h"
#include <CL/cl2.hpp>

//#define TOTAL_PACKETS   397

#define PACKET_SIZE 8192

int offset = 0;
unsigned char* file;
volatile int done = 0;

//stopwatch ethernet_timer;
stopwatch cdc_timer;
stopwatch dedup_timer;
stopwatch lzw_hw_timer;
stopwatch lzw_sw_timer;
stopwatch sha_timer;
stopwatch total_timer;

bool Compare_matrices(int *ch_hw, int *ch_sw, int size)
{
	bool fail = 0;
    for (int X = 0; X < size; X++) {
     //std::cout << ch_hw[X] << " " << ch_sw[X] << std::endl;
      if (ch_hw[X] != ch_sw[X])
      {
        std::cout << "Data not match at " << X << std::endl;
        fail = 1;
		    break;
      }
    }

    return fail;
}

int main(int argc, char ** argv)
{
  	volatile int writer = 0;
  	int length = 0;
  	int count = 0;
    uint64_t total_len = 0;
    int offset = 0;
    unsigned char* file;
    volatile int done = 0;

    BYTE chunk[PACKET_SIZE];
    int max_chunk_size;
    hashtable_t temp;
    BYTE sha_output[SHA256_BLOCK_SIZE];
    size_t chunk_size=0;

    std::vector<uint32_t> output_packet;
    std::vector<hashtable_t> hashTable;
    std::vector<int> chunk_indices;
    std::vector<int> chunk_sizes;
    
    int fail;
    //int output_hw[8192];
    unsigned char input[8192];
    //int out_hw_size;
    //char* chunk_arr = new char[8192];
    
	FILE *fp_in = fopen(FILE_PATH INPUT_FILE, "r");
	if (fp_in == NULL) {
		perror("invalid file");
		exit(EXIT_FAILURE);
	}

	fseek(fp_in, 0, SEEK_END); // seek to end of file
	int file_size = ftell(fp_in); // get current file pointer
	std::cout << file_size << std::endl;
	fseek(fp_in, 0, SEEK_SET); // seek back to beginning of fil

	int bytes_read = fread(input, sizeof(unsigned char), PACKET_SIZE, fp_in);

    std::vector<int> output_sw;
	std::string str((char *)input);

    //EventTimer timer1;
    // ------------------------------------------------------------------------------------
    // Step 1: Initialize the OpenCL environment
     // ------------------------------------------------------------------------------------
    //timer2.add("OpenCL Initialization");
    
    std::cout << "Main program start" << std::endl;
    cl_int err;
    std::string binaryFile = argv[1];
    unsigned fileBufSize;
    std::vector<cl::Device> devices = get_xilinx_devices();
    devices.resize(1);
    cl::Device device = devices[0];
    cl::Context context(device, NULL, NULL, NULL, &err);
    char *fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    cl::Program program(context, devices, bins, NULL, &err);
    cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    cl::Kernel lzw(program, "hardware_encoding", &err);

    // ------------------------------------------------------------------------------------
    // Step 2: Create buffers and initialize test values
    // ------------------------------------------------------------------------------------    
  	cl::Buffer s1_buf(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY, sizeof(char) * PACKET_SIZE, NULL, &err);
    cl::Buffer out_buf(context, CL_MEM_ALLOC_HOST_PTR |CL_MEM_WRITE_ONLY, sizeof(int) * PACKET_SIZE, NULL, &err);
    cl::Buffer out_size(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(int) * 1, NULL, &err);
	
	lzw.setArg(0, out_size);
    lzw.setArg(1, out_buf);
    lzw.setArg(2, s1_buf);
        
    char *Lzw_Input = (char *)q.enqueueMapBuffer(s1_buf, CL_TRUE, CL_MAP_WRITE, 0, sizeof(char) * PACKET_SIZE);
    int *Lzw_Output = (int *)q.enqueueMapBuffer(out_buf, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(int) * PACKET_SIZE);
    int *Lzw_Size = (int *)q.enqueueMapBuffer(out_size, CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(int) * 1);
	
	//memcpy(Lzw_Input, input, sizeof(char) * size);
    
    lzw.setArg(0, out_size);
    lzw.setArg(1, out_buf);
    lzw.setArg(2, s1_buf);
    //lzw.setArg(3, size);
    
    std::vector<cl::Event> write_events;
    std::vector<cl::Event> execute_events;
    std::vector<cl::Event> read_events;
    cl::Event write, execute, read;
	
	// ------------------------------------------------------------------------------------
    // Step 3: Runnin CDC SHA Dedup before the kernel
    // ------------------------------------------------------------------------------------
	
	total_timer.start();
	
    cdc_timer.start();
    max_chunk_size = cdc(chunk_indices, input, PACKET_SIZE);
    cdc_timer.stop();

    sha_timer.start();
    for(int i=0; i< chunk_indices.size(); i++) {
        SHA256_CTX ctx;
        if(i==0) {
            chunk_size = chunk_indices[i];
        } else {
            chunk_size = chunk_indices[i] - chunk_indices[i-1];
        }

        chunk_sizes.push_back(chunk_size);
        if(i==0) {
            memcpy(chunk, &input[0], chunk_size*sizeof(BYTE));
        } else {
            memcpy(chunk, &input[chunk_indices[i]], chunk_size*sizeof(BYTE));
        }

        sha256_hash(&ctx, chunk, sha_output, 1, chunk_size);
        temp.id = i;
        temp.seen=0;
        memcpy(temp.hashval, sha_output, (SHA256_BLOCK_SIZE * sizeof(BYTE)));
        hashTable.push_back(temp);
    }
    sha_timer.stop();
	
    dedup_timer.start();
    dedup(hashTable);
    dedup_timer.stop();
	
    for(int i=0; i<hashTable.size(); i++) {
        if(hashTable[i].seen == 0) {
            std::string chunk_str = "";

            if(i==0) {
                for(int k=0; k<chunk_sizes[0]; k++)
                    chunk_str += input[k];
            }
            else {
                for(int k=chunk_indices[i-1]; k < chunk_indices[i]; k++)
                    chunk_str += input[k];
            }

            int length = chunk_str.length();
            strcpy(Lzw_Input, chunk_str.c_str());
      			
			lzw.setArg(3, length);
			
			// ------------------------------------------------------------------------------------
			// Step 4: Run the kernel
			// ------------------------------------------------------------------------------------
			
			lzw_hw_timer.start();
  
			//std::cout << "Working before memcopy:"<< len << std::endl;
			q.enqueueMigrateMemObjects({s1_buf}, 0 /* 0 means from host*/, NULL, &write);   
			write_events.push_back(write);       
  
			q.enqueueTask(lzw, &write_events, &execute);
			execute_events.push_back(execute);
  
			//make sure the input and output are passed in order, need to ensure the we declare the buffer properly in durin cosimulation
			q.enqueueMigrateMemObjects({out_size, out_buf}, CL_MIGRATE_MEM_OBJECT_HOST, &execute_events, &read);
			read_events.push_back(read);
  
			//clWaitForEvents(1, (const cl_event*)&execute_events[0]);
  
			//q.enqueueMigrateMemObjects({out_buf, out_size}, CL_MIGRATE_MEM_OBJECT_HOST, NULL, &read);
  
			clWaitForEvents(1, (const cl_event*)&read);	
  
			q.finish();
			
			lzw_hw_timer.stop();
      
			lzw_sw_timer.start();
            output_sw = encoding(chunk_str);
			lzw_sw_timer.stop();
      
            //std::cout << " Hardware output size " << Lzw_Size << std::endl;
      
            //std::cout << " Software output size " << output_sw.size() << std::endl;
      
            fail = Compare_matrices(Lzw_Output, &output_sw[0], output_sw.size());
        }
    }

	total_timer.stop();
	
	if(fail)
	{
		printf("test failed!! \n");
	} else {
		printf("test passed!! \n");
	}
	
	//float ethernet_latency =    ethernet_timer.latency() / 1000.0;
	float cdc_latency =         cdc_timer.latency() / 1000.0;
	float sha_latency =         sha_timer.latency() / 1000.0;
	float dedup_latency =       dedup_timer.latency() / 1000.0;
	float lzw_hw_latency =      lzw_hw_timer.avg_latency() / 1000.0;
	float lzw_sw_latency =      lzw_sw_timer.avg_latency() / 1000.0;
	float total_latency =       total_timer.latency() / 1000.0;
    
    //timer1.finish();
	std::cout << "--------------- Key Throughputs ---------------" << std::endl;
    //std::cout << "Ethernet Throughput:" <<  (total_len * 8 / 1000000.0) / ethernet_latency << " Mb/s." << " Latency " << ethernet_timer.latency() << " ns." << std::endl;
    std::cout << "Cdc Throughput:" <<  (PACKET_SIZE * 8 / 1000000.0) / cdc_latency << " Mb/s." << " Latency " << cdc_timer.latency() << " ns." << std::endl;
    std::cout << "SHA Throughput:" <<  (PACKET_SIZE * 8 / 1000000.0) / sha_latency << " Mb/s." << " Latency " << sha_timer.latency() << " ns." << std::endl;
    std::cout << "Dedup Throughput:" <<  (PACKET_SIZE * 8 / 1000000.0) / dedup_latency << " Mb/s." << " Latency " << dedup_timer.latency() << " ns." << std::endl;
	std::cout << "LZW FPGA Throughput:" <<  (PACKET_SIZE * 8 / 1000000.0) / lzw_hw_latency << " Mb/s." << " Latency " << lzw_hw_timer.avg_latency() << " ns." << std::endl;
	std::cout << "LZW Software Throughput:" <<  (PACKET_SIZE * 8 / 1000000.0) / lzw_sw_latency << " Mb/s." << " Latency " << lzw_sw_timer.avg_latency() << " ns." << std::endl;
	std::cout << "Total Throughput (HW + SW):" <<  (PACKET_SIZE * 8 / 1000000.0) / total_latency << " Mb/s." << " Latency " << total_timer.latency() << " ns." << std::endl;

	return 0; 
}