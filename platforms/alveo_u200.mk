#######################################################################################################################################
#
#   Basic Makefile for Vitis 2019.2
#   Davide Conficconi, Emanuele Del Sozzo
#   {davide.conficconi, emanuele.delsozzo}@polimi.it
#
# 
#  See https://www.xilinx.com/html_docs/xilinx2019_2/vitis_doc/Chunk1193338764.html#wrj1504034328013
#  for more information about Vitis compiler options:) 
#######################################################################################################################################

CORE_LIST_NR = $(shell seq 1 ${CORE_NR})
BRD_PARTS= "xcu200-fsgd2104-2-e"
#
TARGET_DEVICE=xilinx_u200_xdma_201830_2
# TARGET DSA # 


# #Host code
HOST_SRC=$(HOSTCPP_DIR)/alveo/drambenchmark_host_alveo.cpp $(HOSTCPP_DIR)/alveo/xcl2/xcl2.cpp
HOST_HDRS= $(wildcard $(HOSTCPP_DIR)/alveo/*.hpp)

HOST_CFLAGS=-g  -I${XILINX_XRT}/include/ \
	-I $(HOSTCPP_DIR)/alveo/xcl2 -I$(XILINX_XRT)/include
 	 #-O3 -I${XILINX_VITIS}/include/ --std=c++1y

HOST_LFLAGS=-L${XILINX_XRT}/lib/ -lxilinxopencl -pthread 
#-lopencv_core -lopencv_highgui -lopencv_imgproc \
# 	-L$(XILINX_VITIS)/lnx64/tools/opencv


 ifndef LD_LIBRARY_PATHVTS_DST_DIR
 	LD_LIBRARY_PATH=$(XILINX_VITIS)/lnx64/tools/opencv:/usr/lib/:/usr/lib64/
 else
 	LD_LIBRARY_PATH:=$(LD_LIBRARY_PATH):$(XILINX_VITIS)/lnx64/tools/opencv:/usr/lib:/usr/lib64/
 endif

ifeq (,$(LD_LIBRARY_PATH))
LD_LIBRARY_PATH := $(XILINX_XRT)/lib
else
LD_LIBRARY_PATH := $(XILINX_XRT)/lib:$(LD_LIBRARY_PATH)
endif
ifneq (,$(wildcard $(XILINX_VITIS)/bin/ldlibpath.sh))
export LD_LIBRARY_PATH := $(shell $(XILINX_VITIS)/bin/ldlibpath.sh $(XILINX_VITIS)/lib/lnx64.o):$(LD_LIBRARY_PATH)
endif

#name of host executable
HOST_EXE=$(KERNEL)_host_exe
#argument passed to the execution of the kernel
HOST_ARGS=$(KERNEL_EXE).xclbin


#kernel
KERNEL_SRC=$(HLS_DIR)/$(TOP_FILE_NAME)
KERNEL_HDRS= $(HLS_DIR)/
KERNEL_SRC_CONFIG = $(HLS_CONFIG_DIR)/$(TOP_FILE_NAME)
KERNEL_HDRS_CONFIG = $(HLS_CONFIG_DIR)/
KERNEL_FLAGS= -D USING_XILINX_VITIS
KERNEL_EXE=$(KERNEL)
KERNEL_NAME=$(KERNEL)

# #######################################################



 XO_LIST = $(foreach core, $(CORE_LIST_NR), $(VTS_DST_DIR)/$(KERNEL_NAME)_$(core).xo )


#Port mapping for Vitis version up to 2019.2 and other advanced options
#care that gmem0 gmem1 gmem2 are specific bundle of the core
krnl_map_lcdlflags = --connectivity.nk $(KERNEL_NAME)_$(1):1 \
	--connectivity.sp $(KERNEL_NAME)_$(1)_1.m_axi_gmem:DDR[$(2)]
# \
#	--connectivity.sp $(KERNEL_NAME)_$(1)_1.m_axi_gmem1:DDR[$(2)] \
#	--connectivity.sp $(KERNEL_NAME)_$(1)_1.m_axi_gmem2:DDR[$(2)]

#advanced options not used
#   --connectivity.slr $(KERNEL_NAME)_$(1)_1:SLR$(3) \
#   --hls.memory_port_data_width 256
#	--hls.max_memory_ports $(KERNEL_NAME)_$(1) \
#--advanced.prop kernel.$(KERNEL_NAME)_$(1).kernel_flags="-std=c++11"

KRNL_LDCLFLAGS_MULTI_CORE = 

XO_GEN_FLAGS = -D KERNEL_NAME=$(KERNEL_NAME)_$(1)
RUN_TIME_INPUT_KERNEL_NAME=$(KERNEL_NAME)_$(1)

ifeq ($(CORE_NR),1)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,1,0,0)
else ifeq ($(CORE_NR), 2)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,1,0,0)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,2,1,1)
else ifeq ($(CORE_NR), 3)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,1,0,0)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,2,1,1)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,3,2,1)
else ifeq ($(CORE_NR), 4)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,1,0,0)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,2,1,1)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,3,2,1)
KRNL_LDCLFLAGS_MULTI_CORE += $(call krnl_map_lcdlflags,4,3,2)
endif


#######################################################

include platforms/vitis_alveo_like.mk