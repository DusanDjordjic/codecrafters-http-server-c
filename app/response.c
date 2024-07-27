#include "response.h"
#include "request.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_STEP 1024

static int realloc_buffer(char** buffer, uint32_t * capacity) {
    if (buffer == NULL) {
        return 2;
    }
    
    char* new = realloc(*buffer, sizeof(char) * (*capacity + BUFFER_STEP));
    if (new == NULL) {
        return 1;
    }

    *capacity += BUFFER_STEP;
    *buffer = new;
    return 0;
}

int response_new(Response* res, HTTP_STATUS status) {
    res->done = false;
    res->buffer = NULL;
    res->buffer_len = 0;
    res->buffer_cap = 0;

    int err = realloc_buffer(&(res->buffer), &(res->buffer_cap));
    if (err != 0) {
        fprintf(stderr, "failed to allocate buffer");
        response_dealloc(res);
        return err;
    }

    strcpy(res->buffer +res->buffer_len, "HTTP/1.1");
    res->buffer_len += 8;

    const char* r = NULL;
    switch (status) {
        case OK:
            r = " 200 OK\r\n"; 
            break;
        case BAD_REQUEST:
            r = " 400 Bad Request\r\n"; 
            break;
        case UNAUTHORIZED:
            r = " 401 Unauthorized\r\n"; 
            break;
        case NOT_FOUND:
            r = " 404 Not Found\r\n"; 
            break;
    }

    strcpy(res->buffer +res->buffer_len, r);
    res->buffer_len += strlen(r);
    return 0;
}


int with_header(Response* res, char* header, char* value) {
    if (res->done) {
        return 1;
    }
    // Check if the header len + buffer len is greater than capacity
    uint32_t header_name_len = strlen(header);
    uint32_t header_value_len = strlen(value);
    // + 1 for ": "
    // + 2 for "\r\n"
    uint32_t total_header_len = header_name_len + 1 + header_value_len + 2;
    if (res->buffer_len + total_header_len > res->buffer_cap) {
        // Realloc buffer
        int err = realloc_buffer(&(res->buffer), &(res->buffer_cap));
        if (err != 0) {
            fprintf(stderr, "failed to allocate buffer for header %s: %s", header, value);
            response_dealloc(res);
            return err;
        }  
    }

    strncpy(res->buffer + res->buffer_len, header, header_name_len);
    res->buffer_len += header_name_len;

    strncpy(res->buffer + res->buffer_len, ":", 1);
    res->buffer_len += 1;

    strncpy(res->buffer + res->buffer_len, value, header_value_len);
    res->buffer_len += header_value_len;

    strncpy(res->buffer + res->buffer_len, "\r\n", 2);
    res->buffer_len += 2;
    return 0;
}

int with_body(Response* res, char* body) {
    if (res->done) {
        return 1;
    }

    uint32_t body_len = strlen(body);
    char body_len_string[12] = {0};
    sprintf(body_len_string, "%u", body_len);

    int err = with_header(res, "Content-Length", body_len_string);
    if (err != 0) {
        fprintf(stderr, "failed to add header to response error %d\n", err);
        response_dealloc(res);
        return err;
    }

    // + 2 for \r\n
    if (res->buffer_len + body_len + 2> res->buffer_cap) {
        // Realloc buffer
        int err = realloc_buffer(&(res->buffer), &(res->buffer_cap));
        if (err != 0) {
            fprintf(stderr, "failed to allocate buffer for body %s", body);
            response_dealloc(res);
            return err;
        }  
    }

    
    strncpy(res->buffer + res->buffer_len, "\r\n", 2);
    res->buffer_len += 2;

    strncpy(res->buffer + res->buffer_len, body, body_len);
    res->buffer_len += body_len;

    res-> done = true;

    return 0;
}

int with_no_body(Response* res) {
    if (res->buffer_len + 2 > res->buffer_cap) {
        // Realloc buffer
        int err = realloc_buffer(&(res->buffer), &(res->buffer_cap));
        if (err != 0) {
            fprintf(stderr, "failed to alloc buffer to add \\r\\n");
            response_dealloc(res);
            return err;
        }  
    }


    strncpy(res->buffer + res->buffer_len, "\r\n", 2);
    res->buffer_len += 2;

    return 0;
}
void response_dealloc(Response* res) {
    if (res->buffer != NULL) {
        free(res->buffer);
        res->buffer = NULL;
    }
}
