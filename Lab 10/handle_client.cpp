#include "funtion_declarations.h"


void *gradeTheFile(void* f)
{
	while(1)
	{
		struct processq* t = process_dequeue();
		int newsockfd = t->sockfd;
		int reqID = t->requestid;
		
		request_status_map[reqID].first=1;
		
		if (newsockfd < 0) {
			error("ERROR on accept");
		}
	
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

		sprintf(output_file,"output%d.txt",reqID);
		sprintf(Rerror_file,"Rerror%d.txt",reqID);
		sprintf(Cerror_file,"Cerror%d.txt",reqID);
		sprintf(diff_file,"diff%d.txt",reqID);
		sprintf(grade_file,"gradeFile%d.c",reqID);
		sprintf(grade_file_exe,"file%d",reqID);
		
					

		sprintf(compile_command,"gcc gradeFile%d.c -o file%d 2>Cerror%d.txt",reqID,reqID,reqID);
		int compiling = system(compile_command);
		
		if(compiling != 0)
		{
			sprintf(delete_files,"rm gradeFile%d.c",reqID);
			system(delete_files);

			request_status_map[reqID].second = 0;
		}
		else
		{
			sprintf(run_command,"./file%d >output%d.txt",reqID,reqID);
			int runTheFile = system(run_command);

			if(runTheFile != 0)
			{
				sprintf(delete_files,"rm gradeFile%d.c file%d Cerror%d.txt",reqID,reqID,reqID);
				system(delete_files);

				request_status_map[reqID].second = 1;
			}
			else
			{
				sprintf(diff_command,"echo -n '1 2 3 4 5 6 7 8 9 10 ' | diff - output%d.txt > diff%d.txt",reqID,reqID);
				int difference = system(diff_command);
				if(difference != 0)
				{
					sprintf(delete_files,"rm gradeFile%d.c file%d output%d.txt Cerror%d.txt",reqID,reqID,reqID,reqID);
					system(delete_files);

					request_status_map[reqID].second = 2;
				}
				else
				{
					request_status_map[reqID].second = 3;
					sprintf(delete_files,"rm gradeFile%d.c file%d diff%d.txt Cerror%d.txt",reqID,reqID,reqID,reqID);
					system(delete_files);
				}
				
			}
		}
		close(newsockfd);

		request_status_map[reqID].first=2;
	}
}
