# Compilador y opciones
CXX = g++
CXXFLAGS = -Wall -Isrc -g -fdiagnostics-color=always
LDFLAGS = -lssl -lcrypto

# Directorios
SRC_DIR = src
BIN_DIR = bin

# Archivos fuente y objetos
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(SRCS))

# Nombre del ejecutable
EXECUTABLE = $(BIN_DIR)/httpserver.out

# Regla por defecto
all: $(EXECUTABLE)

# Regla de compilación para objetos
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para el ejecutable
$(EXECUTABLE): $(OBJS) app.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) app.cpp -o $(EXECUTABLE) $(LDFLAGS)

# Regla para limpiar archivos generados
clean:
	rm -f $(OBJS)

.PHONY: all clean
