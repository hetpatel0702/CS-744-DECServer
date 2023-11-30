#include "function_declarations.h"
#include "handle_client.cpp"
#include "function_definitions.cpp"

unordered_map<long long int, pair<int,int>> request_status_map;

//Initializing lock and condition variable

pthread_mutex_t receive_queue_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t receive_cond=PTHREAD_COND_INITIALIZER;

pthread_mutex_t status_queue_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t status_cond=PTHREAD_COND_INITIALIZER;

pthread_mutex_t process_queue_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t process_cond=PTHREAD_COND_INITIALIZER;

struct receiveQueue *r_front = NULL, *r_rear = NULL;
struct statusq *s_front = NULL, *s_rear= NULL;
struct processq *p_front = NULL, *p_rear = NULL;

int main(int argc, char *argv[]) 
{
    int sockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 3) 
	{
        fprintf(stderr, "ERROR, no port or thread pool size provided\n");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    portno = atoi(argv[1]);
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
        error("ERROR on binding");
		exit(0);
	}

    listen(sockfd, 500);

    clilen = sizeof(cli_addr);

    int total_threads = atoi(argv[2]);
	int thread_pool1,thread_pool2;

    if(total_threads % 2 == 0)
    {
        thread_pool1 = total_threads / 2;
        thread_pool2 = total_threads / 2;
    }    
    else 
    {
         thread_pool1 = total_threads / 2;       
         thread_pool2 = total_threads / 2 + 1;       
    }

    pthread_t receive_thread[thread_pool1];

    //Creating thread pool for receiving file
	for (int i = 0; i < thread_pool1; i++) 
	{
        if (pthread_create(&receive_thread[i], nullptr, receive_file, nullptr) != 0)
		{
            fprintf(stderr, "Error Creating Thread\n");
        }
    }

    pthread_t thread[thread_pool2];

    //Creating thread pool for grading client requested file
    for (int i = 0; i < thread_pool2; i++) 
	{
        if (pthread_create(&thread[i], nullptr, gradeTheFile, NULL) != 0)
		{
            fprintf(stderr, "Error Creating Thread\n");
        }
    }

    pthread_t store_data;
    if (pthread_create(&store_data, nullptr, storedata, NULL) != 0)
    {
        fprintf(stderr, "Error Creating Thread for storing data in file\n");
    }

    retrivedata();

    while (true) 
	{
        int *newsockfd = (int *)(malloc(sizeof(int)));

        if (newsockfd == nullptr) 
		{
            fprintf(stderr, "Error: Unable to allocate memory for newsockfd\n");
            continue;
        }
        *newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (*newsockfd < 0) 
		{
            fprintf(stderr, "Error: accept failed\n");
            free(newsockfd);
            continue;
        }
		bool flag;
		read(*newsockfd,&flag,sizeof(flag));

		if(flag==1)
		{
            //Calling generateUniqueID for generating id and pushed in the queue
			long long int id = generateUniqueID();
			receive_enqueue(*newsockfd, id);
		}
		else
		{
			long long int id;
			read(*newsockfd,&id,sizeof(id));
			status_enqueue(*newsockfd, id);
			
			pthread_t status_thread;
            
            //Creating status_thread for checking status of request
			if (pthread_create(&status_thread, nullptr, checkStatus, NULL) != 0)
			{
				fprintf(stderr, "Error Creating Thread for checking status\n");
			}
		} 
    }
    return 0;
}

