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

extern int queue[MAX_QUEUE_SIZE];

void receive_enqueue(int newsockfd, int reqID);

struct receiveQueue* receive_dequeue();

int generateUniqueID();

void filesize(FILE *fp, int newsockfd);

void* gradeTheFile();

void sresult(int newsockfd, int fp, int a, char *buffer);

void error(const char *msg);

struct receiveQueue{
	int sockfd;
	int requestid;
	struct receiveQueue * next;
};

extern struct receiveQueue *r_front, *r_rear;

#endif  // FUNCTION_DECLARATIONS_H