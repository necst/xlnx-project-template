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
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <random>
#include <stdio.h>
#include <chrono>
#include <fstream>
#include <string>
#include <cstring>


#include "platform.hpp"


#define MAGIC_NUMBER 1000

#define BANDWIDTH 512
#define HOST_DATA_BITWIDTH 32
#define AP_UINT_FACTOR BANDWIDTH / HOST_DATA_BITWIDTH
#define ARRAY_TEST_DIM MAGIC_NUMBER * AP_UINT_FACTOR
#define RANGE_UPPER_BOUND 255

#define ADDR_AP_CTRL 0x0000
#define ADDR_IN1 0x10

#define ADDR_OUT_1 0x18

#define REPETITIONS 100


std::ofstream logger;
int rep = 0;
char test [20]="";

// perform a test with random images
void random_test();


// execute and wait
void execAndWait();

//prepare the buffers and set the IP
void prepareIP(int * ref);

// execute a single test and display results and time
void execTest(int * ref);

// execute the test in order to get avg execution times
void execTestMultiple(int * ref);
//
void logging(double time, char * plt, bool res);



DonutDriver * platform = 0;
void * hw_in_buff;
void * hw_out_buff;

void logging(double time, char * plt, bool res){
   logger << std::fixed << test << std::scientific << "\t" << time << "\t" << plt << "\t"  << rep << "\t" << res << std::endl;

}


void execAndWait(){
   int ap_ctrl  = platform->readJamRegAddr(ADDR_AP_CTRL);
   platform->writeJamRegAddr(ADDR_AP_CTRL, ((ap_ctrl &0x80)| 0x01) );
   bool finish = false;
   while(!finish){
      ap_ctrl = platform->readJamRegAddr(ADDR_AP_CTRL);
      finish = (ap_ctrl >> 1) & 0x1;
   }
}

void prepareIP(int * ref){

   size_t ref_bytes = ARRAY_TEST_DIM * sizeof(int);

   hw_in_buff = platform->allocAccelBuffer(ref_bytes);
   hw_out_buff = platform->allocAccelBuffer(ref_bytes);

   platform->copyBufferHostToAccel(ref, hw_in_buff, ref_bytes );
   platform->writeJamRegAddr(ADDR_IN1,(uint64_t )hw_in_buff);
   platform->writeJamRegAddr(ADDR_OUT_1,(uint64_t )hw_out_buff);
}

void testCheck(int *ref, int * out, bool * res){
  for(int i =0; i < ARRAY_TEST_DIM; i++){
    if (ref[i] != out[i])
    {
      *res=false;
    }
  }
}



void execTest(int * ref, int * out){

   bool result=true;
   prepareIP(ref);
   auto start_hw = std::chrono::system_clock::now();
   execAndWait();
   auto end_hw = std::chrono::system_clock::now();
   //read back result
   platform->copyBufferAccelToHost(hw_out_buff, out, ARRAY_TEST_DIM * sizeof(int));
   testCheck(ref,out, &result);
   //
   double nseconds_hw = std::chrono::duration_cast<std::chrono::nanoseconds>(end_hw - start_hw).count(); 
   logging(nseconds_hw, "hw", result);
   std::cout << "result hw " << result << std::endl;

   std::cout << "time hw " << nseconds_hw << "[ns] time sw :?" <<std::endl;
}

void execTestMultiple(int * ref, int * out){
  double tot_time_sw = 0.0;
  double tot_time_hw = 0.0;
  double sw_avg = 0.0;
  double hw_avg = 0.0;
  bool result=true;
  rep=0;

  for (int i = 0; i < REPETITIONS;i++)
  {
   prepareIP(ref);
   auto start_hw = std::chrono::system_clock::now();
   execAndWait();
   auto end_hw = std::chrono::system_clock::now();
   //read back result
   platform->copyBufferAccelToHost(hw_out_buff, out, ARRAY_TEST_DIM * sizeof(int));
   testCheck(ref,out,&result);
   //
   double nseconds_hw = std::chrono::duration_cast<std::chrono::nanoseconds>(end_hw - start_hw).count();
   logging(nseconds_hw, "hw", result);
   std::cout << "result hw " << result << std::endl;
   rep++;
   tot_time_hw += nseconds_hw;
   result = true;
  }
  rep=0;
  hw_avg = tot_time_hw / REPETITIONS;

   std::cout << "Average time hw " << hw_avg << " [ns]  "<<std::endl;
}


void random_test(){
   std::cout << "************************" << std::endl;
   std::cout  << std::endl;
   std::cout << "Random input test" << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;

   int ref[ARRAY_TEST_DIM];
   int out[ARRAY_TEST_DIM];

   int myseed = 1234;

   std::default_random_engine rng(myseed);
   std::uniform_int_distribution<int> rng_dist(0, RANGE_UPPER_BOUND);

   for(int i=0;i<ARRAY_TEST_DIM;i++){
         ref[i]=static_cast<int>(rng_dist(rng));
         out[i]=0;
   }

   std::string s = "Rand";
   strcpy(test, s.c_str()); 
   execTest(ref,out);
    #ifdef AVERAGE_REPS
      execTestMultiple(ref,out);
    #endif
}




int main(int argc, char * argv[]){


   const char * attachName = "ddrbenchmark_wrapper";
   platform = initPlatform();
   platform->attach(attachName);
   logger.open ("benchmark.csv");
   logger <<"Test\tTime[ns]\tHW/SW";
    #ifdef AVERAGE_REPS
      logger << "\tRepetion#";
    #endif
    logger << std::endl;
   
   random_test();
   std::cout  << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;
   std::cout  << std::endl;
   std::cout << "End of Random input test" << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;
  
   platform->deallocAccelBuffer(hw_in_buff);

   deinitPlatform(platform);
   platform = 0;

   return 0;
}


