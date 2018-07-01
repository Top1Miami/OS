#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
/*void setnonblockable(int desc) {
	int flag = fcntl(desc, F_GETFL, 0);
	fcntl(desc, F_SETFL, flag | O_NONBLOCK);
}*/

int main(int argc, char * argv[]) {

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	int sin_port = 1347;
	char * name = "local_host";
	if(argc > 1) {
		sin_port = atoi(argv[1]);
	}
	if(argc > 2) {
		name = argv[2];
	}
	server_address.sin_port = htons(sin_port);
	inet_pton(AF_INET, name, &server_address.sin_addr);

	int socket_client = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_client == -1) {
		fprintf(stderr, "Failed to create socket %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	
	//setnonblockable(socket_client);
	int epoller = epoll_create(4);
	if(epoller == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	


	struct epoll_event event;
	struct epoll_event event_array[10];

	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = socket_client;

	int ctl_return_value = epoll_ctl(epoller, EPOLL_CTL_ADD, socket_client, &event);
	if(ctl_return_value == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
	}

	int connection = connect(socket_client, (struct sockaddr *)&server_address, sizeof(server_address));
	if(connection == -1) {
		if(errno == EINPROGRESS) {
			struct epoll_event establish_connection;
			establish_connection.events = EPOLLIN | EPOLLOUT;
			establish_connection.data.fd = socket_client;
			epoll_wait(epoller, &establish_connection, 10, -1);
		} else {
			fprintf(stderr, "Failed to establish connection with server%s\n", strerror(errno));
			return EXIT_FAILURE;	
		}
	}

	for(int j = 0; j < 1;j++) {
		
		int n = epoll_wait(epoller, event_array, 10, -1);

		for(int i = 0; i < n;i++) {
			if(event_array[i].events & EPOLLOUT) {
				char * data = "Yo sobaki";
				printf("Sends message = %s\n", data);
				int data_size = strlen(data);
				char * data_pointer = data;
				while(data_size > 0) {
					ssize_t sender = send(event_array[i].data.fd, data_pointer, data_size, 0);
					if(sender == -1) {
						fprintf(stderr, "Failed to send data to server %s\n", strerror(errno));
						break;
					}
					data_pointer += sender;
					data_size -= sender;	
				}
				shutdown(socket_client, SHUT_WR);
			}
			if(event_array[i].events & EPOLLIN) {
				int current_size = 0, max_size = 75;
				int received = 0;
				char buffer[max_size];
				char * pointer = buffer;
				printf("Receives message = ");
				while((received = recv(event_array[i].data.fd, pointer, max_size, 0)) > 0) {
					pointer += received;
					current_size += received;
					max_size -= received;
					buffer[current_size] = '\0';
					printf("%s\n", buffer);
				}
				if(received == -1)  {
					fprintf(stderr, "Failed to receive data %s\n", strerror(errno));
				}
				if(received == 0) {
					fprintf(stderr, "Connection disabled %s\n", strerror(errno));
				}				
			}
		}
	}

	//pause();
	close(socket_client);
	return EXIT_SUCCESS;
}