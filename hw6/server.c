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
#include <sys/epoll.h>
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
	int max_size_epoll = 20;
	int epoller = epoll_create(max_size_epoll);
	if(epoller == -1) {
		fprintf(stderr, "%s\n", strerror(errno));
	}
	
	struct epoll_event epoll_event_array[max_size_epoll];
	memset(&epoll_event_array, 0, sizeof(epoll_event_array));

	while(1) {
		struct sockaddr_storage client_addr;
		socklen_t client_addr_length = sizeof(client_addr);
		int socket_client = accept(socket_listener, (struct sockaddr *)&client_addr, &client_addr_length);
		if(socket_client == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			close(socket_listener);
			return EXIT_FAILURE;
		}
		static struct epoll_event event;
		int client_socket;
		event.events = EPOLLIN | EPOLLOUT;
		event.data.fd = socket_client;
		int eventer = epoll_ctl(epoller, EPOLL_CTL_ADD, socket_client, &event);
		if(eventer == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			continue;
		}

		int ready_steady = epoll_wait(epoller, epoll_event_array, max_size_epoll, -1);
		if(ready_steady == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		
		for(int i = 0; i < ready_steady;i++) {
			if(epoll_event_array[i].events & EPOLLIN) {	
				int current_size = 0, max_size = 75;
				int received = 0;
				char buffer[max_size];
				char * pointer = buffer;
				while((received = recv(epoll_event_array[i].data.fd, pointer, max_size, 0)) > 0) {
					current_size += received;
					pointer += received;
					max_size -= received;
					buffer[current_size] = '\0';
					printf("Recieves message = %s\n", buffer);
				}
				shutdown(epoll_event_array[i].data.fd, SHUT_RD);
			}
			if(epoll_event_array[i].events & EPOLLOUT) {
				char * data = "Yo sobaki";
				printf("Sends message = %s\n", data);
				int data_size = strlen(data);
				char * data_pointer = data;
				while(data_size > 0) {
					ssize_t sender = send(epoll_event_array[i].data.fd, data_pointer, data_size, 0);
					if(sender == -1) {
						fprintf(stderr, "%s\n", strerror(errno));
						break;
					}
					//printf("lol\n");
					data_pointer += sender;
					data_size -= sender;	
				}
				//printf("dick\n");
				shutdown(epoll_event_array[i].data.fd, SHUT_WR);
			}
		}
		close(socket_client);
	}
	close(socket_listener);
}