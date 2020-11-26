/******************************************
#*MIT License
#*
#*Copyright (c) [2020] [Davide Conficconi, Emanuele Del Sozzo, Eleonora D'Arnese]
#*
#*Permission is hereby granted, free of charge, to any person obtaining a copy
#*of this software and associated documentation files (the "Software"), to deal
#*in the Software without restriction, including without limitation the rights
#*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#*copies of the Software, and to permit persons to whom the Software is
#*furnished to do so, subject to the following conditions:
#*
#*The above copyright notice and this permission notice shall be included in all
#*copies or substantial portions of the Software.
#*
#*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#*AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#*LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#*SOFTWARE.
#******************************************/
#include "host.hpp"
#include "xcl2.hpp"
#include <vector>


#define INPUT_SIZE 65536

#define MAGIC_NUMBER 1000
#define BANDWIDTH 512
#define HOST_DATA_BITWIDTH 32
#define AP_UINT_FACTOR BANDWIDTH / HOST_DATA_BITWIDTH
#define ARRAY_TEST_DIM MAGIC_NUMBER * AP_UINT_FACTOR
#define RANGE_UPPER_BOUND 255


template<typename T>
void software_test(std::vector<T,aligned_allocator<T>> source_host_memory, 
    std::vector<T,aligned_allocator<T>> swOutput_memory){
    for (int i = 0; i < ARRAY_TEST_DIM; ++i)
    {
        swOutput_memory[i]=source_host_memory[i];
    }
}

template<typename T>
bool verifyOutputs(std::vector<T,aligned_allocator<T>> swOutput_memory, 
    std::vector<T,aligned_allocator<T>> hwOutput_memory){
    for (int i = 0; i < ARRAY_TEST_DIM; ++i)
    {
        if (swOutput_memory[i] != hwOutput_memory[i])
        {
            return false;
        }
    }
    return true;
}


template<typename T>
void prepareRandomTest(std::vector<T,aligned_allocator<T>>  ref){

    std::cout << "************************" << std::endl;
    std::cout  << std::endl;
    std::cout << "Random input test" << std::endl;
    std::cout  << std::endl;
    std::cout << "************************" << std::endl;

    // int ref[ARRAY_TEST_DIM];

    int myseed = 1234;

    std::default_random_engine rng(myseed);
    std::uniform_int_distribution<int> rng_dist(0, RANGE_UPPER_BOUND);

    for(int i=0;i<ARRAY_TEST_DIM;i++){
         ref[i]=static_cast<T>(rng_dist(rng));
    }
}

int main(int argc, char** argv) {


    if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
    return EXIT_FAILURE;
    }

    // std::string binaryFile = argv[1];


    cl_int err;
    std::vector<int,aligned_allocator<int>> 
    source_host_memory(ARRAY_TEST_DIM);
    std::vector<int,aligned_allocator<int>> 
    hwOutput_memory(ARRAY_TEST_DIM);
    std::vector<int,aligned_allocator<int>> 
    swOutput_memory(ARRAY_TEST_DIM);

    size_t size_in_bytes = source_host_memory.size() * sizeof(int);

    prepareRandomTest<int>(source_host_memory);
    // Create the test data 

    // OPENCL HOST CODE AREA START
    // The get_xil_devices will return vector of Xilinx Devices
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];
    // Creating Context and Command Queue for selected Device
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE,
                                    &err));

    std::string device_name = device.getInfo<CL_DEVICE_NAME>();
    std::cout << "Found Device=" << device_name.c_str() << std::endl;

    // Import XCLBIN
    std::string xclbin_file_name = argv[1];
    auto fileBuf = xcl::read_binary_file(xclbin_file_name);
    cl::Program::Binaries my_bins{{fileBuf.data(), fileBuf.size()}};

    // Program and Kernel
    devices.resize(1);
    //cl::Program program(context, devices, my_bins);
    OCL_CHECK(err,cl::Program program(context, {device}, my_bins, NULL, &err));

    auto kernel_name = argv[2];
    if(kernel_name == NULL){
        kernel_name = "drambenchmark_top";
    }
    //kernel name
    OCL_CHECK(err,cl::Kernel krnl_drambenchmark(program, kernel_name));

    // Allocate
    OCL_CHECK(err, cl::Buffer buffer_input(context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, 
            size_in_bytes, source_host_memory.data(), &err));


    OCL_CHECK(err, cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, 
            size_in_bytes, hwOutput_memory.data(), &err));

    OCL_CHECK(err, err = krnl_drambenchmark.setArg(0, buffer_input));
    OCL_CHECK(err, err = krnl_drambenchmark.setArg(1, buffer_output));


    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_input},0/* 0 means from host*/));
    q.finish();

    auto start = std::chrono::high_resolution_clock::now();

    // Launch the Kernel
    OCL_CHECK(err, err = q.enqueueTask(krnl_drambenchmark));
    q.finish();

    auto stop = std::chrono::high_resolution_clock::now();

    // Copy Result from Device Global Memory to Host Local Memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();

    auto fpga_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

    std::cout << "FPGA implementation took: " << fpga_duration.count() << " nanoseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();

    software_test<int>(source_host_memory, swOutput_memory);

    stop = std::chrono::high_resolution_clock::now();

    auto cpu_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

    std::cout << "CPU and FPGA have " << verifyOutputs<int>(swOutput_memory,hwOutput_memory) << " as result of output equality" << std::endl;
    std::cout << "CPU implementation took: " << cpu_duration.count() << " nanoseconds" << std::endl;

    std::cout << "Difference of: " << (float)cpu_duration.count() / (float)fpga_duration.count() << " times" << std::endl;

    //std::cout << "FPGA result: " << fpga_result << std::endl << "CPU result: " << cpu_result << std::endl;
}