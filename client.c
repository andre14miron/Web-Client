#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson/parson.h"

#define SERVER_HOST     "34.254.242.81"
#define SERVER_PORT     8080
#define PAYLOAD_TYPE    "application/json"
#define PATH_REGISTER   "/api/v1/tema/auth/register"
#define PATH_LOGIN      "/api/v1/tema/auth/login"
#define PATH_LOGOUT     "/api/v1/tema/auth/logout"
#define PATH_ACCESS     "/api/v1/tema/library/access"
#define PATH_BOOKS      "/api/v1/tema/library/books"
#define MAX_STDIN       256
#define MAX             1000

char* get_user_infos() {
    char *username;
    char *password;
    char *serialized_string;

    /* Allocate memory for the data */
    username = calloc(MAX_STDIN, sizeof(char));
    password = calloc(MAX_STDIN, sizeof(char));
    serialized_string = calloc(MAX_STDIN, sizeof(char));

    /* Read the username and password */
    printf("username=");
    fgets(username, MAX_STDIN, stdin);
    printf("password=");
    fgets(password, MAX_STDIN, stdin);

    if (strchr(username, ' ') != NULL) {
        return NULL;
    }

    /* Remove the new line from the end of the strings */
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';

    /* Create the JSON object */
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    /* Add the username and password to the JSON object */
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    /* Convert the JSON object to string */
    serialized_string = json_serialize_to_string_pretty(root_value);

    /* Free the memory */
    free(username);
    free(password);
    json_value_free(root_value);

    return serialized_string;
}

