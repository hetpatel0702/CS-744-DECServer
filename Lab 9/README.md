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
