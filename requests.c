#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookie, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    /* Write the method name, URL, request params (if any) and protocol type */
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    /* Add the host */
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Add the cookie */
    if (cookie != NULL) {
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }

    /* Add the token */
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    /* Add new line at end of header */
    compute_message(message, "");

    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char *cookie, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    strcpy(body_data_buffer, body_data[0]);
    for (int i = 1; i < body_data_fields_count; i++) {
        strcat(body_data_buffer, "&\0");
        strcat(body_data_buffer, body_data[i]);
    }

    /* Write the method name, URL and protocol type */
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    /* Add the host */
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Add necessary headers (Content-Type and Content-Length are mandatory) */
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    compute_message(message, line);

    /* Add the cookie */
    if (cookie != NULL) {
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }

    /* Add the token */
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    /* Add new line at end of header */
    compute_message(message, "");

    /* Add the actual payload data */
    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    /* Write the method name, URL and protocol type */
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    
    /* Add the host */
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    /* Add the token */
    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    /* Add new line at end of header */
    compute_message(message, "");

    free(line);
    return message;
}
