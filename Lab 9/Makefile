all: server client

server: main.o handle_client.o function_definitions.o
	gcc main.o handle_client.o function_definitions.o -o server -lpthread 

main.o: main.c function_declarations.h
	gcc -c main.c -o main.o 

handle_client.o: handle_client.c function_declarations.h
	gcc -c handle_client.c -o handle_client.o  

function_definitions.o: function_definitions.c function_declarations.h
	gcc -c function_definitions.c -o function_definitions.o 

client: gradingclient.c
	gcc gradingclient.c -o client -lpthread

clean:
	rm server client *.o
