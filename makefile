# Build all cpp and hpp files in the code directory
# Output the executable into the build directory

# define the C++ compiler to use
CC = clang

# define any compile-time flags
CPPFLAGS = -Wall -g -std=c++11 -fopenmp
CPPFLAGS += -I/usr/include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LDFLAGS 	 	:= -fopenmp -lstdc++ -lm
LDLIBS   		:= -L /usr/lib

SRC_DIR 	:= ./code
OBJ_DIR 	:= ./build
SRC_FILES   := $(wildcard $(SRC_DIR)/*.cpp)
INCLUDES    := $(wildcard $(SRC_DIR)/*.hpp)
OBJ_FILES   := $(SRC_FILES:.cpp=.o)

# define the executable file 
TARGET = skogsnet_v0.0.3

all: dirs raylibfrontend $(TARGET) 
	@echo
	@echo        The Serial Temperature and Humidity reader program has been built and runned successfully!
	@echo 

$(TARGET): $(OBJ_FILES)
	@echo
	@echo        Building the program
	$(CC) $(CPPFLAGS) -o ./build/$(TARGET) $(SRC_FILES) $(LDLIBS) $(LDFLAGS)
	./build/$(TARGET)
	mv ./code/*.o ./build/

raylibfrontend:
	@echo
	@echo		Building and running the raylib frontend
	./raylib_frontend/build.sh&

dirs:
	@echo
	@echo        Creating directories
	mkdir -p build

clean:
	@echo
	@echo        Cleaning
	rm graph.png 2> /dev/null || true
	rm -r FrontendVirtualEnvironment/* 2> /dev/null || true
	rm -r build/* 2> /dev/null || true
	rm -r code/*.o 2> /dev/null || true
	@echo
	@echo        Taking a backup of the old results
	cp ./build/output.dat output_old.dat 2> /dev/null || true
	
run:
	@echo
	@echo        Running the executable
	./build/$(TARGET)

