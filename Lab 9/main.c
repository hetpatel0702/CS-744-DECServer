#include "funtion_declarations.h"
#include "handle_client.c"
#include "funtion_definitions.c"

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qempty = PTHREAD_COND_INITIALIZER;
int qsize = 0;
int front = -1;
int rear = -1;

int main(int argc, char *argv[]) 
{
	int sockfd, //the listen socket descriptor (half-socket)
	portno; //port number at which server listens

	socklen_t clilen; //a type of an integer for holding length of the socket address

	struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
	int n;

	if (argc < 2) {
	fprintf(stderr, "ERROR, no port provided\n");
	exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 

	if (sockfd < 0)
	error("ERROR opening socket");

	bzero((char *)&serv_addr, sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = INADDR_ANY;   

	portno = atoi(argv[1]);
	serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	listen(sockfd, 1000); 

	clilen = sizeof(cli_addr);  

	int thread_pool=atoi(argv[2]);

	pthread_t thread[thread_pool];
    for(int i=0;i<thread_pool;i++)
	{
		if(pthread_create(&thread[i], NULL, gradeTheFile, NULL) != 0)
		{
			fprintf(stderr,"Error Creating Thread\n");
		}
	}
	
	pthread_t worker_qsize;
	if(pthread_create(&worker_qsize, NULL, measure_queue_size, NULL) != 0)
	{
		fprintf(stderr,"Error Creating Thread\n");
	}

	while (1)
	{
		int *newsockfd = (int *)malloc(sizeof(int));
		if (newsockfd == NULL) {
    		fprintf(stderr, "Error: Unable to allocate memory for newsockfd\n");
			continue;
		}
		*newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

		if (*newsockfd < 0) {
			fprintf(stderr, "Error: accept failed\n");
			free(newsockfd); 
			continue;
    	}
		enqueue(*newsockfd);		
	}

	return 0;
}
