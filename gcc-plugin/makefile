# The gcc used to compile our plugin (consider that is written in C++!)
HOST_GCC = g++

# The gcc that will support our plugin 
TARGET_GCC = gcc

# The name of the file we want to compile 
PLUGIN_SOURCE_FILES = main.cpp

# Plugins directory of GCC
GCC_PLUGINS_DIR = $(shell $(TARGET_GCC) -print-file-name=plugin)

# GCC CPP flags (we need position independent-code and run-time type info support disabled)
CXXFLAGS+= -I$(GCC_PLUGINS_DIR)/include -fPIC -fno-rtti

# our recipes
.PHONY: all clean test

all: bin/uPrint.so

bin/uPrint.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

install: bin/uPrint.so
	sudo cp $(shell pwd)/$^ $(GCC_PLUGINS_DIR)/uPrint.so

test:
	gcc test.c -fplugin=uPrint -o bin
	- ./bin
	@rm bin

clean: 
	rm -f bin/*