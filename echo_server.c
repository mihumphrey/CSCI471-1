#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <errno.h>
#include <string.h>

int main(int argc, char **argv) {

	int socket(int domain, int type, int protocol);

	int lfd = -1;

	if((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("failed to create listening socket %s", strerror(errno));
		exit(-1);
	}
	
	
}
