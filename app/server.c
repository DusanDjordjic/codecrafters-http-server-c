#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "request.h"
#include "response.h"

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
	socklen_t client_address_len = sizeof(client_address);  

	int client_socket = accept(server_fd, (struct sockaddr *) &client_address, &client_address_len);
	printf("client connected\n");

	char buffer[1024] = {0};
	int read_num = recv(client_socket, buffer, sizeof(buffer), 0);
	if (read_num == 0) {
		printf("EOF\n");
	} else if (read_num == -1) {
		printf("failed to read data from socket: %s\n", strerror(errno));
		return -1;
	}


    Request req = {0};
    int err = request_parse(&req, buffer);
    if (err != 0) {
        close(client_socket);
		close(server_fd);
        exit(err);
    }


    if (req.method != GET) {
        Response res;
        err = response_new(&res, NOT_FOUND);
        if (err != 0) {
            fprintf(stderr, "failed to create new resonse ok error %d\n", err);
            goto SEND_SERVER_ERROR;
        }

        err = response_end(&res);
        if (err != 0) {
            fprintf(stderr, "failed to end response error %d\n", err);
            response_dealloc(&res);
            goto SEND_SERVER_ERROR;
        }

        if (send(client_socket, res.buffer, res.buffer_len, 0) == -1) {
            response_dealloc(&res);
            fprintf(stderr, "failed to write data to client: %s\n", strerror(errno));
            goto DONE;
        }
        response_dealloc(&res);
    }


	if (req.path[0] == '/') {
        // split path into chunks
	    if (strlen(req.path) == 1) {
            Response res;
            err = response_new(&res, OK);
            if (err != 0) {
                fprintf(stderr, "failed to create new resonse ok error %d\n", err);
                goto SEND_SERVER_ERROR;
            }

            err = response_end(&res);
            if (err != 0) {
                fprintf(stderr, "failed to end response error %d\n", err);
                response_dealloc(&res);
                goto SEND_SERVER_ERROR;
            }

			if (send(client_socket, res.buffer, res.buffer_len, 0) == -1) {
				fprintf(stderr, "failed to write data to client: %s\n", strerror(errno));
			}

            response_dealloc(&res);
            goto DONE;
		} else {
            printf("PATH: %s\n", req.path);
            char* token = strtok(req.path, "/");
            if (token == NULL) {
                fprintf(stderr, "failed to tokenize path");
                goto SEND_SERVER_ERROR;
            }
            
            if (strncmp(token, "echo", 4) == 0) {

                token = strtok(NULL, "/");

                Response res;
                err = response_new(&res, OK);
                if (err != 0) {
                    fprintf(stderr, "failed to create new respnse ok error %d\n", err);
                    goto SEND_SERVER_ERROR;
                }

                err = with_header(&res, "Content-Type", "text/plain");
                if (err != 0) {
                    fprintf(stderr, "failed to add header resonse ok error %d\n", err);
                    response_dealloc(&res);
                    goto SEND_SERVER_ERROR;
                }
        
                err = with_body(&res, token);
                if (err != 0) {
                    fprintf(stderr, "failed to add body %d\n", err);
                    response_dealloc(&res);
                    goto SEND_SERVER_ERROR;
                }    

                if (send(client_socket, res.buffer, res.buffer_len, 0) == -1) {
                    fprintf(stderr, "failed to write data to client: %s\n", strerror(errno));
                }

                response_dealloc(&res);
                goto DONE;
            }


        }
	}

    goto SEND_NOT_FOUND;

SEND_SERVER_ERROR:
    fprintf(stderr, "Sending server error"); 
	char* response_internal = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
	if (send(client_socket, response_internal, strlen(response_internal), 0) == -1) {
		fprintf(stderr, "failed to write data to client: %s\n", strerror(errno));
	}
    goto DONE;

SEND_NOT_FOUND:
    fprintf(stderr, "Sending not found"); 
	char* response_not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
	if (send(client_socket, response_not_found, strlen(response_not_found), 0) == -1) {
		fprintf(stderr, "failed to write data to client: %s\n", strerror(errno));
	}
    goto DONE;

DONE:
    request_dealloc(&req);
    close(client_socket);
	close(server_fd);

	return 0;
}
