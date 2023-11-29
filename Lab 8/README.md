Performance Testing for version 2 (Create-Destroy Thread)

--->For Compiling the server 
	use : g++ gradingserver.c -o server

--->For Compiling the client 
	use : g++ gradingclient.cpp -o client

--->Run the server
	using : ./server <port_number>  (port number set in the script is 7000)

--->Run	the analysis.sh
	use : bash analysis.sh 

The results of the script will the plot of response_time, throughput, timeout_rate, request_rate, nlwp, goodput_data, error_rate, cpu_utilization in their respective .png files
