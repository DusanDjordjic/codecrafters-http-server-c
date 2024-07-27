#ifndef REQUEST_H
#define REQUEST_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Possible errors that methods may return
#define ALLOC_FAIL                                      1
#define ERR_REQ_PARSE_FAILED_TO_PARSE_REQ_STATUS_LINE   2
#define ERR_REQ_PARSE_FAILED_TO_PARSE_METHOD            3
#define ERR_REQ_PARSE_UNSUPPORTED_METHOD                4
#define ERR_REQ_PARSE_FAILED_TO_PARSE_PATH              5
#define ERR_REQ_PARSE_INVALID_BUFFER                    6
#define ERR_REQ_PARSE_FAILED_TO_PARSE_HEADERS           7
#define ERR_REQ_PARSE_INVALID_PATH                      8

typedef enum http_method {
    GET,
    POST,
    PUT,
    DELETE, 
    PATCH,
    OPTIONS, 
} HTTP_METHOD;

typedef struct request {
    char* buffer;
    HTTP_METHOD method;
    char* path;
    char* body;
    char** headers;
    uint16_t header_count;
    bool done;
} Request;

// Extracts the method, path, body and headers 
// in separate fields in passed request.
//
// Allocates a new memory for all fields in the request
//
// After you are done with request you must call request_dealloc.
//
// Returns 0 if there are no errors and 1 otherwise
// The request could be bigger than the blob in that case
// you will have to call request_parse muliple times until
// the request is market done
int request_parse(Request* req, char* blob);

// Deallocates memory that was allocated during request_parse function.
//
// Returns 0 if there are no errors and 1 otherwise
int request_dealloc(Request* req);

#endif
