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

#define BUFFER_SIZE 30
int numErrors = 0;
void error(char *msg) {
    fprintf(stderr,"%s", msg);
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
    double totalTime = 0;
    double Tsend = 0, Trcv = 0;
    int successfulRes = 0;
    int numTimeouts = 0;
    int numErrors = 0;

    if (gettimeofday(&tv, NULL) == 0) {
        start = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    int loopNum2 = loopNum;
    while (loopNum--) 
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0)
        {
            error("ERROR opening socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
        {
            error("Setsockopt\n");
            continue;
        }

        server = gethostbyname(argv[1]);

        if (server == NULL) {
            error("ERROR, no such host\n");
            continue;
        }

        bzero((char *) &serv_addr, sizeof(serv_addr)); // set server address bytes to zero

        serv_addr.sin_family = AF_INET; // Address Family is IP

        bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
                server->h_length);

        serv_addr.sin_port = htons(portno);

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        {
            error("ERROR Connecting!");
            close(sockfd);
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
            error("ERROR writing to socket");
            close(sockfd);
            continue;
        }

        int gradeFd = open(sourceFile, O_RDONLY);
        
        bzero(buffer,sizeof(buffer));
        int readBytes;

        if (gettimeofday(&tv, NULL) == 0) 
        {
            Tsend = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
        }
        printf("Tsend:%lf\n",Tsend);
        while ((readBytes = read(gradeFd, buffer , sizeof(buffer))) > 0) 
        {   
            n = write(sockfd, buffer, readBytes);   
            if (n < 0)
            {
                error("ERROR writing to socket");
                close(sockfd);
                goto Label;
            }
            bzero(buffer, sizeof(buffer));
        }
            
        int file=0,flag=0;
        int n=read(sockfd,&file,sizeof(file));
        if (n < 0){
            error("ERROR reading from socket\n");
            close(sockfd);
            continue;
        }

        if(file==0)
        {
            int reab=recv(sockfd,buffer,sizeof(buffer),0);
            printf("Reads:%d\n",reab);
            if (reab == -1) 
            {
                flag=1;
                if (errno == EAGAIN || errno == EWOULDBLOCK) 
                {
                    fprintf(stderr,"Received timeout.\n");
                    numTimeouts++;
                } 
                else 
                {
                    error("Error receiving data.\n");
                    close(sockfd);
                    continue;
                }
	        }
            write(1,buffer,reab);
        }
        else
        {
            int readb;
            file+=16;
            while(file>0)
            {
                readb=recv(sockfd,buffer,sizeof(buffer),0);
                if (readb == -1) 
                {
                    flag=1;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) 
                    {
                        fprintf(stderr,"Received timeout.\n");
                        numTimeouts++;
                    } 
                    else 
                    {
                        error("Error receiving data.\n");
                        close(sockfd);
                    }
                    goto Label;
                }
                
                n = write(1,buffer,readb);
                if (n < 0)
                {
                    error("ERROR writing to STDOUT!\n");
                    close(sockfd);
                    continue;
                }
                file-=readb;
                
                bzero(buffer,sizeof(buffer));
            }
        }
        
        if (gettimeofday(&tv, NULL) == 0)
        {
            Trcv = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
        }
        printf("Trcv:%lf\n",Trcv);
        if (!flag)
            successfulRes++;
        
        totalTime += (Trcv - Tsend);
        printf("\nResponse Time: %lf\n", Trcv - Tsend);
        
        Label:
        sleep(sleepTime);
        close(sockfd);
    }

    if (gettimeofday(&tv, NULL) == 0) 
    {
        end = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    avgTime = (1.0 * totalTime) / loopNum2;
    double totalLoopTime = end - start;


    printf("\nAverage Response Time:%lf\n", avgTime);
    printf("Successful Responses:%d\n", successfulRes);
    printf("Total Time:%lf\n", totalLoopTime);
    printf("Throughput:%lf\n", successfulRes / totalTime);
    printf("Request Sent Rate:%lf\n",1.0*loopNum2/totalLoopTime);
    printf("Successful Request Rate:%lf\n",1.0*successfulRes/totalLoopTime);
    printf("Timeout Rate:%lf\n",1.0*numTimeouts/totalLoopTime);
    printf("Error Rate:%lf\n",1.0*(numErrors)/totalLoopTime);

    return 0;
}
