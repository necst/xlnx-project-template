#!/usr/bin/env python
#ATTENTION IF PYTHON OR PYTHON 3
# coding: utf-8

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

import os
import cv2
import numpy as np
import math
import glob
import time
import pandas as pd

import pynq
from pynq import Overlay
from pynq import allocate

import struct
import statistics
import argparse

from ddrbenchmark_handler import my_accel_map


def main():

    parser = argparse.ArgumentParser(description='Python-based host')
    parser.add_argument("-ol", "--overlay", nargs='?', help='Path and filename of the target overlay', default='./drambenchmark_top_wrapper.bit')
    parser.add_argument("-clk", "--clock", nargs='?', help='Target clock frequency of the PL fclk0_mhz', default=100, type=int)
    parser.add_argument("-t", "--core_number", nargs='?', help='Number of // threads', default=1, type=int)
    parser.add_argument("-p", "--platform", nargs='?', help='platform to target.\
     \'Alveo\' is used for PCIe based,\n while others will setup for a Zynq-based environment', default='Alveo')
    parser.add_argument("-id", "--input_dimension", nargs='?', help='Target input dimensions', default=1000, type=int)
    parser.add_argument("-ib", "--input_bitwidth", nargs='?', help='Target input bitwidth', default=512, type=int)
    parser.add_argument("-rp", "--res_path", nargs='?', help='Path of the Results', default='./')

    t=0
    args = parser.parse_args()
    accel_number=args.core_number

    myoverlay = Overlay(args.overlay)

    if args.platform=='Zynq':
        from pynq.ps import Clocks;
        print("Previous Frequency "+str(Clocks.fclk0_mhz))
        Clocks.fclk0_mhz = args.clock; 
        print("New frequency "+str(Clocks.fclk0_mhz))
    host_dt=np.uint32
    host_dt_size = np.dtype(host_dt).itemsize * 8
    packet_factor = math.ceil(args.input_bitwidth / host_dt_size)
    packet_number = args.input_dimension * packet_factor

    accel_list=my_accel_map(myoverlay, args.platform, accel_number, \
        packet_number, host_dt, packet_number, host_dt)

    #test
    iterations=10
    t_tot = 0
    times=[]
    dim=args.input_dimension
    diffs=[]
    start_tot = time.time()
    for i in range(iterations):

        input_vector = np.random.randint(low=0, high=255, size=(packet_number,), dtype=host_dt)
        sw_data_out=accel_list[0].my_func_sw(input_vector, packet_number)
        start_single = time.time()
        accel_list[0].prepare_buff_one(input_vector)
        fpga_data_out = accel_list[0].exec_and_wait()
        end_single = time.time()
        print("Hw result: ")
        print(fpga_data_out)
        print("Sw result: ")
        print(sw_data_out)
        t = end_single - start_single
        times.append(t)
        diff=np.all(fpga_data_out == sw_data_out)
        diffs.append(diff)
        t_tot = t_tot +  t
    end_tot = time.time()

    accel_list[0].reset_cma_buff()
    print("Mean value of hw vs sw difference" +str(np.mean(diffs)))

    df = pd.DataFrame([\
        ["total_time_hw ",t_tot],\
        ["mean_time_hw",np.mean(times)],\
        ["std_time_hw",np.std(times)],\
        ["mean_diff",np.mean(diffs)],\
        ["std_diffs",np.std(diffs)]],\
                    columns=['Label','Test'+str(args.overlay)])
    df_path = os.path.join(args.res_path,'Time_%02d.csv' % (args.clock))
    df.to_csv(df_path, index=False)
    data = {'time'+str(args.overlay):times,\
            'error'+str(args.overlay):diffs}

    df_breakdown = pd.DataFrame(data,\
            columns=['time'+str(args.overlay),'error'+str(args.overlay)])
    df_path_breakdown = os.path.join(args.res_path,'Breakdown_%02d.csv' % (args.clock))
    df_breakdown.to_csv(df_path_breakdown, index=False)
    
    if args.platform =='Alveo':
        myoverlay.free()

    print("The host code is at the end :)")


if __name__== "__main__":
    main()
