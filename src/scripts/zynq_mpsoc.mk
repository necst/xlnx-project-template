ifndef KERNEL
	KERNEL = drambenchmark_top
endif
BOARD_BITSTREAM=${KERNEL}_wrapper.bit
CLK_NAME?=fclk0_mhz
CLK_FREQ?=100
PREV_FREQ=$(shell sudo python3 -c "from pynq.ps import Clocks; import sys; sys.stdout.write(str(Clocks.$(CLK_NAME)))")
APPLICATION_NAME=drambenchmark
CXX=g++
CFLAGS= -D"ULTRA" -std=c++11
LFLAGS= -lcma -pthread

compile_host:
	@echo "Add -D\"AVERAGE_REPS\" if you would like to execute multiple times"
	$(CXX) $(CFLAGS) *.cpp $(LFLAGS) -o $(APPLICATION_NAME)

$(APPLICATION_NAME):compile_host

run: $(APPLICATION_NAME) 
	sudo ./$(APPLICATION_NAME)

load_bitfile:
	@echo "Downloading bitstream named $(BOARD_BITSTREAM)"
	sudo python3 -c "from pynq import Overlay; o = Overlay('$(BOARD_BITSTREAM)'); o.download()"

set_clk:
	@echo "Prev frequency was $(PREV_FREQ)"
	@echo "$(PREV_FREQ)"
	@echo "Setting frequency to $(CLK_FREQ)"
	sudo python3 -c "from pynq.ps import Clocks; Clocks.$(CLK_NAME) = $(CLK_FREQ); print(Clocks.$(CLK_NAME))"