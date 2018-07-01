#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>

int main(int argc, char * argv[]) {
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(1347);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if(argc > 2) {
		server_address.sin_port = htons(atoi(argv[2]));
	}
	if(argc > 1) {
		int atoner = inet_aton(argv[1], &server_address.sin_addr);
		if(atoner == 0) {
			fprintf(stderr, "Failed to convert to Ip address %s\n", strerror(errno));
			server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		}
	}
	int socket_listener = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_listener == -1) {
		fprintf(stderr, "Failed to initialize listener socket %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	int binder = bind(socket_listener, (struct sockaddr *)&server_address, sizeof(server_address));
	if(binder < 0) {
		fprintf(stderr, "Failed to bind socket %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	int size_queue = 16;
	int listener = listen(socket_listener, size_queue);
	if(listener == -1) {
		fprintf(stderr, "Failed to start socket for listening %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	struct sockaddr_in client_address;
	int cl_len = 0;
	while(1) {
		int temp_socket = accept(socket_listener, (struct sockaddr *)&client_address, (socklen_t *)&cl_len);
		if(temp_socket == -1) {
			fprintf(stderr, "Failed to establish connection with client %s\n", strerror(errno));
			continue;
		}
		/*struct timeval tv;
		tv.tv_sec = 3;
		int set_result = setsockopt(temp_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
		if(set_result == -1) {
			fprintf(stderr, "Failed to set time limit %s\n", strerror(errno));
		}*/
		int current_size = 0, max_size = 75;
		int received = 0;
		char buffer[max_size];
		char * pointer = buffer;
		while((received = recv(temp_socket, pointer, max_size, 0)) > 0) {
			current_size += received;
			pointer += received;
			max_size -= received;
			printf("Recieves message = %s\n", buffer);
		}
		
		printf("Sends message = %s\n", buffer);
		int buffer_size = current_size;
		char * buffer_pointer = buffer;
		while(buffer_size > 0) {
			ssize_t sender = send(temp_socket, buffer, current_size, 0);
			if(sender == -1) {
				fprintf(stderr, "Failed to send data to server %s\n", strerror(errno));
				break;
			}
			buffer_pointer += sender;
			buffer_size -= sender;	
		}
		if(received == -1) {
			close(temp_socket);
			continue;
		}
		if(received == 0) {
			fprintf(stderr, "Connection disabled %s\n", strerror(errno));
		}

		close(temp_socket);
	}

	close(socket_listener);
	return EXIT_SUCCESS;
}