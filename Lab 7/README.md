## Version 1 (Single Threaded Server)

- **Performance Testing for Version 1**

	- Compile Server

			g++ gradingserver.c -o server -lpthread
			
	- Compile Client

			g++ gradingclient.cpp -o client -lpthread

	- Run Server

			./server <port_number>

	- Run analysis.sh 
	
			bash analysis.sh <ipaddress> <server_port> <file> <loopNum> <sleeptime>

- The resultant plots will be stored in the respective .png files of response graph and throughput graph.
