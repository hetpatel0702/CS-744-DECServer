#include "funtion_declarations.h"

void error(const char *msg) 
{
	perror(msg);
}

void filesize(FILE *fp, int newsockfd)
{
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fclose(fp);
    write(newsockfd, &file_size, sizeof(file_size));
}

void receive_enqueue(int newsockfd, int reqID) 
{
	struct receiveQueue *node = new struct receiveQueue;
	node->sockfd = newsockfd;
	node->requestid=reqID;
	node->next = NULL;
	
	if (r_front == NULL) 
	{
		r_front = r_rear = node;
		pthread_cond_signal(&receive_cond);
	} 
	else
	{
		r_rear->next = node;
		r_rear = node;
	}
}

struct receiveQueue* receive_dequeue()  
{  
	pthread_mutex_lock(&receive_queue_mutex);

    struct receiveQueue *temp;   
    while(r_front == NULL)
    {
		pthread_cond_wait(&receive_cond,&receive_queue_mutex);  
    }  
    temp = r_front;
	if(r_front == r_rear)  
    {  
        r_front=r_rear=NULL;   
    }  
    else  
    {  
        r_front = r_front->next;  
        r_rear->next = r_front;   
    }  

	pthread_mutex_unlock(&receive_queue_mutex);
	return temp;
}

void status_enqueue(int sockfd,int reqID) 
{
	struct statusq *node = new struct statusq;
	node->requestid=reqID;
	node->sockfd = sockfd;
	node->next = NULL;
	if (s_front == NULL) 
	{
		s_front = s_rear = node;
		pthread_cond_signal(&status_cond);
	} 
	else
	{
		s_rear->next = node;
		s_rear = node;
	}
}

struct statusq* status_dequeue()  
{  
	pthread_mutex_lock(&status_queue_mutex);

    struct statusq *temp;   
	
    while(s_front == NULL)
    {  
		pthread_cond_wait(&status_cond,&status_queue_mutex);
    }  
	
    temp = s_front;  
	if(s_front==s_rear)  
    {  
        s_front=s_rear=NULL;  
    }  
    else  
    {  
        s_front=s_front->next;  
        s_rear->next=s_front;  
    }  
   
	pthread_mutex_unlock(&status_queue_mutex);
	return temp;
}

void process_enqueue(int sockfd,int reqID) 
{
	struct processq *node = new struct processq;
	node->requestid=reqID;
	node->sockfd = sockfd;
	node->next = NULL;
	if (p_front == NULL) 
	{
		p_front = p_rear = node;
		pthread_cond_signal(&process_cond);
	} 
	else
	{
		p_rear->next = node;
		p_rear = node;
	}
}

struct processq* process_dequeue()  
{  
	pthread_mutex_lock(&process_queue_mutex);

    struct processq *temp;   

    while(p_front == NULL)
    {  
		pthread_cond_wait(&process_cond,&process_queue_mutex);
    }  
    temp = p_front;  
	if(p_front==p_rear)  
    {  
        p_front=p_rear=NULL;  
    }  
    else  
    {  
        p_front=p_front->next;  
        p_rear->next=p_front;   
    }  
	pthread_mutex_unlock(&process_queue_mutex);
	return temp;
}

void sresult(int newsockfd, int fp, int a, char *buffer) 
{
    memset(buffer, 0, BUFFER_SIZE);
    int x, n = 0;
    
    if(a == 0)
    {
        strcat(buffer, "\nCOMPILE ERROR\n\n");
        x = strlen("\nCOMPILE ERROR\n\n");
    } 
    else if(a == 1) 
    {
        n = send(newsockfd, "\nRUNTIME ERROR\n\n", strlen("\nRUNTIME ERROR\n\n"), 0);
        if (n < 0)
            error("Send failed");
        return;
    } 
    else if(a == 2)
    {
        strcat(buffer, "\nOUTPUT ERROR \n\n");
        x = strlen("\nOUTPUT ERROR \n\n");
    }
	else
    {
        n = send(newsockfd, "PASS\n", strlen("PASS\n"), 0);
        if (n < 0)
            error("Send failed");
        return;
    } 
    int k, flag = 0;
    while (true) 
    {
        if (flag)
            x = 0;
        k = read(fp, buffer + x, BUFFER_SIZE - x);

        flag = 1;
        n = send(newsockfd, buffer, k + x, 0);
        if (n < 0)
            error("Send failed");
        if (k <= 0)
            break;

        memset(buffer, 0, BUFFER_SIZE);
    }
}

int generateUniqueID()
{
	auto currentTime = chrono::system_clock::now();
	auto durationSinceEpoch = currentTime.time_since_epoch();
	auto secondsSinceEpoch = chrono::duration_cast<chrono::seconds>(durationSinceEpoch);

	time_t timeInSeconds = secondsSinceEpoch.count();
	return timeInSeconds;
}

