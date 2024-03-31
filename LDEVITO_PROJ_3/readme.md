Copyright D. "L." Devito
This project consists of 3 source files, calculate.cc, client.cc, and server.cc, located in ./src, and their 3 respective headers, in ./inc.

From the root directory, the makefile can be found and can be used to create executables by:
make bin/client.cc
or 
make client.cc
or
make

The client and server can be executed using 

./bin/client <file_path> <integer_representing_number_of_lines_in_file>

./bin/server

This project allows a client, implemented in client.cc, to communicate with a server, implemented in server.cc, using shared memory.

The client sends a file path and number of lines requested to the server via shared memory, which the server reads from. The server then uses that information to open the file, processes the lines of that file such that they are translated to a delineated string, and loads those lines into shared memory. Finally, the client reads from the shared memory and proceeds to create four threads. These 4 threads call on the functions defined within client.h and calculator.h header files to convert the individual lines into string vectors so that they can be passed to calculator.cc::calculate(). Each thread processes 1/4th of the lines in a round robin ordering.

Calculate() then uses these string vectors to run calculations on the infix notation found within, returning the value to the client.


client.cc manages creation and destruction of shared memory, while server.cc handles creation and destruction of semaphores. Buffer size and shared memory structure are defined in the header files. 

