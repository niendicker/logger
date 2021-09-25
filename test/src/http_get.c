
#include <stdio.h>
#include <stdlib.h>     //malloc()
#include <unistd.h>     //read() and write()
#include <string.h>	   //strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <netdb.h>	   //hostent
#include <assert.h>

//Receive a hostname and try to get and return the IP address
char * host_to_ip(char* host_name){
   //char *hostname = "www.google.com";
	char* ip = (char*)malloc(sizeof(char)*100);
	struct in_addr **addr_list;
   printf("%s", host_name);
   struct hostent *he = gethostbyname(host_name);
		
	if(he == NULL) return NULL;
	
	//Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(int i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
	}
	return ip;
}

int main(int argc , char *argv[])
{
   //Create socket
	int connectionPipe = socket(AF_INET , SOCK_STREAM , 0);
	assert(connectionPipe != (-1));

	//Server
   //const int server_port = mbDevice->link.modbusTcp.port;
   //Remove new line from hostname for host_to_ip working correctly
   char* host_name = "www.google.com";
   assert(host_name != NULL);

   char* host_ip_addr = host_to_ip(host_name);
   if(host_ip_addr == NULL){
      printf("Could not resolve host name %s!\n", host_name);
      return -1;
   }

   //Configure server
   struct sockaddr_in server;
	server.sin_family = AF_INET;
   server.sin_addr.s_addr = inet_addr(host_ip_addr);
	server.sin_port = htons(80);

	//Connect to remote server
	if (connect(connectionPipe , (struct sockaddr *)&server , sizeof(server)) < 0){
		printf("Could not connect to IP: %s\n", host_ip_addr);
		return -1;
	}
   else
	   printf("Connected to %s at ip: %s \n", host_name, host_ip_addr);
	
	//Send some data
   char message[] = "GET / HTTP/1.1\r\n\r\n";

	if(write(connectionPipe , message , strlen(message)) < 0){
		puts("Could not send HTTP GET method...");
		return 1;
	}
   else
	   puts("Send HTTP GET. Waitting for reply...");
	
   //Receive a reply from the server
	char server_reply[2000];

   if( read(connectionPipe, server_reply , 2000) < 0){
		puts("Receive failed");
      return 1;
	}
	
   puts("\nReply received:\n");
	puts(server_reply);

   //Free mem and files
   free(host_ip_addr);
   close(connectionPipe);

	return 0;
}

