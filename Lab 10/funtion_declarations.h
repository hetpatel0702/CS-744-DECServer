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
#define MAX_QUEUE_SIZE 500


extern unordered_map<int, pair<int,int>> request_status_map;

extern pthread_mutex_t receive_queue_mutex;
extern pthread_cond_t receive_cond;

extern pthread_mutex_t status_queue_mutex;
extern pthread_cond_t status_cond;

extern pthread_mutex_t process_queue_mutex;
extern pthread_cond_t process_cond;

extern int queue[MAX_QUEUE_SIZE];

void receive_enqueue(int newsockfd, int reqID);

struct receiveQueue* receive_dequeue();

void status_enqueue(int sockfd,int reqID);

struct statusq* status_dequeue();

void process_enqueue(int reqID);

struct processq* process_dequeue();

int generateUniqueID();

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
	int requestid;
	struct receiveQueue * next;
};

struct statusq
{
	int sockfd;
	int requestid;
	struct statusq* next;
};

struct processq
{
	// int sockfd;
	int requestid;
	struct processq* next;
};

extern struct receiveQueue *r_front, *r_rear;
extern struct statusq *s_front, *s_rear;
extern struct processq *p_front , *p_rear;
#endif  // FUNCTION_DECLARATIONS_H