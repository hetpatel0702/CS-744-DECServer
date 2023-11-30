all: server client

server: main.o handle_client.o function_definitions.o
	g++ main.o handle_client.o function_definitions.o -o server -lpthread 

main.o: main.cpp function_declarations.h
	g++ -c main.cpp -o main.o 

handle_client.o: handle_client.cpp function_declarations.h
	g++ -c handle_client.cpp -o handle_client.o  

function_definitions.o: function_definitions.cpp function_declarations.h
	g++ -c function_definitions.cpp -o function_definitions.o 

client: gradingclient.cpp
	g++ gradingclient.cpp -o client -lpthread

clean:
	rm server client *.o
