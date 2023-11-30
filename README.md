# CS-744-DECServer

[**Github Repository link**](https://github.com/hetpatel0702/CS-744-DECServer.git)

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

## Version 2 (Multithreaded Server with Create-Destroy threads)

- **Performance Testing for version 2**

	- For Compiling the server

			g++ gradingserver.c -o server
			
	- For Compiling the client

			g++ gradingclient.cpp -o client

	- Run the server

			./server <port_number> (port number set in the analysis.sh is 7000)

	- Run the analysis.sh

			bash analysis.sh
			
		Script will generate the plot of response time, throughput, timeout rate, request rate, nlwp, goodput data, 		error rate and cpu utilization in their respective .png files


## Version 3 (Multithreaded Server with Thread pool)

- **Performance Testing for version 3**

	- For Compiling the server

			g++ main.c -o server
			
	- For Compiling the client

			g++ gradingclient.cpp -o client

	- Run the server

			./server <port_number> <thread_pool_size> (port number set in the analysis.sh is 7000)

	- Run the analysis.sh

			bash analysis.sh

		Script will generate the plot of response time, throughput, timeout rate, request rate, nlwp, goodput data, 		error rate and cpu utilization in their respective .png files

## Version 4 (Asynchronous Server)

- **Performance Testing for version 4**

	- For compiling client and server
			
			make

	- Run the server

			./server <port_number> <thread_pool_size> (port number set in the analysis.sh is 7000)

	- Run the analysis.sh

			bash analysis.sh

		Script will generate the plot of response time, throughput, timeout rate, request rate, nlwp, goodput data, 		error rate and cpu utilization in their respective .png files

