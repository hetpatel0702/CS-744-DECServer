/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>
#include<fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include<stdbool.h>
 //buffer for reading and writing the messages
#define BUFFER_SIZE 128
#define MAX_QUEUE_SIZE 1000

pthread_mutex_t qmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qempty=PTHREAD_COND_INITIALIZER;
int queue[MAX_QUEUE_SIZE];
int qsize=0;
int front=-1,rear = -1;

void error(char *msg) {
  perror(msg);
//   pthread_exit(NULL);
}
int isFull() 
{
	if ((front == rear + 1) || (front == 0 && rear == MAX_QUEUE_SIZE - 1)) return 1;
	return 0;
}

int isEmpty()
{
	if (front == -1) return 1;
	return 0;
}
void enqueue(int newsockfd) 
{
    pthread_mutex_lock(&qmutex);
    if (isFull())
    	printf("\n Queue is full!! \n");
  	else
	{
		if (front == -1) 
			front = 0;
		rear = (rear + 1) % MAX_QUEUE_SIZE;
		queue[rear] = newsockfd;
		qsize=qsize+1;
		pthread_cond_signal(&qempty);
  	}
    pthread_mutex_unlock(&qmutex);
}

int dequeue()
{
	int sockfd;
    pthread_mutex_lock(&qmutex);
  	while(isEmpty()) 
	{
		pthread_cond_wait(&qempty,&qmutex);
	}
   
	sockfd= queue[front];
	if (front == rear) 
	{
		front = -1;
		rear = -1;
	} 
	else 
	{
		front = (front + 1) % MAX_QUEUE_SIZE;
	}
	qsize=qsize-1;

    pthread_mutex_unlock(&qmutex);
    return sockfd;
   
}

void filesize(FILE *fp,int newsockfd)
{
	fseek(fp,0,SEEK_END);
	int file_size = ftell(fp);
	fclose(fp);
	write(newsockfd,&file_size,sizeof(file_size));
}
void sresult(int newsockfd,int fp,int a,char * buffer)
{
	bzero(buffer,BUFFER_SIZE);
	int x,n=0;
	if(fp==-1)
	{
		n = send(newsockfd,"PASS\n",strlen("PASS\n"),0);
		if(n < 0)
			error("Send failed");
		return;
	}
	else if(a==0)
	{
		strcat(buffer,"\nCOMPILE ERROR\n\n");
		x = strlen("\nCOMPILE ERROR\n\n");
	}
	else if(a==1)
	{
		strcat(buffer,"\nRUNTIME ERROR\n\n");
		x = strlen("\nRUNTIME ERROR\n\n");
	}
	else
	{
		strcat(buffer,"\nOUTPUT ERROR \n\n");
		x = strlen("\nOUTPUT ERROR \n\n");
	}
	int k,flag=0;
	while(1)
	{
		if(flag)
			x = 0;
		k = read(fp,buffer + x,BUFFER_SIZE-x);

		flag=1;
		n = send(newsockfd,buffer,k+x,0);
		if(n < 0)
			error("Send failed");
		if(k <= 0)
			break;
		
		bzero(buffer,BUFFER_SIZE);
	}
}

void *measure_queue_size()
{
	while(1)
	{
		printf("Queue size is:%d\n",qsize);
		sleep(1);
	}
}

void *gradeTheFile(void *f)
{
	while(1)
	{
		int newsockfd=dequeue();
		if (newsockfd < 0) {
			error("ERROR on accept");
		}

		pthread_t threadID = pthread_self();		
		char buffer[BUFFER_SIZE];
		char Cerror_file[50];
		char Rerror_file[50];
		char output_file[50];
		char diff_file[50];
		char grade_file[50];
		char grade_file_exe[50];

		char compile_command[150];
		char run_command[150];
		char diff_command[150];
		char delete_files[150];

		sprintf(output_file,"output%lu.txt",threadID);
		sprintf(Rerror_file,"Rerror%lu.txt",threadID);
		sprintf(Cerror_file,"Cerror%lu.txt",threadID);
		sprintf(diff_file,"diff%lu.txt",threadID);
		sprintf(grade_file,"gradeFile%lu.c",threadID);
		sprintf(grade_file_exe,"file%lu",threadID);



		// bzero(buffer, BUFFER_SIZE); //set buffer to zero
		
					
		int file_size = 0;
		int n = read(newsockfd,&file_size,sizeof(file_size));
		if (n < 0)
			error("ERROR reading from socket");

		int newGradeFd = open(grade_file,O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
		if (newGradeFd < 0)
			error("ERROR opening file");

		bzero(buffer,BUFFER_SIZE);
		while (file_size > 0)
		{	
			int readBytes = read(newsockfd, buffer, BUFFER_SIZE);
			if (readBytes < 0)
				error("ERROR reading from socket");

			int wroteBytes = write(newGradeFd,buffer,readBytes);
			if (wroteBytes < 0)
				error("ERROR writing to file");

			file_size -= readBytes;

			bzero(buffer,BUFFER_SIZE);
		}
		close(newGradeFd);

		sprintf(compile_command,"gcc gradeFile%lu.c -o file%lu 2>Cerror%lu.txt",threadID,threadID,threadID);
		int compiling = system(compile_command);
		
		if(compiling != 0)
		{
			
			// char err[40] = "COMPILE ERROR\n";
			FILE* fp = fopen(Cerror_file,"rb");
			filesize(fp,newsockfd);
			
			int cerror = open(Cerror_file,O_RDONLY);
			if (cerror < 0)
				error("ERROR opening file");

			bzero(buffer,BUFFER_SIZE);
			sresult(newsockfd,cerror,0,buffer);
			
			sprintf(delete_files,"rm gradeFile%lu.c Cerror%lu.txt",threadID,threadID);
			system(delete_files);
			close(cerror);
		}
		else
		{
			sprintf(run_command,"./file%lu >output%lu.txt 2>Rerror%lu.txt",threadID,threadID,threadID);
			int runTheFile = system(run_command);

			if(runTheFile != 0)
			{
				FILE* fp = fopen(Rerror_file,"rb");
				filesize(fp,newsockfd);
							
				int rerror = open(Rerror_file,O_RDONLY);
				if(rerror < 0)
					error("ERROR opening file");

				bzero(buffer,BUFFER_SIZE);
				sresult(newsockfd,rerror,1,buffer);
				close(rerror);
			}
			else
			{
				sprintf(diff_command,"echo -n '1 2 3 4 5 6 7 8 9 10 ' | diff - output%lu.txt > diff%lu.txt",threadID,threadID);
				int difference = system(diff_command);
				if(difference != 0)
				{
					FILE* fp = fopen(diff_file,"rb");
					filesize(fp,newsockfd);
					
					int diffFd = open(diff_file,O_RDONLY);
					if(diffFd < 0)
						error("ERROR opening file");

					bzero(buffer,BUFFER_SIZE);
					sresult(newsockfd,diffFd,2,buffer);
					close(diffFd);
				}
				else
				{
					int x=0;
					write(newsockfd,&x,sizeof(x));
					sresult(newsockfd,-1,-1,buffer);
				}
				sprintf(delete_files,"rm diff%lu.txt",threadID);
				system(delete_files);
			}
			sprintf(delete_files,"rm gradeFile%lu.c file%lu output%lu.txt Cerror%lu.txt Rerror%lu.txt",threadID,threadID,threadID,threadID,threadID);
			system(delete_files);
		}
		close(newsockfd);
	}
}

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

	/* create socket */

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

	listen(sockfd, 3000); 


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

