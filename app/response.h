#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum http_status {
    OK = 200,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    NOT_FOUND = 404,
} HTTP_STATUS;

typedef struct response {
    char* buffer;
    uint32_t buffer_len;
    uint32_t buffer_cap;

    HTTP_STATUS status;
    char** headers;
    char* body;
    bool done;
} Response;

int response_new(Response* res, HTTP_STATUS status);
int with_header(Response* res, char* header, char* value);
int with_body(Response* res, char* body);
int response_end(Response* res);
void response_dealloc(Response* res);
#endif
