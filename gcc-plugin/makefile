# The gcc used to compile our plugin (consider that is written in C++!)
HOST_GCC = g++

# The gcc that will support our plugin 
TARGET_GCC = gcc

# The name of the file we want to compile 
PLUGIN_SOURCE_FILES = main.cpp

LIBRARY_SOURCE_FILES = libuPrint.c

# Plugins directory of GCC
GCC_PLUGINS_DIR = $(shell $(TARGET_GCC) -print-file-name=plugin)

# GCC CPP flags (we need position independent-code and run-time type info support disabled)
CXXFLAGS+= -I$(GCC_PLUGINS_DIR)/include -fPIC -fno-rtti

# our recipes
.PHONY: all clean test

all: bin/uPrint.so bin/libuPrint.o

bin/uPrint.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

install: bin/uPrint.so
	sudo cp $(shell pwd)/$^ $(GCC_PLUGINS_DIR)/uPrint.so

bin/libuPrint.o: $(LIBRARY_SOURCE_FILES)
	gcc -c -Wall -Werror -fpic $^ -o bin/libuPrint.o
	ar rcs bin/libuPrint.a bin/libuPrint.o
	rm -f bin/libuPrint.o

test:
	rm -f uPrint_db.csv
	gcc test.c test1.c -Lbin -fplugin=uPrint -luPrint -o bin/test
	./bin/test

clean: 
	rm -f bin/*