char* get_id() {
    char *id;

    /* Allocate memory for the data */
    id = calloc(MAX_STDIN, sizeof(char));

    /* Read the id */
    printf("id=");
    fgets(id, MAX_STDIN, stdin);

    /* Remove the new line from the end of the string */
    id[strlen(id) - 1] = '\0';

    return id;
}

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    char *command;
    char *cookie = NULL;
    char *token = NULL;
    int sockfd;

    while(1) {
        /* Read the input from the user */
        command = calloc(MAX_STDIN, sizeof(char));
        fgets(command, MAX_STDIN, stdin);

        /* Open the connection with the server */
        sockfd = open_connection(SERVER_HOST, SERVER_PORT, PF_INET, SOCK_STREAM, 0);

        /* The program is closing */
        if (!strcmp(command, "exit\n")) {
            /* Close the connection with the server  */
            close_connection(sockfd);

            break;

        /* Perform the registration */
        } else if (!strcmp(command, "register\n")) {
            char *serialized_string;

            /* Get the user infos */
            serialized_string = get_user_infos();
            if (serialized_string == NULL) {
                printf("[ERROR] The username cannot contain spaces!\n");
                continue;
            }

            /* Create the POST request */
            message = compute_post_request(
                SERVER_HOST,
                PATH_REGISTER,
                PAYLOAD_TYPE, 
                &serialized_string, 0, 
                NULL, NULL);

            /* Send the POST request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            /* Check if the username is taken */
            if (strstr(response, "{\"error\":\"The username") != NULL) {
                printf("[ERROR] The username is taken!\n");
                continue;
            } 
            
            /* Print the response */
            printf("User registered successfully!\n");

            json_free_serialized_string(serialized_string);

        /* Perform login */
        } else if (!strcmp(command, "login\n")) {
            char *serialized_string;

            /* Get the user infos */
            serialized_string = get_user_infos();
            if (serialized_string == NULL) {
                printf("[ERROR] The username cannot contain spaces!\n");
                continue;
            }
            
            /* Create the POST request */
            message = compute_post_request(
                SERVER_HOST,
                PATH_LOGIN,
                PAYLOAD_TYPE, 
                &serialized_string, 0, 
                NULL, NULL);

            /* Send the POST request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            /* Check if the credentials do not match */
            if (strstr(response, "{\"error\":\"Credentials are not good!\"}") != NULL) {
                printf("[ERROR] - Log-in failed! The credentials do not match.\n");
                continue;
            } 
                
            /* Print the response */
            printf("Welcome!\n");

            /* Get the cookie */
            cookie = strtok(strstr(response, "Set-Cookie:"), " ");
            cookie = strtok(NULL, ";");

            token = NULL;

            json_free_serialized_string(serialized_string);

        /* Perform logout */
        } else if (!strcmp(command, "logout\n")) {
            /* Create the GET request */
            message = compute_get_request(
                SERVER_HOST,
                PATH_LOGOUT,
                NULL, 
                cookie, token);

            /* Send the request to the server */
            send_to_server(sockfd, message);

            /* Receive the response from the server */
            response = receive_from_server(sockfd);

            /* Check if the logout was successful */
            if (strstr(response, "{\"error\":\"You are not logged in!\"}") != NULL) {
                printf("[ERROR] You are not logged in!\n");
                continue;
            }

            /* Print the response */
            printf("Goodbye!:( You are now logged out\n");

            /* Eliminate the cookie and the token */
            cookie = NULL;
            token = NULL;
            
        /* Request access to the library */
        } else if (!strcmp(command, "enter_library\n")) {
            /* Create the GET request */
            message = compute_get_request(
                SERVER_HOST,
                PATH_ACCESS,
                NULL, 
                cookie, NULL);

            /* Send the request to the server */
            send_to_server(sockfd, message);

            /* Receive the response from the server */
            response = receive_from_server(sockfd);

            /* Check if the access was denied */
            if (strstr(response, "{\"error\":\"You are not logged in!\"}") != NULL) {
                printf("[ERROR] You did not get access to the library.\n");
                continue;
            }
                
            /* Print the response */
            printf("You get access to the library.\n");

            /* Get the token */
            token = strtok(strstr(response, "token"), ":");
            token = strtok(NULL, "\"");

        /* Request all books from the server */
        } else if (!strcmp(command, "get_books\n")) {
            /* Create the GET request */
            message = compute_get_request(
                SERVER_HOST,
                PATH_BOOKS,
                NULL, 
                cookie, token);

            /* Send the request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            /* Check if the client has access to the library */
            if (strstr(response, "{\"error\":\"Authorization header is missing!\"}") != NULL) {
                printf("[ERROR] You do not have access to the library.\n");
                continue;
            }
            
            /* Print the wanted result */
            char *result = strchr(response, '[');
            JSON_Value *root_value = json_parse_string(result);
            JSON_Array *root_array = json_value_get_array(root_value);

            if (json_array_get_count(root_array) == 0) {
                printf("There are no books in the library.\n");
                continue;
            }

            for (int i = 0; i < json_array_get_count(root_array); i++) {
                JSON_Object *root_object = json_array_get_object(root_array, i);
                const int id = json_object_get_number(root_object, "id");
                const char *title = json_object_get_string(root_object, "title");
                
                printf("Book nr. #%d\nid=%d\ntitle=%s\n\n", i + 1, id, title);
            }

            json_value_free(root_value);

        /* Request information about a book */
        } else if (!strcmp(command, "get_book\n")) {
            char *id;

            /* Get the ID from the user */
            id = get_id();

            /* Check if the user entered the ID */
            if (strlen(id) == 0) {
                printf("[ERROR] The ID was not entered.\n");
                continue;
            }

            /* Create the access path */
            char access_path[50] = "/api/v1/tema/library/books/";
            strcat(access_path, id);

            /* Create the GET request */
            message = compute_get_request(
                SERVER_HOST,
                access_path,
                NULL, 
                cookie, token);

            /* Send the request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            /* Check if the client has access to the library */
            if (strstr(response, "{\"error\":\"Authorization header is missing!\"}") != NULL) {
                printf("[ERROR] You do not have access to the library.\n");
                continue;
            }

            /* Check if the ID is valid */
            if (strstr(response, "{\"error\":\"id is not int!\"}") != NULL) {
                printf("[ERROR] The ID is not valid. Try a number.\n");
                continue;
            }

            /* Check if the book was not found */
            if (strstr(response, "{\"error\":\"No book was found!\"}") != NULL) {
                printf("[ERROR] The book was not found.\n");
                continue;
            }

            /* Print the wanted result */
            char *result = strchr(response, '{');
            JSON_Value *root_value = json_parse_string(result);
            JSON_Object *root_object = json_value_get_object(root_value);

            const char *title = json_object_get_string(root_object, "title");
            const char *author = json_object_get_string(root_object, "author");
            const char *genre = json_object_get_string(root_object, "genre");
            const char *publisher = json_object_get_string(root_object, "publisher");
            const int page_count = json_object_get_number(root_object, "page_count");
                
            printf("title=%s\n", title);
            printf("author=%s\n", author);
            printf("genre=%s\n", genre);
            printf("publisher=%s\n", publisher);
            printf("page_count=%d\n", page_count);
            
            json_value_free(root_value);
            free(id);

        /* Add a book */
        } else if (!strcmp(command, "add_book\n")) {
            char *title;
            char *author;
            char *genre;
            char *publisher;
            char *page_count;

            /* Allocate memory for the data */
            title = calloc(MAX_STDIN, sizeof(char));
            author = calloc(MAX_STDIN, sizeof(char));
            genre = calloc(MAX_STDIN, sizeof(char));
            publisher = calloc(MAX_STDIN, sizeof(char));
            page_count = calloc(MAX_STDIN, sizeof(char));

            /* Read the data from the user */
            printf("title=");
            fgets(title, MAX_STDIN, stdin);
            printf("author=");
            fgets(author, MAX_STDIN, stdin);
            printf("genre=");
            fgets(genre, MAX_STDIN, stdin);
            printf("publisher=");
            fgets(publisher, MAX_STDIN, stdin);
            printf("page_count=");
            fgets(page_count, MAX_STDIN, stdin);

            /* Check if the user entered all the data */
            if (strlen(title) == 1 || 
                strlen(author) == 1 || 
                strlen(genre) == 1 || 
                strlen(publisher) == 1 || 
                strlen(page_count) == 1) {
                printf("[ERROR] You did not enter all the data.\n");
                continue;
            }

            /* Remove the \n from the end of the string */
            title[strlen(title) - 1] = '\0';
            author[strlen(author) - 1] = '\0';
            genre[strlen(genre) - 1] = '\0';
            publisher[strlen(publisher) - 1] = '\0';
            page_count[strlen(page_count) - 1] = '\0';

            /* Create the json */
            JSON_Value *json_value = json_value_init_object();;
            JSON_Object *json_object = json_value_get_object(json_value);
            json_object_set_string(json_object, "title", title);
            json_object_set_string(json_object, "author", author);
            json_object_set_string(json_object, "genre", genre);
            json_object_set_string(json_object, "publisher", publisher);
            json_object_set_string(json_object, "page_count", page_count);
            char *serialized_string = json_serialize_to_string_pretty(json_value);

            /* Create the POST request */
            message = compute_post_request(
                SERVER_HOST,
                PATH_BOOKS,
                PAYLOAD_TYPE, 
                &serialized_string, 0, 
                cookie, token);

            /* Send the request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            /* Check if the client has access to the library */
            if (strstr(response, "{\"error\":\"Authorization header is missing!\"}") != NULL) {
                printf("[ERROR] You do not have access to the library.\n");
                continue;
            }

            /* Check if the entered information is incomplete or 
                    does not comply with the formatting.  */
            if (strstr(response, "{\"error\":\"Something Bad Happened\"}") != NULL) {
                printf("[ERROR] The entered information is incomplete or does not comply with the formatting.\n");
                continue;
            }

            /* Print the response */
            printf("The book was succesfully added to the library.\n");

            json_free_serialized_string(serialized_string);
            json_value_free(json_value);
            free(title);
            free(author);
            free(genre);
            free(publisher);
            free(page_count);

        /* Delete a book */
        } else if (!strcmp(command, "delete_book\n")) {
            char *id;

            /* Get the ID from the user */
            id = get_id();

            /* Check if the user entered the ID */
            if (strlen(id) == 0) {
                printf("[ERROR] The ID was not entered.\n");
                continue;
            }

            /* Update the access path */
            char access_path[50] = "/api/v1/tema/library/books/";
            strcat(access_path, id);

            /* Create the DELETE request */
            message = compute_delete_request(
                SERVER_HOST,
                access_path, 
                token);

            /* Send the request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            /* Check if the client has access to the library */
            if (strstr(response, "{\"error\":\"Authorization header is missing!\"}") != NULL) {
                printf("[ERROR] You do not have access to the library.\n");
                continue;
            }

            /* Check if the entered information is incomplete or 
                    does not comply with the formatting.  */
            if (strstr(response, "{\"error\":\"Something Bad Happened\"}") != NULL) {
                printf("[ERROR] The entered information is incomplete or does not comply with the formatting.\n");
                continue;
            }

            /* Check if the book was deleted */
            if (strstr(response, "{\"error\":\"No book was deleted!\"}") != NULL) {
                printf("[ERROR] The book does not exist.\n");
                continue;
            }

            /* Print the response */
            printf("The book was successfully deleted from library\n");

            free(id);
        } else {
            printf("[ERROR] Invalid command.\n");
        }

        /* Close the connection with the server  */
        close_connection(sockfd);

        free(command);
    }

    return 0;
}
