#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1 ) {
		printf("failed to create socket: %s\n", strerror(errno));
		return 1;
	}

	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s\n", strerror(errno));
		return 1;
	}

	struct sockaddr_in server_address = {
		.sin_family = AF_INET,
		.sin_port = htons(4221),
		.sin_addr = htonl(INADDR_ANY),
	};

	if (bind(server_fd, (struct sockaddr *) &server_address, sizeof(server_address)) != 0) {
		printf("failed to bind socket: %s\n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("failed to listen: %s\n", strerror(errno));
		return 1;
	}

	printf("waiting for client connection...\n");
	

	struct sockaddr_in client_address;
	int client_address_len = sizeof(client_address);  

	int client_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_address_len);
	printf("client connected\n");

	// char buffer[1024] = {0};
	// int read_num = recv(client_socket, buffer, sizeof(buffer), 0);
	// if (read_num == 0) {
	// 	printf("EOF\n");
	// } else if (read_num == -1) {
	// 	printf("failed to read data from socket: %s\n", strerror(errno));
	// 	return -1;
	// }

	// printf("Received %d %s", read_num, buffer);

	char* response_ok = "HTTP/1.1 200 OK\r\n\r\n";
	if (send(client_socket, response_ok, strlen(response_ok), 0) == -1) {
		printf("failed to write data to client: %s\n", strerror(errno));
		return -1;
	}

	close(server_fd);

	return 0;
}
