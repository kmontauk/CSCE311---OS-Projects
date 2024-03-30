# Compiler to use 
CXX = g++

# Output directory
OUTPUT = bin

# Compiler flags
CXXFLAGS = -Wall -std=c++17 -Iinc

# Source files
CLIENT_SRC = src/client.cc src/calculator.cc
SERVER_SRC = src/server.cc 
CALCULATOR_SRC = src/calculator.cc

# Header files
CLIENT_HEADERS = inc/client.h inc/calculator.h
SERVER_HEADERS = inc/server.h
CALCULATOR_HEADERS = inc/calculator.h

# Executables
CLIENT_EXEC = $(OUTPUT)/client
SERVER_EXEC = $(OUTPUT)/server
CALCULATOR_EXEC = $(OUTPUT)/calculator

all: $(CLIENT_EXEC) $(SERVER_EXEC)
client: $(CLIENT_EXEC)
server: $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_SRC) 
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER_EXEC): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
#$(CALCULATOR_EXEC): $(CALCULATOR_SRC)
#	$(CXX) $(CXXFLAGS) -o $@ $^



clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(CALCULATOR_EXEC)