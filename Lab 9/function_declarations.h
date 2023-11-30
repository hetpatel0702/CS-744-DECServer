#pragma once
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#define BUFFER_SIZE 128
#define MAX_QUEUE_SIZE 500

extern pthread_mutex_t qmutex;
extern pthread_cond_t qempty; 
extern int queue[MAX_QUEUE_SIZE];
extern int qsize;
extern int front;
extern int rear;
void enqueue(int newsockfd);
int dequeue();
void filesize(FILE *fp, int newsockfd);
void sresult(int newsockfd, int fp, int a, char *buffer);
void* gradeTheFile();
void error(char *msg);
void *measure_queue_size();
int isEmpty();
int isFull(); 
