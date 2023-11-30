#include "function_declarations.h"

// function to handle errors
void error(const char *msg) 
{
	perror(msg);
}

// function to write file size
void filesize(FILE *fp, int newsockfd)
{
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fclose(fp);
    write(newsockfd, &file_size, sizeof(file_size));
}

// function to enqueue the request
void receive_enqueue(int newsockfd, long long int reqID) 
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
// function to dequeue the request
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
    }  

	pthread_mutex_unlock(&receive_queue_mutex);
	return temp;
}

// function to assign status
void status_enqueue(int sockfd,long long int reqID) 
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

// function to dequeue status
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
    }  
   
	pthread_mutex_unlock(&status_queue_mutex);
	return temp;
}

// function to enqueue process
void process_enqueue(long long int reqID) 
{
	struct processq *node = new struct processq;
	node->requestid=reqID;
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

// function to dequeue process
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
    }  
	pthread_mutex_unlock(&process_queue_mutex);
	return temp;
}

// function to grade the file
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

// function to generate unique id
long long generateUniqueID()
{
	// using chrono to get the current time and generating unique ID of it
    auto currentTime = chrono::system_clock::now();
    auto durationSinceEpoch = currentTime.time_since_epoch();
    auto secondsSinceEpoch = chrono::duration_cast<chrono::seconds>(durationSinceEpoch);
    auto microsecondsComponent = chrono::duration_cast<chrono::microseconds>(durationSinceEpoch);

    long long timeInSeconds = secondsSinceEpoch.count();
    long long microseconds = microsecondsComponent.count() % 1000000; // Get the microseconds component

    return timeInSeconds * 1000000 + microseconds; // Combine seconds and microseconds for a unique ID
}

// function to check the status
void *checkStatus(void *f)
{
	struct statusq* x = status_dequeue();
	long long int reqID= x->requestid;
	int sockfd = x->sockfd;
	// check if the request is in the request_status_map
	if(request_status_map.find(reqID) != request_status_map.end())
	{
		// check if the status is 0
		if(request_status_map[reqID].first==0)
		{
			struct processq* temp=p_front;
			
			int count=0;
			pthread_mutex_lock(&receive_queue_mutex);
			while(temp && temp->requestid!=reqID)
			{
				temp=temp->next;
				count++;
			}
			pthread_mutex_unlock(&receive_queue_mutex);
			count++;
			
			int x = 0;
			write(sockfd,&x,sizeof(x));
			write(sockfd,&count,sizeof(count));
			
		}
		// check if the status is 1
		else if(request_status_map[reqID].first==1)
		{
			int x = 1;
			write(sockfd,&x,sizeof(x));
		}
		// check if the status is 2
		else
		{
			int x = 2;
			write(sockfd,&x,sizeof(x));
			
			char buffer[BUFFER_SIZE];
			// check if the status result is 0
			if(request_status_map[reqID].second == 0)
			{
				string Cerror_file = "Cerror"+to_string(reqID)+".txt";

				FILE* fp = fopen(Cerror_file.c_str(),"rb");
				filesize(fp,sockfd);
				int cerror = open(Cerror_file.c_str(),O_RDONLY);
				sresult(sockfd,cerror,0,buffer);
			}
			// check if the status result is 1
			else if(request_status_map[reqID].second == 1)
			{
				//file size is not called here and same to pass also
				int x=0;
				write(sockfd,&x,sizeof(x));
				sresult(sockfd,-1,1,buffer);
			}	
			// check if the status result is 2
			else if(request_status_map[reqID].second == 2)
			{
				string diff_file = "diff"+to_string(reqID)+".txt";
				FILE* fp = fopen(diff_file.c_str(),"rb");
				filesize(fp,sockfd);
				
				int diffFd = open(diff_file.c_str(),O_RDONLY);
				if(diffFd < 0)
					error("ERROR opening file");
				sresult(sockfd,diffFd,2,buffer);
			}
			else
			{
				int x=0;
				write(sockfd,&x,sizeof(x));
				sresult(sockfd,-1,3,buffer);
			}

		}
	}
	else
	{
		int x = 3;
		write(sockfd,&x,sizeof(x));
	}
	
	return NULL;
}

// function to store the data
void *storedata(void *f)
{
	while(1)
	{
		FILE* file = fopen("processq_data.txt", "w");
		if (!file) {
			error("Error opening processq_data file");
			return NULL;
		}
		pthread_mutex_lock(&process_queue_mutex);
		processq* current = p_front;
		while (current != nullptr) {

			fprintf(file, "%lld\n", current->requestid);
			current = current->next;
		}
		pthread_mutex_unlock(&process_queue_mutex);
		fclose(file);
		
		//Storing the data into the hash table
		FILE* f = fopen("hashtable_data.txt", "w");
		if (!f) {
			error("Error opening hashtable_data file");
			return NULL;
		}
		for (const auto& entry : request_status_map) {
			fprintf(f, "%lld %d %d\n", entry.first, entry.second.first, entry.second.second);
		}
		fclose(f);
		sleep(1);
	}
	return NULL;
	
}

// function to retrieve the data
void retrivedata()
{
	FILE* f1 = fopen("processq_data.txt", "r");

    if (!f1) {
        error("Error opening processq_data file for reading");
        return;
    }

    long long int value;

    while (fscanf(f1, "%lld", &value) == 1) 
	{
		process_enqueue(value);
    }

    fclose(f1);

	FILE* f2 = fopen("hashtable_data.txt", "r");
    if (!f2) 
	{
        error("Error opening hashtable_data file for reading");
        return;
    }
    long long int key; 
	int value1, value2;
    while (fscanf(f2, "%lld %d %d", &key, &value1, &value2) == 3) {
		if(value1 == 1)
		{
			value1 = 0;
			process_enqueue(key);
		}
        request_status_map[key] = {value1, value2};
    }
    fclose(f2);
}