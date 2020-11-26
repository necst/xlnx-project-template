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
#include "ddrbenchmark.hpp"

void topComparisonTester(char *a, char *b, bool * res){

	char local_a, local_b, local_res;
	local_a = *a;
	local_b = *b;
	local_res = local_a == local_b;

	*res=local_res;
	return;

}

#ifdef KERNEL_NAME
extern "C"{
	void KERNEL_NAME
#else
	void drambenchmark_top
#endif //KERNEL_NAME
	(ap_uint<INPUT_BIWIDTH> * in1, ap_uint<INPUT_BIWIDTH> * out){
#pragma HLS INTERFACE m_axi port=in1 depth=data_array_size_depth offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=out depth=data_array_size_depth offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite register port=in1 bundle=control
#pragma HLS INTERFACE s_axilite register port=out bundle=control
#pragma HLS INTERFACE s_axilite register port=return bundle=control

	ap_uint<INPUT_BIWIDTH> in1_local[DATA_ARRAY_SIZE];

	for(int i=0; i < DATA_ARRAY_SIZE; i++){
#pragma HLS PIPELINE
		in1_local[i] = *(in1+i);
	}

	for(int i=0; i < DATA_ARRAY_SIZE; i++){
#pragma HLS PIPELINE
		out[i] = in1_local[i];
	}

	return;

}
#ifdef KERNEL_NAME
}
#endif //KERNEL_NAME