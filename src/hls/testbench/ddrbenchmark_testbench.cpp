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


#include "ddrbenchmark.hpp"


#define MAGIC_NUMBER 1000

#define BANDWIDTH 512
#define HOST_DATA_BITWIDTH 32
#define AP_UINT_FACTOR BANDWIDTH / HOST_DATA_BITWIDTH
#define ARRAY_TEST_DIM MAGIC_NUMBER * AP_UINT_FACTOR
#define RANGE_UPPER_BOUND 255

#define REPETITIONS 100


// perform a test with random images
void random_test();

// // exec effectively the test
// void hw_test(int * ref, bool res);
// execute a single test and display results and time
void execTest(int * ref);

// execute the test in order to get avg execution times



void execTest(int * ref, int * out){

   bool result=true;
   drambenchmark_top(ref, out);
   //read back result
   testCheck(ref,out, &result);
   //
   printf("%d\n", result);
   std::cout << "result hw " << result << std::endl;

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
    drambenchmark_top(ref, out);
   //read back result
   testCheck(ref,out,&result);
   //
   std::cout << "result hw " << result << std::endl;
   rep++;
   result = true;
  }
  rep=0;
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
   execTest(ref,out);
    #ifdef AVERAGE_REPS
      execTestMultiple(ref,out);
    #endif
}




int main(int argc, char * argv[]){
   
   random_test();
   std::cout  << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;
   std::cout  << std::endl;
   std::cout << "End of Random input test" << std::endl;
   std::cout  << std::endl;
   std::cout << "************************" << std::endl;

   return 0;
}


