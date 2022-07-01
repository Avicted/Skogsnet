# Build all cpp and hpp files in the code directory
# Output the executable into the build directory

# define the C++ compiler to use
CC = g++

# define any compile-time flags
CFLAGS = -std=c++23 -g -O3 -Wall -Wextra -Wsystem-headers -isystem code

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS 	 	:= -lm # -fopenmp  -lnoise
LIBRARIES   := -L /usr/lib

SRC_DIR 	:= ./code
OBJ_DIR 	:= ./build
SRC_FILES   := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES    := $(wildcard $(SRC_DIR)/*.hpp)
OBJ_FILES   := $(SRC_FILES:.cpp=.o)

# define the executable file 
TARGET = skogsnet_v0.0.1

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean


all: clean dirs $(TARGET)
	@echo
	@echo        The Serial Temperature and Humidity reader program has been built and runned successfully!

$(TARGET): $(OBJ_FILES)
	@echo
	@echo        Building the program
	$(CC) $(CFLAGS) -o ./build/$(TARGET) $(SRC_FILES) $(LFLAGS) $(LIBRARIES)
	./build/$(TARGET)
	mv ./code/*.o ./build/

dirs:
	@echo
	@echo        Creating directories
	mkdir -p build

clean:
	@echo
	@echo        Cleaning
	rm -r build 2> /dev/null || true
	rm -r code/*.o 2> /dev/null || true
	
run:
	@echo
	@echo        Running the executable
	./build/$(TARGET)

depend: $(SRC_FILES)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it

