import pynq
from pynq import allocate
import numpy as np

class SingleAccelDRAMBenchmark :
    def __init__(self, accel_id,  platform='Alveo', mem_bank=None, in_vec_size=512, in_vec_dt=np.uint8, out_vec_size=512, out_vect_dt=np.uint8 ):
            self.AP_CTRL = 0x00
            self.done_rdy = 0x6
            self.ap_start = 0x1
            self.IN1_ADDR = 0x10
            self.OUT_ADDR = 0x18

            self.buff1_input = allocate(in_vec_size, in_vec_dt, target=mem_bank, cacheable=True)
            self.buff2_output = allocate(out_vec_size, out_vect_dt, target=mem_bank, cacheable=True)

            self.buff1_input_addr = self.buff1_input.device_address
            self.buff2_output_addr = self.buff2_output.device_address
            
            self.accel = accel_id
            self.platform = platform


    def init_accel(self, input_vector):
        self.prepare_buff_one(input_vector)
    

    def prepare_buff_one(self, input_vector):
        self.buff1_input[:] = input_vector.flatten()
        self.buff1_input.flush()#sync_to_device


    def execute_zynq(self):
        self.accel.write(self.IN1_ADDR, self.buff1_input.device_address)
        self.accel.write(self.OUT_ADDR, self.buff2_output.device_address)
        self.accel.write(self.AP_CTRL, self.ap_start)
        while(self.accel.mmio.read(0) & 0x4 != 0x4):
            pass
    
    def exec_and_wait(self):
        result = []
        if self.platform == 'Alveo':
            self.accel.call(self.buff1_input, self.buff2_output)
        else:# ZYNQ based
            self.execute_zynq()
        self.buff2_output.invalidate()#sync_from_device
        result.append(self.buff2_output)
        
        return result[0]

    
    def reset_cma_buff(self):
        del self.buff1_input 
        del self.buff2_output

    def my_func_sw(self, input_vector, packet_number):
        return np.copy(input_vector)


# function for specific multicore mapping on different platforms, memory banks and namings
def my_accel_map(overlay_pl, platform='Alveo', core_number=1, in_vec_size=512, in_vec_dt=np.uint8, out_vec_size=512, out_vect_dt=np.uint8):

    mem_bank=None
    my_accel_list = []

    if(core_number>=1):
        if platform == 'Alveo':#pcie card based
            mem_bank=overlay_pl.bank0 #mapping specific!!!!! 
            my_accel_0=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_1_1, platform, mem_bank,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        else: #ZYNQ based
            my_accel_0=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_0, platform, None,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        my_accel_list.append(my_accel_0)

    if (core_number >= 2):
        if platform == 'Alveo':#pcie card based
            mem_bank=overlay_pl.bank1 #mapping specific!!!!! 
            my_accel_1=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_2_1, platform, mem_bank,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        else: #ZYNQ based
            my_accel_1=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_1,platform,None,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt,config)
        my_accel_list.append(my_accel_1)

    if(core_number >= 3):
        if platform == 'Alveo':#pcie card based
            mem_bank=overlay_pl.bank2 #mapping specific!!!!! 
            my_accel_2=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_3_1,platform, mem_bank,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        else: #ZYNQ based
            my_accel_2=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_2,platform,None,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        my_accel_list.append(my_accel_2)

    if(core_number >= 4):
        if platform == 'Alveo':#pcie card based
            mem_bank=overlay_pl.bank3 #mapping specific!!!!! 
            my_accel_3=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_4_1,platform, mem_bank,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        else: #ZYNQ based
            my_accel_3=SingleAccelDRAMBenchmark(overlay_pl.drambenchmark_top_3,platform,None,\
                in_vec_size, in_vec_dt, out_vec_size, out_vect_dt)
        my_accel_list.append(my_accel_3)
        
    return my_accel_list