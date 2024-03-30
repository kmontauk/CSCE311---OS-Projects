# Compiler to use 
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++17

# Source files
CLIENT_SRC = client.cc calculator.cc
SERVER_SRC = server.cc
CALCULATOR_SRC = calculator.cc calculator.h

# Executables
CLIENT_EXEC = client
SERVER_EXEC = server
#CALCULATOR_EXEC = calculator

all: $(CLIENT_EXEC) $(SERVER_EXEC) $(CALCULATOR_EXEC)

$(CLIENT_EXEC): $(CLIENT_SRC) 
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER_EXEC): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
#$(CALCULATOR_EXEC): $(CALCULATOR_SRC)
#	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(CLIENT_EXEC) $(SERVER_EXEC) $(CALCULATOR_EXEC)