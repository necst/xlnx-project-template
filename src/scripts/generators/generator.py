#/******************************************
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


import argparse
import os
import numpy
import math
###################################################################################################################################
def print_ddrbenchmark_config(inp_img_bits, inp_img_dim, vitis, out_path, config_name):
    if vitis:
        vitis_externC="\"C\""
    else:
        vitis_externC=""

    ddrbenchmark_header = open(out_path+config_name,"w+")
    ddrbenchmark_header.write("\n \
#ifndef DDRBENCHMARK_HPP \n \
#define DDRBENCHMARK_HPP \n \
#include \"string.h\" \n \
#include <stdlib.h> \n \
#include \"ap_int.h\" \n \
\n \
#define INPUT_BIWIDTH {0}\n \
#define DATA_ARRAY_SIZE {1}\n \
\n \
const unsigned int data_array_size_depth = DATA_ARRAY_SIZE;\n \
\n \
\n \
\n \
#ifndef USING_XILINX_VITIS\n \
\n \
    extern void  drambenchmark_top(ap_uint<INPUT_BIWIDTH> * in1, ap_uint<INPUT_BIWIDTH> * out);\n \
#else\n \
    extern {2} void drambenchmark_top(ap_uint<INPUT_BIWIDTH> * in1, ap_uint<INPUT_BIWIDTH> * out);\n \
#endif //USING_XILINX_VITIS\n \
\n \
\n \
#endif //DDRBENCHMARK_HPP\n".format(inp_img_bits, \
    inp_img_dim, \
    vitis_externC) )


###################################################################################################################################
###################################################################################################################################
###################################################################################################################################

def main():
    parser = argparse.ArgumentParser(description='Configuration Generation for the DDRbenchmark')
    parser.add_argument("-op","--out_path", nargs='?', help='output path for the out files, default ./', default='./')
    parser.add_argument("-cfg","--config_name", nargs='?', help='configuration file name', default='ddrbenchmark.hpp')
    parser.add_argument("-c", "--clean", help='clean previously created files befor starting', action='store_true')
    parser.add_argument("-ib", "--in_bits", nargs='?', help='number of bits of a single axi packet, default 32', default='32', type=int)
    parser.add_argument("-id", "--in_dim", nargs='?', help='number of axi packets to r/w, default 500', default='500', type=int)
    parser.add_argument("-vts", "--vitis", help='generate vitis version?', action='store_true')
    args = parser.parse_args()
    if args.clean:
        os.remove(args.out_path+args.config_name)

    print_ddrbenchmark_config(args.in_bits,args.in_dim, args.vitis, args.out_path,args.config_name)


if __name__== "__main__":
    main()
