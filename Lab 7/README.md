Performance Testing for version 2 (single threaded server)
--->For Compiling the server 
	use : g++ gradingserver.c -o server

--->For Compiling the client 
	use : g++ gradingclient.cpp -o client

--->Run the server
	using : ./server <port_number>  (port number set in the script is 7000)

4. Run analysis.sh: bash analysis.sh <ipaddress> <server_port> <grading_file.c> <loopNum> <sleeptime>


The resultant plot will be stored in the respective .png files of response graph and throughput graph


