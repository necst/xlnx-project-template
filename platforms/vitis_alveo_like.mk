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



VPP=v++
XCC=g++
JOBS=8


REPORT_FLAG = -R0 
# -R0 -R1 -R2 

#######################################################

FULL_DEBUG=true
ifdef FULL_DEBUG
KERNEL_ADDITIONAL_FLAGS= --save-temps --log_dir $(VTS_DST_DIR)/ \
	--temp_dir $(VTS_DST_DIR)/ --report_dir $(VTS_DST_DIR) \
	--debug
endif

#######################################################

#######################################################
#Optimization levels: 0 1 2 3 s quick
# 0: Default optimization. Reduces compilation time and makes debugging produce the expected results.
# 1: Optimizes to reduce power consumption. This takes more time to build the design.
# 2: Optimizes to increase kernel speed. This option increases build time, but also improves the performance of the generated kernel.
# 3: This optimization provides the highest level performance in the generated code, but compilation time can increase considerably.
# s: Optimizes for size. This reduces the logic resources of the device used by the kernel.
# quick: Reduces Vivado implementation time, but can reduce kernel performance, and increases the resources used by the kernel
OPT_LVL=
ifdef OPT_LVL
    KERNEL_ADDITIONAL_FLAGS += --optimize $(OPT_LVL)
endif

#--kernel_frequency <clockID>:<freq>|<clockID>:<freq>
ifdef CLK_FRQ
    ifdef KRNL_FRQ
KERNEL_LDCLFLAGS += --kernel_frequency "0:$(CLK_FRQ)|1:$(KRNL_FRQ)" #--optimize 3 
    else
KERNEL_LDCLFLAGS += --kernel_frequency "0:$(CLK_FRQ)" #|1:350"
KRNL_LDCLFLAGS_MULTI_CORE += --kernel_frequency "0:$(CLK_FRQ)"
    endif
else
    ifdef KRNL_FRQ
KERNEL_LDCLFLAGS+= --kernel_frequency "1:$(KRNL_FRQ)" #--optimize 3 
    else
        #KERNEL_LDCLFLAGS+= --k ernel_frequency "200"
    endif

endif




#TARGET for compilation [sw_emu | hw_emu | hw]
TARGET=hw
REPORT_FLAG=n
REPORT=
ifeq (${TARGET}, sw_emu)
$(info software emulation)
TARGET=sw_emu
ifeq (${REPORT_FLAG}, y)
$(info creating REPORT for software emulation set to true. This is going to take longer at it will synthesize the kernel)
REPORT=-Restimate
else
$(info I am not creating a REPORT for software emulation, set REPORT_FLAG=y if you want it)
REPORT=
endif
else ifeq (${TARGET}, hw_emu)
$(info hardware emulation)
TARGET=hw_emu
REPORT= 
else ifeq (${TARGET}, hw)
$(info system build)
TARGET=hw
REPORT=
else
$(info no TARGET selected)
endif

#######################################################

PERIOD:= :
UNDERSCORE:= _
VTS_DST_DIR=$(VTS_BUILD_DIR)/$(TARGET)/$(subst $(PERIOD),$(UNDERSCORE),$(TARGET_DEVICE))

#######################################################

ifndef XILINX_VITIS
$(error XILINX_VITIS is not set. Please source the Vitis settings64.{csh,sh} first)
endif

ifndef XILINX_XRT
$(error XILINX_XRT is not set. Please source the XRT /opt/xilinx/xrt/setup.sh first)
endif

check_TARGET: curr_status
ifeq (${TARGET}, none)
    $(error Target can not be set to none)
endif

xo: check_TARGET
	mkdir -p $(VTS_DST_DIR)
	$(VPP) --platform $(TARGET_DEVICE) -t $(TARGET) \
	--jobs $(JOBS) --compile --include $(KERNEL_HDRS) $(REPORT) \
	--kernel $(KERNEL_NAME) $(KERNEL_FLAGS) \
	$(KERNEL_ADDITIONAL_FLAGS) -o '$(VTS_DST_DIR)/$(KERNEL_EXE).xo' \
	$(KERNEL_SRC)

hw: xclbin

hw_gen: xclbin_config

