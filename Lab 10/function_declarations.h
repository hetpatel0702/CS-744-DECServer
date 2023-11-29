#ifndef FUNCTION_DECLARATIONS_H
#define FUNCTION_DECLARATIONS_H
#include<iostream>
#include<unordered_map>
#include<chrono>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
using namespace std;

#define BUFFER_SIZE 128

extern unordered_map<long long int, pair<int,int>> request_status_map;

extern pthread_mutex_t receive_queue_mutex;
extern pthread_cond_t receive_cond;

extern pthread_mutex_t status_queue_mutex;
extern pthread_cond_t status_cond;

extern pthread_mutex_t process_queue_mutex;
extern pthread_cond_t process_cond;

void receive_enqueue(int newsockfd,long long int reqID);

struct receiveQueue* receive_dequeue();

void status_enqueue(int sockfd,long long int reqID);

struct statusq* status_dequeue();

void process_enqueue(long long int reqID);

struct processq* process_dequeue();

long long int generateUniqueID();

void filesize(FILE *fp, int newsockfd);

void *receive_file(void *arg);

void* gradeTheFile();

void sresult(int newsockfd, int fp, int a, char *buffer);

void error(const char *msg);

void *storedata(void *f);
void retrivedata();

void *checkStatus(void *f);
struct receiveQueue{
	int sockfd;
	long long int requestid;
	struct receiveQueue * next;
};

struct statusq
{
	int sockfd;
	long long int requestid;
	struct statusq* next;
};

struct processq
{
	long long int requestid;
	struct processq* next;
};

extern struct receiveQueue *r_front, *r_rear;
extern struct statusq *s_front, *s_rear;
extern struct processq *p_front , *p_rear;
#endif  // FUNCTION_DECLARATIONS_H