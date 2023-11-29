/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<fcntl.h>
#include <netdb.h>
#include<sys/time.h>
#include<math.h>

void error(char *msg) {
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[]) 
{
	int sockfd, portno, n;

	struct timeval tv;
	struct sockaddr_in serv_addr; 
	struct hostent *server; 

	char buffer[10000]; //buffer for message

	if (argc < 4) {
	fprintf(stderr, "usage %s hostname port\n", argv[0]);
	exit(0);
	}

	portno = atoi(argv[2]);
	char *sourceFile = argv[3]; 
	int loopNum = atoi(argv[4]);
	int sleepTime = atoi(argv[5]);
	double start,end;
	double avgTime = 0;
	double Tsend = 0,Trcv=0;
	int successfulRes=0;
	if(gettimeofday(&tv,NULL) == 0){
		start = (double)tv.tv_sec + tv.tv_usec / 1000000.0;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd < 0)
		error("ERROR opening socket");

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

	serv_addr.sin_family = AF_INET; // Address Family is IP

	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);

	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");

	int loopNum2 = loopNum;
	while(loopNum--)
	{  
		bzero(buffer, 10000); 

		if(loopNum > 0)
		{
			strcat(buffer,"NDONE");
		}
		else
		{
			strcat(buffer,"DONE");
		}
		
		int gradeFd = open(sourceFile,O_RDONLY);
		int readBytes = read(gradeFd,buffer+6,sizeof(buffer)-6);
		
		if(gettimeofday(&tv,NULL) == 0){
			Tsend = (double)tv.tv_sec + tv.tv_usec / 1000000.0;
		}

		n = write(sockfd, buffer, readBytes+6);

		if (n < 0)
			error("ERROR writing to socket");
		bzero(buffer, 10000);

		n = read(sockfd, buffer, sizeof(buffer));

		if(n > 0)
			successfulRes++;

		printf("%s\n", buffer);

		if(gettimeofday(&tv,NULL) == 0){
			Trcv = (double)tv.tv_sec + tv.tv_usec/1000000.0;
		}
		avgTime += (Trcv - Tsend);
		printf("Response Time: %lf\n",Trcv - Tsend);
		sleep(sleepTime);

		if (n < 0)
			error("ERROR reading from socket");
	}

	printf("\nAverage Response Time:%lf\n",avgTime/loopNum2);

	if(gettimeofday(&tv,NULL) == 0){
	end = (double)tv.tv_sec + tv.tv_usec / 1000000.0;
	}

	close(sockfd);
	printf("Successful Responses:%d\n",successfulRes);
	printf("Total Time:%lf\n",end-start);
	printf("Throughput:%lf\n",successfulRes/avgTime);

	return 0;
}
