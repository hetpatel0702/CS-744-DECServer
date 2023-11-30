/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <math.h>
#include<errno.h>
#include<stdbool.h>

#define BUFFER_SIZE 128

double totalTime = 0;
double Tsend = 0, Trcv = 0;
int successfulRes = 0;
int numTimeouts = 0;
int numErrors = 0;

void error(char *msg,int sockfd,int sleepTime,bool timeout) 
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == 0)
    {
        Trcv = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    }
    
    if(Tsend != 0)
        totalTime += (Trcv - Tsend);

    perror(msg);
    close(sockfd);
    sleep(sleepTime);
    if(timeout)
        numTimeouts++;
    else
        numErrors++;
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;

    struct timeval tv;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE]; // buffer for message

    if (argc < 7) {
        fprintf(stderr, "usage %s hostname port file loop sleep timeout\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    char *sourceFile = argv[3];
    int loopNum = atoi(argv[4]);
    int sleepTime = atoi(argv[5]);

    struct timeval timeout;
    timeout.tv_sec = atoi(argv[6]);
    timeout.tv_usec = 0;

    double start, end;
    double avgTime = 0;
    

    if (gettimeofday(&tv, NULL) == 0) {
        start = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    int loopNum2 = loopNum;
    for(int i=0;i<loopNum;i++) 
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0)
        {
            error("ERROR opening socket",sockfd,sleepTime,0);
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
        {
            error("Setsockopt",sockfd,sleepTime,0);
            continue;
        }

        server = gethostbyname(argv[1]);

        if (server == NULL) {
            error("ERROR, no such host",sockfd,sleepTime,0);
            continue;
        }

        bzero((char *) &serv_addr, sizeof(serv_addr)); // set server address bytes to zero

        serv_addr.sin_family = AF_INET; // Address Family is IP

        bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
                server->h_length);

        serv_addr.sin_port = htons(portno);

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        {
            error("ERROR Connecting!",sockfd,sleepTime,0);
            continue;
        }

        bzero(buffer, sizeof(buffer));

        FILE* fp = fopen(sourceFile,"rb");
        fseek(fp,0,SEEK_END);
        int file_size = ftell(fp);
        fclose(fp);

        n = write(sockfd,&file_size,sizeof(file_size));
        if (n < 0)
        {
            error("ERROR writing to socket",sockfd,sleepTime,0);
            continue;
        }

        int gradeFd = open(sourceFile, O_RDONLY);
        
        bzero(buffer,sizeof(buffer));
        int readBytes;

        if (gettimeofday(&tv, NULL) == 0) 
        {
            Tsend = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
        }

        int f1 = 0;
        while ((readBytes = read(gradeFd, buffer , sizeof(buffer))) > 0) 
        {   
            n = write(sockfd, buffer, readBytes);   
            if (n < 0)
            {
                error("ERROR writing to socket",sockfd,sleepTime,0);
                f1=1;
                break;
            }
            bzero(buffer, sizeof(buffer));
        }
        if(f1)
            continue;

        int file=0,flag=0;
        int n=recv(sockfd,&file,sizeof(file),0);
        if (n < 0){
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                error("Received Timeout",sockfd,sleepTime,1);
            }
            else
            {
                error("ERROR reading from socket",sockfd,sleepTime,0);
            }
            continue;
        }

        if(file==0)
        {
            int reab=recv(sockfd,buffer,sizeof(buffer),0);
            if (reab == -1) 
            {
                flag=1;
                if (errno == EAGAIN || errno == EWOULDBLOCK) 
                {
                    error("Received Timeout..",sockfd,sleepTime,1);
                } 
                else 
                {
                    error("Error receiving data.",sockfd,sleepTime,0);
                }
                continue;
	        }
            write(1,buffer,reab);
        }
        else
        {
            int readb,f2=0;
            file+=16;
            while(file>0)
            {
                readb=recv(sockfd,buffer,sizeof(buffer),0);
                if (readb == -1) 
                {
                    flag=1;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) 
                    {
                        error("Received Timeout...",sockfd,sleepTime,1);
                    } 
                    else 
                    {
                        error("Error receiving data.",sockfd,sleepTime,0);
                    }
                    f2=1;
                    break;
                }
                
                n = write(1,buffer,readb);
                if (n < 0)
                {
                    error("ERROR writing to STDOUT!",sockfd,sleepTime,0);
                    f2=1;
                    break;
                }
                file-=readb;
                bzero(buffer,sizeof(buffer));
            }
            if(f2)
                continue;
        }
        
        if (gettimeofday(&tv, NULL) == 0)
        {
            Trcv = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
        }

        if (!flag)
            successfulRes++;
        
        totalTime += (Trcv - Tsend);
        printf("\nResponse Time: %lf\n", Trcv - Tsend);
        
        sleep(sleepTime);
        close(sockfd);
    }

    if (gettimeofday(&tv, NULL) == 0) 
    {
        end = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    avgTime = (1.0 * totalTime) / successfulRes;
    double totalLoopTime = end - start;


    if(successfulRes == 0)
        printf("\nAverage Response Time:%lf\n", totalLoopTime/loopNum2);    
    else
        printf("\nAverage Response Time:%lf\n", avgTime);
    
    printf("Successful Responses:%d\n", successfulRes);
    printf("Total Time:%lf\n", totalLoopTime);
    
    if(successfulRes == 0)
        printf("Throughput:%lf\n",0.0);
    else
        printf("Throughput:%lf\n", successfulRes / totalTime);

    printf("Request Sent Rate:%lf\n",1.0*loopNum2/totalLoopTime);
    printf("Successful Request Rate:%lf\n",1.0*successfulRes/totalLoopTime);
    printf("Timeout Rate:%lf\n",1.0*numTimeouts/totalLoopTime);
    printf("Error Rate:%lf\n",1.0*(numErrors)/totalLoopTime);

    return 0;
}
