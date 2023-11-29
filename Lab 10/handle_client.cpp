#include "function_declarations.h"

void *receive_file(void *arg)
{
	while(1)
	{
		struct receiveQueue* t = receive_dequeue(); 
		int newsockfd = t->sockfd;
		long long int reqID = t->requestid;
		
		char buffer[BUFFER_SIZE];
		char grade_file[50];

		sprintf(grade_file,"gradeFile%lld.c",reqID);

		int file_size = 0;
		int n = read(newsockfd,&file_size,sizeof(file_size));
		if (n < 0)
			error("ERROR reading from socket");

		int newGradeFd = open(grade_file,O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IXUSR);
		if (newGradeFd < 0)
			error("ERROR opening file");

		memset(buffer, 0, BUFFER_SIZE);
		while (file_size > 0)
		{	
			int readBytes = read(newsockfd, buffer, BUFFER_SIZE);
			if (readBytes < 0)
				error("ERROR reading from socket");

			int wroteBytes = write(newGradeFd,buffer,readBytes);

			if (wroteBytes < 0)
				error("ERROR writing to file");

			file_size -= readBytes;

			memset(buffer, 0, BUFFER_SIZE);
		}

		process_enqueue(reqID);
		request_status_map[reqID].first = 0;

		write(newsockfd,&reqID,sizeof(reqID));
		close(newGradeFd);
	}	
}
void *gradeTheFile(void* f)
{
	while(1)
	{
		struct processq* t = process_dequeue();
		long long int reqID = t->requestid;
		
		request_status_map[reqID].first=1;
	
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

		sprintf(output_file,"output%lld.txt",reqID);
		sprintf(Rerror_file,"Rerror%lld.txt",reqID);
		sprintf(Cerror_file,"Cerror%lld.txt",reqID);
		sprintf(diff_file,"diff%lld.txt",reqID);
		sprintf(grade_file,"gradeFile%lld.c",reqID);
		sprintf(grade_file_exe,"file%lld",reqID);
		
		sprintf(compile_command,"gcc gradeFile%lld.c -o file%lld 2>Cerror%lld.txt",reqID,reqID,reqID);
		int compiling = system(compile_command);
		
		if(compiling != 0)
		{
			sprintf(delete_files,"rm gradeFile%lld.c",reqID);
			system(delete_files);

			request_status_map[reqID].second = 0;
		}
		else
		{
			sprintf(run_command,"./file%lld >output%lld.txt",reqID,reqID);
			int runTheFile = system(run_command);

			if(runTheFile != 0)
			{
				sprintf(delete_files,"rm gradeFile%lld.c file%lld Cerror%lld.txt",reqID,reqID,reqID);
				system(delete_files);

				request_status_map[reqID].second = 1;
			}
			else
			{
				sprintf(diff_command,"echo -n '1 2 3 4 5 6 7 8 9 10 ' | diff - output%lld.txt > diff%lld.txt",reqID,reqID);
				int difference = system(diff_command);
				if(difference != 0)
				{
					sprintf(delete_files,"rm gradeFile%lld.c file%lld output%lld.txt Cerror%lld.txt",reqID,reqID,reqID,reqID);
					system(delete_files);

					request_status_map[reqID].second = 2;
				}
				else
				{
					request_status_map[reqID].second = 3;
					sprintf(delete_files,"rm gradeFile%lld.c file%lld diff%lld.txt Cerror%lld.txt",reqID,reqID,reqID,reqID);
					system(delete_files);
				}
			}
		}		
		request_status_map[reqID].first=2;
	}
}