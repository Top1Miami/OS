#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

	int connection = connect(socket_client, (struct sockaddr *)&server_address, sizeof(server_address));
	if(connection == -1) {
		fprintf(stderr, "Failed to establish connection with server%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	char * data = "Yo sobaki";
	printf("Sends message = %s\n", data);
	int data_size = strlen(data);
	char * data_pointer = data;
	while(data_size > 0) {
		ssize_t sender = send(socket_client, data_pointer, data_size, 0);
		if(sender == -1) {
			fprintf(stderr, "Failed to send data to server %s\n", strerror(errno));
			break;
		}
		data_pointer += sender;
		data_size -= sender;	
	}
	shutdown(socket_client, SHUT_WR);
	int current_size = 0, max_size = 75;
	int received = 0;
	char buffer[max_size];
	char * pointer = buffer;
	printf("Receives message = ");
	while((received = recv(socket_client, pointer, max_size, 0)) > 0) {
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

	close(socket_client);
	return EXIT_SUCCESS;
}