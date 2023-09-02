CXX = g++
CXXFLAGS_COMMON = -Isrc 
LDFLAGS = -lssl -lcrypto

SRC_DIR = src
BIN_DIR = bin

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRCS))

DEBUG_CXXFLAGS = -Wall -g -Og -fdiagnostics-color=always
RELEASE_CXXFLAGS = -O3

EXECUTABLE = $(BIN_DIR)/httpserver.out

all: debug

debug: CXXFLAGS = $(DEBUG_CXXFLAGS) $(CXXFLAGS_COMMON)
debug: $(EXECUTABLE)

release: CXXFLAGS = $(RELEASE_CXXFLAGS) $(CXXFLAGS_COMMON)
release: $(EXECUTABLE)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJS) app.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) app.cpp -o $(EXECUTABLE) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(EXECUTABLE)

.PHONY: all debug release clean
