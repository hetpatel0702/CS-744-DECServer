#include "function_declarations.h"

// function to handle client requests
void *receive_file(void *arg)
{
	while(1)
	{
		// dequeue the request
		struct receiveQueue* t = receive_dequeue(); 
		int newsockfd = t->sockfd;
		long long int reqID = t->requestid;
		
		char buffer[BUFFER_SIZE];
		char grade_file[50];

		sprintf(grade_file,"gradeFile%lld.c",reqID);

		int file_size = 0;
		// read the file size
		int n = read(newsockfd,&file_size,sizeof(file_size));
		if (n < 0)
			error("ERROR reading from socket");
		// open the file
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
		// enqueue int process map
		process_enqueue(reqID);
		request_status_map[reqID].first = 0;
		// send the request id to client
		write(newsockfd,&reqID,sizeof(reqID));
		close(newGradeFd);
	}	
}

// function to grade the file
void *gradeTheFile(void* f)
{
	while(1)
	{
		// dequeue the request
		struct processq* t = process_dequeue();
		long long int reqID = t->requestid;
		// set the status to 1
		request_status_map[reqID].first=1;
	
		char buffer[BUFFER_SIZE];
		// create the file names
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
		// create the file names
		sprintf(output_file,"output%lld.txt",reqID);
		sprintf(Rerror_file,"Rerror%lld.txt",reqID);
		sprintf(Cerror_file,"Cerror%lld.txt",reqID);
		sprintf(diff_file,"diff%lld.txt",reqID);
		sprintf(grade_file,"gradeFile%lld.c",reqID);
		sprintf(grade_file_exe,"file%lld",reqID);
		// compile command for the file
		sprintf(compile_command,"gcc gradeFile%lld.c -o file%lld 2>Cerror%lld.txt",reqID,reqID,reqID);
		// compile the file
		int compiling = system(compile_command);
		
		if(compiling != 0)
		{
			request_status_map[reqID].second = 0;
		}
		else
		{
			// run command for the file
			sprintf(run_command,"./file%lld >output%lld.txt",reqID,reqID);
			// run the file
			int runTheFile = system(run_command);

			if(runTheFile != 0)
			{
				sprintf(delete_files,"rm file%lld Cerror%lld.txt",reqID,reqID);
				system(delete_files);

				request_status_map[reqID].second = 1;
			}
			else
			{
				// diff command for the file
				sprintf(diff_command,"echo -n '1 2 3 4 5 6 7 8 9 10 ' | diff - output%lld.txt > diff%lld.txt",reqID,reqID);
				// check the difference
				int difference = system(diff_command);
				if(difference != 0)
				{
					sprintf(delete_files,"rm file%lld output%lld.txt Cerror%lld.txt",reqID,reqID,reqID);
					system(delete_files);
					request_status_map[reqID].second = 2;
				}
				else
				{
					request_status_map[reqID].second = 3;
					sprintf(delete_files,"rm file%lld diff%lld.txt Cerror%lld.txt",reqID,reqID,reqID);
					system(delete_files);
				}
			}
		}		
		request_status_map[reqID].first=2;
		sprintf(delete_files,"rm gradeFile%lld.c",reqID);
		system(delete_files);
	}
}