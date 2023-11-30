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