xclbin:  check_TARGET xo
	$(VPP) --platform $(TARGET_DEVICE) -t $(TARGET) \
	--link --jobs $(JOBS) --include $(KERNEL_HDRS) $(REPORT) \
	 --kernel $(KERNEL_NAME) $(VTS_DST_DIR)/$(KERNEL_EXE).xo $(KERNEL_LDCLFLAGS) \
	$(KERNEL_FLAGS) $(KERNEL_ADDITIONAL_FLAGS) \
	-o '$(VTS_DST_DIR)/$(KERNEL_EXE).xclbin'


#######################################################

ifeq ($(TARGET),sw_emu)
RUN_ENV += export XCL_EMULATION_MODE=sw_emu;
EMU_CONFIG = emconfig.json
else ifeq ($(TARGET),hw_emu)
RUN_ENV += export XCL_EMULATION_MODE=hw_emu;
EMU_CONFIG = emconfig.json
else ifeq ($(TARGET),hw)
RUN_ENV += echo "TARGET=hw";
EMU_CONFIG =
endif

emconfig.json :
	emconfigutil --platform $(TARGET_DEVICE) --od $(VTS_DST_DIR)

.PHONY: run run_sw_emu run_hw_emu run_hw check

run_sw_emu:
	make TARGET=sw_emu run

run_hw_emu:
	make TARGET=hw_emu run

run_hw:
	make TARGET=hw run

run: host xclbin $(EMU_CONFIG)
	$(RUN_ENV) \
	cd $(VTS_DST_DIR);	./$(HOST_EXE) $(HOST_ARGS); cd -

run_config: host xclbin_config $(EMU_CONFIG)
	$(RUN_ENV) \
	cd $(VTS_DST_DIR);	./$(HOST_EXE) $(HOST_ARGS) $(call RUN_TIME_INPUT_KERNEL_NAME,1); cd -

.PHONY: host

sw:host

host:  check_TARGET $(HOST_SRC) $(HOST_HDRS)
	mkdir -p $(VTS_DST_DIR)
	$(XCC) $(HOST_SRC) $(HOST_HDRS) $(HOST_CFLAGS) $(HOST_LFLAGS) \
 	-o $(VTS_DST_DIR)/$(HOST_EXE)

$(HOST_SRC):

run_only: $(EMU_CONFIG)
	$(RUN_ENV) \
	cd $(VTS_DST_DIR);	./$(HOST_EXE) $(HOST_ARGS); cd -

run_config_only:  $(EMU_CONFIG)
	$(RUN_ENV) \
	cd $(VTS_DST_DIR);	./$(HOST_EXE) $(HOST_ARGS) $(call RUN_TIME_INPUT_KERNEL_NAME,1); cd -



build:  host xclbin


run_system:  build $(EMU_CONFIG)
	$(RUN_ENV) \
	cd $(VTS_DST_DIR);	./$(HOST_EXE) $(HOST_ARGS); cd -
#########################



generate_vts_config: gen_hls_config_vts $(XO_LIST)
	for n in $(CORE_LIST_NR); do \
	make xo_config_$$n; \
	done; \

##CAREFUL
#This rule need to be called as separate make or with an eval function
xo_config_%: check_TARGET 
	mkdir -p $(VTS_DST_DIR)
	$(VPP) --platform $(TARGET_DEVICE) -t $(TARGET) \
	--jobs $(JOBS) -c --include $(KERNEL_HDRS_CONFIG) \
	-k $(KERNEL_NAME)_$* $(call XO_GEN_FLAGS,$*) \
	$(KERNEL_ADDITIONAL_FLAGS) -o '$(VTS_DST_DIR)/$(KERNEL_EXE)_$*.xo' \
	$(KERNEL_SRC_CONFIG)

xclbin_config: check_TARGET generate_vts_config
	$(VPP) --platform $(TARGET_DEVICE) -t $(TARGET) \
	--link --jobs $(JOBS) $(REPORT) \
	 $(XO_LIST) $(KRNL_LDCLFLAGS_MULTI_CORE) \
	$(KERNEL_ADDITIONAL_FLAGS) \
	-o '$(VTS_DST_DIR)/$(KERNEL_EXE).xclbin'

