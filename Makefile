# Define the ROOTSYS environment variable if not already set
# This points to your ROOT installation directory
ROOTSYS = $(shell root-config --prefix)

# Define the C++ compiler
CXX = g++

# Define compiler flags
# -g for debugging, -Wall for warnings, -fPIC for position-independent code (often needed for shared libraries)
# $(shell root-config --cflags) gets ROOT-specific compiler flags
CXXFLAGS = -g -Wall -fPIC -std=c++17 $(shell root-config --cflags)
CXXFLAGS += -I../../src -I./include/
# Define linker flags
# $(shell root-config --glibs) gets ROOT-specific linker flags (libraries)
LDFLAGS = $(shell root-config --glibs)
LIBS = -lCombineHarvesterCombineTools 
LIBPATH = -L../../lib/el9_amd64_gcc12/

#Define src and objects
SRC_DIR = src
INC_DIR = include
OBJS_DIR = obj
BIN_DIR = .
CPP_SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/SampleTool.cpp $(SRC_DIR)/BuildFitInput.cpp $(SRC_DIR)/JSONFactory.cpp $(SRC_DIR)/ConfigParser.cpp $(SRC_DIR)/ArgumentParser.cpp
CMSSW_CPP_SRCS = $(SRC_DIR)/BFmain.cpp $(SRC_DIR)/BuildFit.cpp $(SRC_DIR)/JSONFactory.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(CPP_SRCS))
CMSSWOBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(CMSSW_CPP_SRCS))

# Define the executable name
TARGET = $(BIN_DIR)/BFI.x
CMSSWTARGET = $(BIN_DIR)/BF.x

# Define source files
#SRCS = main.cpp SampleTool.cpp BuildFitInput.cpp BuildFitTools.h JSONFactory.cpp
#CMSSWSRCS = BFmain.cpp BuildFit.cpp JSONFactory.cpp BuildFitTools.h

# Define object files
#OBJS = $(SRCS:.cpp=.o)
#CMSSWOBJS = $(CMSSWSRCS:.cpp=.o)


# Default target: build the executable
all: $(TARGET)
cmssw: $(CMSSWTARGET)

# Create object directory if it doesn't exist
$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

# Rule to build the executable from object files
$(TARGET): $(OBJS_DIR) $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(CMSSWTARGET): $(OBJS_DIR) $(CMSSWOBJS) 
	$(CXX) $(CMSSWOBJS) $(LDFLAGS) $(LIBPATH) $(LIBS) -o $@

# Rule to compile C++ source files into object files
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Clean target: remove generated files, this deleted my headers!
clean:
	rm ./obj/*.o BFI.x BF.x
