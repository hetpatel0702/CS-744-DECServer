#include "funtion_declarations.h"

void error(char *msg)
{
  perror(msg);
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