#rule for only xclbin
xclbin_only: check_TARGET
	$(VPP) --platform $(TARGET_DEVICE) -t $(TARGET) \
	--link --jobs $(JOBS) $(REPORT) \
	$(XO_LIST) $(KRNL_LDCLFLAGS_MULTI_CORE) \
	$(KERNEL_ADDITIONAL_FLAGS) \
	-o '$(VTS_DST_DIR)/$(KERNEL_EXE).xclbin'


$(XO_LIST):

test_config_%:
	@echo ""
	@echo "[INFO] printg some mapping functions"
	@echo $(KRNL_LDCLFLAGS_MULTI_CORE)
	@echo $(call XO_GEN_FLAGS,$*)
	@echo ""


#pythonKRNL_LDCLFLAGS_MULTI_CORE
sw:
	rm -rf $(DPLY_PY)
	mkdir -p $(DPLY_PY)
	cp $(PY_DIR)/$(PY_MI) $(DPLY_PY)/ 
	cp $(PY_DIR)/$(PY_PWL) $(DPLY_PY)/
	cp $(PY_DIR)/$(PY_ONPL) $(DPLY_PY)/
	cp $(SCRIPT_DIR)/python_tester_* $(DPLY_PY)/








curr_status:
	@echo ""
	@echo "*****************************************************************"
	@echo "                      Alveo make status                     "
	@echo "*****************************************************************"
	@echo ""
	@echo " [Help] curently using $(VPP), $(XCC), jobs=$(JOBS)"
	@echo " [Help] Target platform/xsa/dsa= $(TARGET_DEVICE)" 
	@echo " Remember to change it accordingly to the proper version"
	@echo ""
	@echo " [Help] Target for build=$(TARGET)" 
	@echo " possible targets = sw_emu, hw_emu, hw"
	@echo ""
	@echo ""
	@echo " [Help] Working with $(PORT_NR) memory port(s) and $(CORE_NR) core(s)"
	@echo ""
	@echo "*****************************************************************"
	@echo "               END of Alveo make status                     "
	@echo "*****************************************************************"
	@echo ""
	
prepdeploy:
	@echo "[INFO] placeholder REMEMBER!!!!"
########################
helplat: curr_status 
	@echo ""
	@echo "*****************************************************************"
	@echo "" 
	@echo "                      Alveo Specific helper                     "
	@echo ""
	@echo "*****************************************************************"
	@echo ""
	@echo " [INFO] 'make xo' generate the xo for the fixed code"
	@echo " [INFO] 'make xclbin' generate the xclbin for the fixed code"
	@echo ""
	@echo ""
	@echo " [INFO] 'make generate_vts_config' generate the xo for the target core number"
	@echo ""
	@echo ""
	@echo " [INFO] 'make xclbin_config' generate the xclbin using config genrated xo files"
	@echo ""
	@echo ""
	@echo " [INFO] 'make xclbin_only' create the xclbin only withou regenerating the xo, either config or not"
	@echo ""
	@echo ""
	@echo " [INFO] 'make test_config_%' testing make recipe"
	@echo ""
	@echo ""
	@echo "*****************************************************************"
	@echo "" 
	@echo "               END of Alveo Specific helper                     "
	@echo ""
	@echo "*****************************************************************"
	@echo ""
	@make helpparam_alveo
########################

helpparam_alveo: 
	@echo ""
	@echo "*****************************************************************"
	@echo ""
	@echo " [Help] Follow some Makefile Parameter"
	@echo ""
	@echo " [Help] change REPORT_FLAG=<R0|R1|R2> to report detail levels"
	@echo ""
	@echo " [Help] change OPT_LVL=<0|1|2|3|s|quick> to optimization levels"
	@echo ""
	@echo " [Help] change CLK_FRQ=<target_mhz> to ClockID 0 (board) target frequency"
	@echo ""
	@echo " [Help] change KRNL_FRQ=<target_mhz> to ClockID 1 (kernel) target frequency"
	@echo ""
	@echo "*****************************************************************"


cleanvts:
	rm -rf .Xil emconfig.json 

clean_sw_emu: clean
	rm -rf sw_emu
clean_hw_emu: clean
	rm -rf hw_emu
clean_hw: clean
	rm -rf hw

cleanallvts: clean_sw_emu clean_hw_emu clean_hw
	rm -rf _sdx_* xcl_design_wrapper_* _* sdx_*
