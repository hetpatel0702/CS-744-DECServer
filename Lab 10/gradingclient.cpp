#include<bits/stdc++.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
using namespace std;
#define BUFFER_SIZE 128

int successfulRes = 0;
int numTimeouts = 0;
int numErrors = 0;
double Tsend = 0, Trcv = 0;
double totalTime = 0;

//Handling error message
void error(const char *msg,int sockfd,bool timeout) 
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
    if(timeout)
        numTimeouts++;
    else
        numErrors++;
}
int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct timeval tv;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE]; 

    if (argc != 7) {
        fprintf(stderr, "usage %s new/status hostname port file/requestId timeout fileid\n", argv[0]);
        exit(0);
    }
    char* type = argv[1];
    bool isNew;

    //Checking the request either new or for status checking
    if(!strcmp(type,"new"))
    {
        isNew = true;
    }
    else if(!strcmp(type,"status"))
    {
        isNew = false;
    }
    else
    {
        fprintf(stderr, "Usage %s new/status hostname port file/requestId timeout fileid\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[3]);
    char *sourceFile_or_reqID = argv[4];

    struct timeval timeout;
    timeout.tv_sec = atoi(argv[5]);
    timeout.tv_usec = 0;

    int file_id = atoi(argv[6]);
    double start, end;
    double avgTime = 0;

    if (gettimeofday(&tv, NULL) == 0) 
    {
        start = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("ERROR opening socket",sockfd,0);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0)
    {
        error("Setsockopt",sockfd,0);
    }

    server = gethostbyname(argv[2]);

    if (server == NULL) {
        error("ERROR, no such host",sockfd,0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET; 

    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
            server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR Connecting!",sockfd,0);
    }

    memset(buffer, 0, BUFFER_SIZE);

    if (gettimeofday(&tv, NULL) == 0) 
    {
        Tsend = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    }
    
    n = write(sockfd,&isNew,sizeof(isNew));

    int flag=0;
    if(isNew == false) //if the request is for status checking
    {
        long long int reqID = stoll(sourceFile_or_reqID);

        //writing requestid to socket 
        n = write(sockfd,&reqID,sizeof(reqID));
        if (n < 0)
        {
            error("ERROR writing to socket",sockfd,0);  
        }

        int resType;
        
        //reading the status of request from socket
        n = read(sockfd,&resType,sizeof(resType));

        if(resType == 0)
        {
            cout << "Your Request is in Queue at position ";
            int qpos = -1;
            n = read(sockfd,&qpos,sizeof(qpos));
            cout << qpos << endl;
        }
        else if(resType == 1)
        {
            cout << "Request Processing has started!" << endl;
        }
        else if(resType == 2)
        {
            cout << "Processing is completed!\nHere's Server Response: ";

            int received_file_size=0;

            //Receiving file size 
            int n=recv(sockfd,&received_file_size,sizeof(received_file_size),0);
            if (n < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) 
                {
                    error("Received Timeout",sockfd,1);
                }
                else
                {
                    error("ERROR reading from socket",sockfd,0);
                }
            }
            if(received_file_size==0) 
            {
                int response=recv(sockfd,buffer,sizeof(buffer),0);
                if (response == -1) 
                {
                    flag=1;
                    if (errno == EAGAIN || errno == EWOULDBLOCK) 
                    {
                        error("Received Timeout..",sockfd,1);
                    } 
                    else 
                    {
                        error("Error receiving data.",sockfd,0);
                    }
                }
                write(1,buffer,response);
            }
            else
            {
                int readb=0;
                received_file_size+=16;
                while(received_file_size>0)
                {
                    readb=recv(sockfd,buffer,sizeof(buffer),0);
                    if (readb == -1) 
                    {
                        flag=1;
                        if (errno == EAGAIN || errno == EWOULDBLOCK) 
                        {
                            error("Received Timeout...",sockfd,1);
                        } 
                        else 
                        {
                            error("Error receiving data.",sockfd,0);
                        }
                        break;
                    }
                    n = write(1,buffer,readb);
                    if (n < 0)
                    {
                        flag=1;
                        error("ERROR writing to STDOUT!",sockfd,0);
                        break;
                    }
                    received_file_size-=readb;
                    memset(buffer, 0, BUFFER_SIZE);
                }     
            }
        }
        else
        {
            cout << "Request Not Found!" << endl;
        }
    }
    else //if the request is new 
    {
        FILE* fp = fopen(sourceFile_or_reqID,"rb");
        fseek(fp,0,SEEK_END);
        int file_size = ftell(fp);
        fclose(fp);

        //writing the file size to the socket
        n = write(sockfd,&file_size,sizeof(file_size));
        if (n < 0)
        {
            error("ERROR writing to socket",sockfd,0);
        }

        int gradeFd = open(sourceFile_or_reqID, O_RDONLY);
        
        memset(buffer,0,BUFFER_SIZE);
        int readBytes;

        int f1 = 0;
        //Reading file and writing on socket 
        while ((readBytes = read(gradeFd, buffer , sizeof(buffer))) > 0) 
        {   
            n = write(sockfd, buffer, readBytes);   
            if (n < 0)
            {
                error("ERROR writing to socket",sockfd,0);
                f1=1;
                break;
            }
            memset(buffer, 0, BUFFER_SIZE);
        }
    
        long long int reqID;
        cout << "Your Request ID is: ";

        //Received requestid from server 
        read(sockfd,&reqID,sizeof(reqID));
        cout << reqID << endl;


        char request_ID_file_command[50];
        sprintf(request_ID_file_command,"echo %lld > client_request_id_%d.txt",reqID,file_id);
        system(request_ID_file_command);

    }
    
    if (gettimeofday(&tv, NULL) == 0)
    {
        Trcv = (double) tv.tv_sec + (double)tv.tv_usec / 1000000.0;
    }

    //Calculating number of successful responses
    if (!flag)
        successfulRes++;
    
    totalTime += (Trcv - Tsend);
    
    close(sockfd);
    
    if (gettimeofday(&tv, NULL) == 0) 
    {
        end = (double) tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    avgTime = (1.0 * totalTime) / successfulRes;
    double totalLoopTime = end - start;

    printf("\nSuccessful Responses:%d\n", successfulRes);
    printf("Total Time:%lf\n", totalLoopTime);
    return 0;
}
