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
#ifndef DDRBENCHMARK_HPP
#define DDRBENCHMARK_HPP
#include "string.h"
#include <stdlib.h>
#include "ap_int.h"

#define INPUT_BIWIDTH 512
#define DATA_ARRAY_SIZE 1000

const unsigned int data_array_size_depth = DATA_ARRAY_SIZE;



#ifndef USING_XILINX_VITIS
	extern void  drambenchmark_top(ap_uint<INPUT_BIWIDTH> * in1, ap_uint<INPUT_BIWIDTH> * out);
#endif //KERNEL_NAME
#else
	extern "C" void drambenchmark_top(ap_uint<INPUT_BIWIDTH> * in1, ap_uint<INPUT_BIWIDTH> * out);
#endif //USING_XILINX_VITIS


#endif //DDRBENCHMARK_HPP