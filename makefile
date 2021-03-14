#Run the different makefiles

all: dvemgr main

clean: dvemgr_clean main_clean

dvemgr:
	make -C ./prx/dvemgr
main:
	make -C ./gpsp

dvemgr_clean:
	make -C ./prx/dvemgr clean
main_clean:
	make -C ./gpsp clean