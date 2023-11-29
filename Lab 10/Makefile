all: server client

server: main.cpp handle_client.cpp function_definitions.cpp function_declarations.h
	g++ main.cpp -o server -lpthread

client: gradingclient.cpp
	g++ gradingclient.cpp -o client -lpthread

clean:
	rm server client
