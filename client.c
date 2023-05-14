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
            char *username;
            char *password;

            /* Allocate memory for the data */
            username = calloc(MAX_STDIN, sizeof(char));
            password = calloc(MAX_STDIN, sizeof(char));

            /* Read the data from the user */
            printf("username=");
            fgets(username, MAX_STDIN, stdin);
            printf("password=");
            fgets(password, MAX_STDIN, stdin);

            /* Remove the \n from the end of the string */
            username[strlen(username) - 1] = '\0';
            password[strlen(password) - 1] = '\0';

            /* Create the JSON object */
            JSON_Value *init_object;
            JSON_Object *json_object;
            init_object = json_value_init_object();
            json_object = json_value_get_object(init_object);
            json_object_set_string(json_object, "username", username);
            json_object_set_string(json_object, "password", password);
            char *serialized_json = json_serialize_to_string_pretty(init_object);

            /* Create the POST request */
            message = compute_post_request(
                SERVER_HOST,
                PATH_REGISTER,
                PAYLOAD_TYPE, 
                &serialized_json, 0, 
                cookie, token);

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

            free(username);
            free(password);

        /* Perform login */
        } else if (!strcmp(command, "login\n")) {
            char *username;
            char *password;

            /* Allocate memory for the data */
            username = calloc(MAX_STDIN, sizeof(char));
            password = calloc(MAX_STDIN, sizeof(char));

            /* Read the data from the user */
            printf("username=");
            fgets(username, MAX_STDIN, stdin);
            printf("password=");
            fgets(password, MAX_STDIN, stdin);

            /* Remove the \n from the end of the string */
            username[strlen(username) - 1] = '\0';
            password[strlen(password) - 1] = '\0';

            /* Create the JSON object */
            JSON_Value *init_object;
            JSON_Object *json_object;
            init_object = json_value_init_object();
            json_object = json_value_get_object(init_object);
            json_object_set_string(json_object, "username", username);
            json_object_set_string(json_object, "password", password);
            char *serialized_json = json_serialize_to_string_pretty(init_object);
            
            /* Create the POST request */
            message = compute_post_request(
                SERVER_HOST,
                PATH_LOGIN,
                PAYLOAD_TYPE, 
                &serialized_json, 0, 
                cookie, token);

            /* Send the POST request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            printf("%s\n", response);

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

            free(username);
            free(password);

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

            printf("%s\n", response);

            /* Check if the logout was successful */
            if (strstr(response, "{\"error\":\"You are not logged in!\"}") != NULL) {
                printf("[ERROR] Log out failed!\n");
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
                cookie, token);

            /* Send the request to the server */
            send_to_server(sockfd, message);

            /* Receive the response from the server */
            response = receive_from_server(sockfd);

            printf("RESPONSE: %s\n", response);

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

            printf("RESPONSE: %s\n", response);

            /* Check if the client has access to the library */
            if (strstr(response, "{\"error\":\"Authorization header is missing!\"}") != NULL) {
                printf("[ERROR] You do not have access to the library.\n");
                continue;
            }
            
            /* Print the wanted result */
            char *result = strtok(response, "[");
            result = strtok(NULL, "]");
            printf("[%s]\n", result);

        /* Request information about a book */
        } else if (!strcmp(command, "get_book\n")) {
            char *id;

            /* Allocate memory for the data */
            id = calloc(MAX_STDIN, sizeof(char));

            /* Read the data from the user */
            printf("id=");
            fgets(id, MAX_STDIN, stdin);

            /* Remove the \n from the end of the string */
            id[strlen(id) - 1] = '\0';

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

            printf("RESPONSE: %s\n", response);

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

            /* Check if the book was found */
            if (strstr(response, "{\"error\":\"No book was found!\"}") != NULL) {
                printf("[ERROR] The book was not found.\n");
                continue;
            }

            /* Print the wanted result */
            char *result = strtok(response, "{");
            result = strtok(NULL, "}");
            printf("{%s}\n", result);

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

            /* Remove the \n from the end of the string */
            title[strlen(title) - 1] = '\0';
            author[strlen(author) - 1] = '\0';
            genre[strlen(genre) - 1] = '\0';
            publisher[strlen(publisher) - 1] = '\0';
            page_count[strlen(page_count) - 1] = '\0';

            /* Create the json */
            JSON_Value *init_object;
            JSON_Object *json_object;
            init_object = json_value_init_object();
            json_object = json_value_get_object(init_object);
            json_object_set_string(json_object, "title", title);
            json_object_set_string(json_object, "author", author);
            json_object_set_string(json_object, "genre", genre);
            json_object_set_string(json_object, "publisher", publisher);
            json_object_set_string(json_object, "page_count", page_count);
            char *serialized_json = json_serialize_to_string_pretty(init_object);

            /* Create the POST request */
            message = compute_post_request(
                SERVER_HOST,
                PATH_BOOKS,
                PAYLOAD_TYPE, 
                &serialized_json, 0, 
                cookie, token);

            /* Send the request */
            send_to_server(sockfd, message);

            /* Receive the response */
            response = receive_from_server(sockfd);

            printf("RESPONSE: %s\n", response);

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

            free(title);
            free(author);
            free(genre);
            free(publisher);
            free(page_count);

        /* Delete a book */
        } else if (!strcmp(command, "delete_book\n")) {
            char *id;

            /* Allocate memory for the data */
            id = calloc(MAX_STDIN, sizeof(char));

            /* Read the data from the user */
            printf("id=");
            fgets(id, MAX_STDIN, stdin);

            /* Remove the \n from the end of the string */
            id[strlen(id) - 1] = '\0';

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

            printf("RESPONSE: %s\n", response);

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
