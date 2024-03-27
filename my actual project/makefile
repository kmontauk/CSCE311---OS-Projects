# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# Source files
CLIENT_SRC = client.cc
SERVER_SRC = server.cc

# Executables
CLIENT_EXEC = client
SERVER_EXEC = server

$(CLIENT_EXEC): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SERVER_EXEC): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^
