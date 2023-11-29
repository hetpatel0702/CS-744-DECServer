/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>
#include<fcntl.h>
#include <netinet/in.h>


void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) {
  int sockfd, //the listen socket descriptor (half-socket)
   newsockfd, //the full socket after the client connection is made
   portno; //port number at which server listens

  socklen_t clilen; //a type of an integer for holding length of the socket address
  char buffer[10000]; //buffer for reading and writing the messages
  struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 

  if (sockfd < 0)
    error("ERROR opening socket");

 
  bzero((char *)&serv_addr, sizeof(serv_addr)); 
  
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_addr.s_addr = INADDR_ANY;   

  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  listen(sockfd, 1000); 

  clilen = sizeof(cli_addr);  

    
  while (1)
  {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0)
      error("ERROR on accept");
    int flag = 0;
    int loopnum=0;

    while(!flag)
    {
      bzero(buffer, 10000); 

      n = read(newsockfd, buffer, sizeof(buffer));


      if(!strcmp(buffer,"DONE"))
      {
        flag=1;
      }

      
      if (n < 0)
        error("ERROR reading from socket");

      int newGradeFd = open("gradeFileAtServer.c",O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
      
      int wroteBytes = write(newGradeFd,buffer+6,n-6);

      int compiling = system("gcc gradeFileAtServer.c -o gradeThisFile 2>Cerror.txt");
      
      
      if(compiling != 0)
      {
          char err[15000] = "COMPILE ERROR\n";

          int cerror = open("Cerror.txt",O_RDONLY);
          bzero(buffer,10000);
          
          int k = read(cerror,buffer,sizeof(buffer));
          strcat(err, buffer);
          write(newsockfd,err,k+15);

          system("rm gradeFileAtServer.c Cerror.txt");
      }
      else
      {
        int runTheFile = system("./gradeThisFile >output.txt 2>Rerror.txt");
        if(runTheFile != 0)
        {
          char err[15000] = "RUNTIME ERROR\n";

          int rerror = open("Rerror.txt",O_RDONLY);
          bzero(buffer,10000);
          
          int k = read(rerror,buffer,sizeof(buffer));
          strcat(err, buffer);
          write(newsockfd,err,k+sizeof("RUNTIME ERROR\n"));

          system("rm Rerror.txt");

        }
        else
        {
          int difference = system("echo -n '1 2 3 4 5 6 7 8 9 10 ' | diff - output.txt > diff.txt");
          if(difference != 0)
          {
            char err[15000] = "OUTPUT ERROR\n";

            int outputFd = open("output.txt",O_RDONLY);
            bzero(buffer,10000);
            
            int m = read(outputFd,buffer,sizeof(buffer));
            strcat(err, buffer);
            
            int diffFd = open("diff.txt",O_RDONLY);
            bzero(buffer,10000);

            int l = read(diffFd,buffer,sizeof(buffer));
            strcat(err,buffer);
            write(newsockfd,err,l+m+strlen("OUTPUT ERROR\n"));
          }
          else
          {
            n = write(newsockfd, "PASS", sizeof("PASS"));
          }
          system("rm diff.txt");
        }
          system("rm gradeFileAtServer.c gradeThisFile output.txt Cerror.txt");
      }
    }
    if (n < 0)
      error("ERROR writing to socket");
  }
    return 0;
}


