#Run the different makefiles

all: main

clean: main_clean

main:
	make -C ./gpsp

main_clean:
	make -C ./gpsp clean