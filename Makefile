# Build all cpp and hpp files in the code directory
# Output the executable into the build directory

# define the C++ compiler to use
CC = g++

# define any compile-time flags
CPPFLAGS = -O0 -g -Wall -std=c++11 -Wno-narrowing
CPPFLAGS += -I/usr/include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LDFLAGS 	 	:= -lstdc++ -lm 
LDLIBS   		:= -L /usr/lib

SRC_DIR 	:= ./code
OBJ_DIR 	:= ./build
SRC_FILES   := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES    := $(wildcard $(SRC_DIR)/*.hpp)
OBJ_FILES   := $(SRC_FILES:.cpp=.o)

# define the executable file 
TARGET = skogsnet

all: dirs $(TARGET) raylibfrontend 
	@echo
	@echo        The Serial Temperature and Humidity reader program has been built and runned successfully!
	@echo 

$(TARGET): $(OBJ_FILES)
	@echo
	@echo        Building the program
	$(CC) $(CPPFLAGS) -o ./build/$(TARGET) $(SRC_FILES) $(LDLIBS) $(LDFLAGS)
	./build/$(TARGET)
	mv ./code/*.o ./build/

raylibfrontend: dirs
	mkdir -p ./raylib_frontend/build
	@echo
	@echo		Building the and running raylib frontend
	$(CC) $(CPPFLAGS) -o ./raylib_frontend/build/raylibfrontend ./raylib_frontend/frontend.cpp $(LDLIBS) $(LDFLAGS) -lraylib -lm -lpthread
	./raylib_frontend/build/raylibfrontend

dirs:
	@echo
	@echo        Creating directories
	mkdir -p build

clean:
	@echo
	@echo        Cleaning
	rm -rf build 2> /dev/null || true
	rm -rf code/*.o 2> /dev/null || true

	rm -rf ./raylib_frontend/*.o 2> /dev/null || true
	rm -rf ./raylib_frontend/build 2> /dev/null || true
	@echo
	@echo        Taking a backup of the old results
	cp output.dat output_old.dat 2> /dev/null || true
	cp ./build/output.dat ./output.dat 2> /dev/null || true
	
run:
	@echo
	@echo        Running the executable
	./build/$(TARGET)

