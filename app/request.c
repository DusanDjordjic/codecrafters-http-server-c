#include "request.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int request_parse(Request *req, char *buffer) {
    // copy the buffer and then parse it
    char* request_buffer = malloc(sizeof(char) * strlen(buffer));
    if (request_buffer == NULL) {
        fprintf(stderr, "passed buffer is NULL\n");
        request_dealloc(req);
        return ALLOC_FAIL;
    }
    strcpy(request_buffer, buffer);
    req->buffer = request_buffer;

    // GET /echo/abc HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\nAccept: */*\r\n\r\n

	char* request_line;
	request_line = strtok(req->buffer, "\r\n");
	if (request_line == NULL) {
		fprintf(stderr, "failed to read request line\n");
        request_dealloc(req);
		return ERR_REQ_PARSE_FAILED_TO_PARSE_REQ_STATUS_LINE;
	}

    char** headers = malloc(sizeof(char*) * 10);
    uint16_t headers_count = 0;
    uint16_t header_cap = 10;
    char* token = strtok(NULL, "\r\n");
    while (token != NULL) {
        if (headers_count == header_cap) {
            header_cap += 10;
            char** new_headers = realloc(headers, sizeof(char*) * header_cap);
            if (new_headers == NULL) {
                fprintf(stderr, "Failed to reallocate headers %s\n", strerror(errno));
                request_dealloc(req);
                return ERR_REQ_PARSE_UNSUPPORTED_METHOD;
            }

            headers = new_headers;
        }

        char* header = token;
        token = strtok(NULL, "\r\n");
        headers[headers_count] = header;
        headers_count++;
    }
    
    // The last header is a body
    // headers_count--;
    // req->body = headers[headers_count];
    // headers[headers_count] = NULL;

    req->header_count = headers_count;



	char* method = strtok(request_line, " ");
	if (strncmp(method, "GET", 3) == 0) {
        req->method = GET;
	} else if (strncmp(method, "POST", 4) == 0) {
        req->method = POST;
	} else if (strncmp(method, "PUT", 3) == 0) {
        req->method = PUT;
	} else if (strncmp(method, "PATCH", 5) == 0) {
        req->method = PATCH;
	} else if (strncmp(method, "DELETE", 6) == 0) {
        req->method = DELETE;
	} else if (strncmp(method, "OPTIONS", 7) == 0) {
        req->method = OPTIONS;
	} else {
		fprintf(stderr, "unspoorted method %s\n", method);
        request_dealloc(req);
		return ERR_REQ_PARSE_UNSUPPORTED_METHOD;
    }
    

	char* path = strtok(NULL, " ");
    if (path == NULL) {
        fprintf(stderr, "failed to parse path");
        request_dealloc(req);
        return ERR_REQ_PARSE_INVALID_PATH;
    }
    req->path = path;
    req->done = true;

    printf("method %d\n", req->method);
    printf("path \"%s\"\n", req->path);
    printf("header count %d\n", req->header_count);
    for (uint16_t i = 0; i < req->header_count; i++) {
        printf("header \"%s\"\n", headers[i]);
    }
    printf("body \"%s\"\n", req->body);
    return 0;
}


int request_dealloc(Request* req) {
    if (req == NULL) {
        fprintf(stderr, "request is NULL");
        return 1;
    }

    if (req->headers != NULL) {
        free(req->headers);
    }
    
    if (req->buffer) {
        free(req->buffer);
    }

    return 0;
}
