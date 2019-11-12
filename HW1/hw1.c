#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include<sys/wait.h>
#include<fcntl.h>
#include<sys/sendfile.h>

#define SIZE 1024
#define BACKLOG 10

char webpage[8192]="HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>NetWork_programming_homework1</title></head>\r\n"
"<body><center> upload Image <br>\r\n"
"<img src=\"Picture.jpeg\" alt=\"Revealing\" title=\"Demo picture\" width=\"200px\">\r\n"
"<form method=\"post\" enctype=\"multipart/form-data\">\r\n"
"<p><input type=\"file\" name=\"upload\"></p>\r\n"
"<p><input type=\"submit\" value=\"submit\"></p>\r\n"
"</center>\r\n"
"</body></html>\r\n";

void report(struct sockaddr_in *serverAddress);
void process(int clientSocket);
void print_info(int fdsock, int which);
void setHttpHeader(FILE *fptr_html);

static void sigchld_handler(int sig)
{
	int retval;
	if(sig==SIGCHLD){
		waitpid(0, &retval, WNOHANG);
		//printf("CATCH SIGNAL PID=%d\n", getpid());
	}
}


int main(void)
{
	int serverSocket;
	struct sockaddr_in serverAddress;
	int yes=1;
	int clientSocket;
	struct sigaction sa;       						        // signal	
	//FILE *fptr_html = fopen("index.html", "r");
	//setHttpHeader(fptr_html);
	//fclose(fptr_html);

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);                                // create the socket
	if(serverSocket<0){
		fprintf(stderr, "Error: socket\n");
		exit(EXIT_FAILURE);
	}

	bzero(&serverAddress, sizeof(serverAddress));                                  // initialize structure server
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8002);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1){ //prevent shutdown server and cannot
		fprintf(stderr, "Error: setsockopt\n");                                //create the new socket with same ip 
		exit(EXIT_FAILURE);
	}

	if(bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))<0){
		fprintf(stderr, "Error: bind\n");
		exit(EXIT_FAILURE);
	}

	int listening = listen(serverSocket, BACKLOG);
	if (listening < 0) {
		fprintf(stderr, "Error: The server is not listening.\n");
		exit(EXIT_FAILURE);
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL)==-1){                                          // prevent zombie process
		fprintf(stderr, "Error: sigaction\n");
		exit(EXIT_FAILURE);
	}
	//signal(SIGCHLD, sig_handler);

	print_info(listening, 1);
	while(1) {
		clientSocket = accept(serverSocket, NULL, NULL);
		pid_t child = fork();
		if(child==0){
			close(listening);
			print_info(clientSocket, 2);
			process(clientSocket);
			close(clientSocket);
			printf("connected socket is closing....\n");
			exit(0);	
		}
		close(clientSocket);
	}
	return 0;
}

void process(int clientSocket)
{
	char buf[65536];
	char filename[1024]="DB/";
	char filetype[128];
	char temp_filename[1024];
	char temp_index[128];
	char *temp;
	char *temp1;
	int fdimg;
	char *line;
	int len;
	int count=0;
	FILE *fptr;
	int name_index=0;

	memset(buf, 0, sizeof(buf));
	read(clientSocket, buf, sizeof(buf));

	if(strncmp(buf, "GET /Picture.jpeg", 16)==0){                       // picture
		fdimg = open("Picture.jpeg", O_RDONLY);
		sendfile(clientSocket, fdimg, NULL, 5000000);
		close(fdimg);
	}
	else if(strncmp(buf, "POST /", 6)==0){                              // file
		temp = strstr(buf, "filename");
		if(temp!=NULL){
			len = strlen("filename");
			temp = temp + len + 2;
			count=3;
			while(*temp!='\"'){
				filename[count++]=*temp;
				temp++;
			}
			++temp;
			filename[count]='\0';
			if(count>3){
				printf("receive filename: %s\n", filename);
				while(!(*(temp-4)=='\r' && *(temp-3)=='\n' && *(temp-2)=='\r' && *(temp-1)=='\n')) ++temp;   //file begin

				temp1=buf+strlen(buf)-3;
				while(*temp1!='\r') --temp1;
				--temp1;
				if(access(filename, 0)<0){                    // when file is not exist
					fptr = fopen(filename, "w");
					fwrite(temp, 1, temp1-temp+1, fptr);
					fclose(fptr);
				}
				else{
					name_index=1;
					line = strtok(filename, ".");
					strcpy(filename, line);
				       	line = strtok(NULL, ".");
					strcpy(filetype, line);	
					while(1){
						strcpy(temp_filename, filename);
						sprintf(temp_index, "%d", name_index);
						strcat(temp_filename, "_");
						strcat(temp_filename, temp_index);
						strcat(temp_filename, ".");
						strcat(temp_filename, filetype);
						if(access(temp_filename, 0)<0){
							fptr = fopen(temp_filename, "w");
							fwrite(temp, 1, temp1-temp+1, fptr);
							fclose(fptr);
							break;
						}
						++name_index;	
					}
				}
			}
		}
		else printf("Cannot find\n");
		write(clientSocket, webpage, sizeof(webpage));		
	}
	else //if(strncmp(buf, "GET /", 5)==0)                               // html
		write(clientSocket, webpage, sizeof(webpage));
}

void setHttpHeader(FILE *fptr_html)
{
	char line[128];
	char reponsecontent[1024];
	while(fgets(line, 128, fptr_html)!=NULL){
		strcat(reponsecontent, line);
	}
	strcat(webpage, reponsecontent);
}

void print_info(int fdsock, int which)
{
	socklen_t addresslen;
	struct sockaddr_in address;
	switch(which){
		case 1:
			printf("\n\tServer listen address: ");
			break;
		case 2:
			printf("Connected server address: ");
			break;	
	}
	getsockname(fdsock, (struct sockaddr *)&address, &addresslen);
	printf("%s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port)); 
}

