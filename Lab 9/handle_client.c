#include "funtion_declarations.h"

//function to grade the file
void* gradeTheFile()
{
	while(1)
	{
		int newsockfd=dequeue();
		if (newsockfd < 0) {
			error("ERROR on accept");
		}

		pthread_t threadID = pthread_self();		
		char buffer[BUFFER_SIZE];
		char Cerror_file[50];
		char Rerror_file[50];
		char output_file[50];
		char diff_file[50];
		char grade_file[50];
		char grade_file_exe[50];

		char compile_command[150];
		char run_command[150];
		char diff_command[150];
		char delete_files[150];

		sprintf(output_file,"output%lu.txt",threadID);
		sprintf(Rerror_file,"Rerror%lu.txt",threadID);
		sprintf(Cerror_file,"Cerror%lu.txt",threadID);
		sprintf(diff_file,"diff%lu.txt",threadID);
		sprintf(grade_file,"gradeFile%lu.c",threadID);
		sprintf(grade_file_exe,"file%lu",threadID);

		int file_size = 0;
		
		//reading file size 
		int n = read(newsockfd,&file_size,sizeof(file_size));
		if (n < 0)
			error("ERROR reading from socket");

		int newGradeFd = open(grade_file,O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
		if (newGradeFd < 0)
			error("ERROR opening file");

		bzero(buffer,BUFFER_SIZE);
		while (file_size > 0)
		{	
			//reading the file data 
			int readBytes = read(newsockfd, buffer, BUFFER_SIZE);
			if (readBytes < 0)
				error("ERROR reading from socket");

			int wroteBytes = write(newGradeFd,buffer,readBytes);
			if (wroteBytes < 0)
				error("ERROR writing to file");

			file_size -= readBytes;

			bzero(buffer,BUFFER_SIZE);
		}
		close(newGradeFd);

		sprintf(compile_command,"gcc gradeFile%lu.c -o file%lu 2>Cerror%lu.txt",threadID,threadID,threadID);
		int compiling = system(compile_command);
		
		//if compilation errror occurs
		if(compiling != 0)
		{
			FILE* fp = fopen(Cerror_file,"rb");
			filesize(fp,newsockfd);
			
			int cerror = open(Cerror_file,O_RDONLY);
			if (cerror < 0)
				error("ERROR opening file");

			bzero(buffer,BUFFER_SIZE);
			sresult(newsockfd,cerror,0,buffer);
			
			sprintf(delete_files,"rm gradeFile%lu.c Cerror%lu.txt",threadID,threadID);
			system(delete_files);
			close(cerror);
		}
		else  //compilation success
		{
			sprintf(run_command,"./file%lu >output%lu.txt 2>Rerror%lu.txt",threadID,threadID,threadID);
			int runTheFile = system(run_command);

			//if runtime error occurs
			if(runTheFile != 0)
			{
				FILE* fp = fopen(Rerror_file,"rb");
				filesize(fp,newsockfd);
							
				int rerror = open(Rerror_file,O_RDONLY);
				if(rerror < 0)
					error("ERROR opening file");

				bzero(buffer,BUFFER_SIZE);
				sresult(newsockfd,rerror,1,buffer);
				close(rerror);
			}
			else //no runtime error 
			{
				sprintf(diff_command,"echo -n '1 2 3 4 5 6 7 8 9 10 ' | diff - output%lu.txt > diff%lu.txt",threadID,threadID);
				int difference = system(diff_command);
				
				//checking of actual output with generated output 
				if(difference != 0)
				{
					FILE* fp = fopen(diff_file,"rb");
					filesize(fp,newsockfd);
					
					int diffFd = open(diff_file,O_RDONLY);
					if(diffFd < 0)
						error("ERROR opening file");

					bzero(buffer,BUFFER_SIZE);
					sresult(newsockfd,diffFd,2,buffer);
					close(diffFd);
				}
				else 
				{
					int x=0;
					write(newsockfd,&x,sizeof(x));
					sresult(newsockfd,-1,-1,buffer);
				}
				sprintf(delete_files,"rm diff%lu.txt",threadID);
				system(delete_files);
			}
			sprintf(delete_files,"rm gradeFile%lu.c file%lu output%lu.txt Cerror%lu.txt Rerror%lu.txt",threadID,threadID,threadID,threadID,threadID);
			system(delete_files);
		}
		close(newsockfd);
	}
}
