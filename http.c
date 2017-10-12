#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "http.h"

#define BUF_SIZE 1024

int create_socket(void){			//return a socket
	int s = socket(AF_INET, SOCK_STREAM, 0);
	
	if (s == -1){					//error check
		perror("create socket");
		exit(1);
	}
	
	return s;
}

Buffer* get_response(int s){		//read data from the given socket and return it
	int received = 0;
	char temp_buf[BUF_SIZE];
	memset(temp_buf, 0, BUF_SIZE);
	Buffer *buffer = (Buffer *) calloc(1, sizeof(Buffer));
	
	while ((received = recv(s, temp_buf, BUF_SIZE, 0)) > 0){
		if (received < 0){			//receiving error check
			perror("receive");
			exit(1);
		}
		else{
			buffer->data = realloc(buffer->data, buffer->length + received);	//always give enough memory space
			memcpy(buffer->data + buffer->length, temp_buf, received);			//using memcpy to avoid '\0' error
			buffer->length += received;
		}
		memset(temp_buf, 0, BUF_SIZE);		//clean temp buffer
	}
	return buffer;
}

Buffer* http_query(char *host, char *page, int port) {
	char port_n[20];	//getaddrinfo needs port stored as char
	int fragment = 0;
	int fragment_recv = 0;
	int client_socket = create_socket();
	struct addrinfo hints, *server_address = NULL;
	char* request = (char*)malloc(63 + strlen(host) + strlen(page));	//63 is the length of the original GET request packet
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(port_n, "%d", port);	//as i said, be a char
	getaddrinfo(host, port_n, &hints, &server_address);
	
	int rc = connect(client_socket, server_address->ai_addr, server_address->ai_addrlen);
	if (rc < 0){
		perror("connect");
		exit(1);
	}
	//modify GET request packet
	sprintf(request, "GET %s%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: downloader/1.0\r\n\r\n", page[0] == '/' ? "" : "/", page, host);	
	
	while (fragment < strlen(request)){		//make sure the packet is completely sent
		fragment_recv = send(client_socket, request + fragment, strlen(request) - fragment, 0);
		if (fragment_recv < 0){				//sending error check
			perror("send");
			exit(1);
		}
		fragment += fragment_recv;
	}
	
	Buffer *buffer = get_response(client_socket);	//read response from server
	
	close(client_socket);	//all allocated resources need to been free'd
	free(request);
	freeaddrinfo(server_address);	
	return buffer;
} //36 lines

// split http content from the response string
char* http_get_content(Buffer *response) {
  
  char* header_end = strstr(response->data, "\r\n\r\n");  
    
  if(header_end) {
    return header_end + 4;
  } else {
   return response->data; 
  }
}


Buffer *http_url(const char *url) {
  char host[BUF_SIZE];
  strncpy(host, url, BUF_SIZE);
  
  char *page = strstr(host, "/");
  if(page) {
    page[0] = '\0';
  
    ++page;
    return http_query(host, page, 80);
  } else {
    
    fprintf(stderr, "could not split url into host/page %s\n", url);
    return NULL;
  }
}

