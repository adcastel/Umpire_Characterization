# Umpire installation
HOME_UMPIRE = /mnt/beegfs/uji/adcastel/opt/umpire

# Compiler
CXX = g++

# Flags
CXXFLAGS = -O3 -march=native
INCLUDES = -I$(HOME_UMPIRE)/include/

# Libraries
LIBS = $(HOME_UMPIRE)/lib/libumpire.a \
       $(HOME_UMPIRE)/lib/libcamp.a \
       -lnuma -lrt -pthread

# Target
TARGET = bw_lat

# Sources
SRC = bw_lat_bench.cpp
OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET) $(TEST) $(SRC_NODE) $(DST_NODE) $(BYTES) $(REPS)
