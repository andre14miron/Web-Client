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

#define SERVER_HOST "34.254.242.81"
#define SERVER_PORT 8080
#define MAX_STDIN 256
#define MAX 1000

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    char *command;
    int sockfd;
    int flag_login = 0;
    char **cookies = NULL;
    int cookies_count = 0;
    char *token = NULL;

    cookies = calloc(MAX, sizeof(*cookies));
    //token = calloc(MAX, sizeof(char));

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

            /* Initialize the buffers */
            username = calloc(MAX_STDIN, sizeof(char));
            password = calloc(MAX_STDIN, sizeof(char));

            /* Get the data from the user */
            printf("username=");
            fgets(username, MAX_STDIN, stdin);
            printf("password=");
            fgets(password, MAX_STDIN, stdin);

            /* Remove the '\n' */
            username[strlen(username)-1] = '\0';
            password[strlen(password)-1] = '\0';

            /* Create the json */
            JSON_Value *init_object;
            JSON_Object *json_object;
            init_object = json_value_init_object();
            json_object = json_value_get_object(init_object);
            json_object_set_string(json_object, "username", username);
            json_object_set_string(json_object, "password", password);

            /*json_object j = {
                {"username", username},
                {"password", password},
            };*/

            char *serialized_json = json_serialize_to_string_pretty(init_object);
            message = compute_post_request(
                SERVER_HOST,
                "/api/v1/tema/auth/register",
                "application/json", 
                &serialized_json, 0, NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *ptr = strstr(response, "error");
            if (ptr != NULL) {
                printf("400 - Error - The username is taken!\n");
            } else {
                printf("201 - OK - User registered successfully!\n");
            }

            free(username);
            free(password);

        /* Perform login */
        } else if (!strcmp(command, "login\n")) {
            char *username;
            char *password;

            if (flag_login == 1) {
                printf("You are already logged in\n");
                continue;
            }

            /* Initialize the buffers */
            username = calloc(MAX_STDIN, sizeof(char));
            password = calloc(MAX_STDIN, sizeof(char));

            /* Get the data from the user */
            printf("username=");
            fgets(username, MAX_STDIN, stdin);
            printf("password=");
            fgets(password, MAX_STDIN, stdin);

            /* Remove the '\n' */
            username[strlen(username)-1] = '\0';
            password[strlen(password)-1] = '\0';

            /* Create the json */
            JSON_Value *init_object;
            JSON_Object *json_object;
            init_object = json_value_init_object();
            json_object = json_value_get_object(init_object);
            json_object_set_string(json_object, "username", username);
            json_object_set_string(json_object, "password", password);
            
            char *serialized_json = json_serialize_to_string_pretty(init_object);
            message = compute_post_request(
                SERVER_HOST,
                "/api/v1/tema/auth/login",
                "application/json", 
                &serialized_json, 0, NULL, 0, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("%s\n", response);

            char *ptr = strstr(response, "error");
            if (ptr != NULL) {
                printf("400 - Error - Login!\n");
            } else {
                printf("201 - OK - Welcome!\n");

                char *cookie = strtok(strstr(response, "Set-Cookie:"), " ");
                cookie = strtok(NULL, ";");

                cookies[cookies_count] = cookie;
                cookies_count++;

                flag_login = 1;
            }

            free(username);
            free(password);

        /* Perform logout */
        } else if (!strcmp(command, "logout\n")) {
            /*  */
            if (flag_login == 0) {
                printf("Error: Nu este logat ca sa dai logout\n");
                continue;
            }

            message = compute_get_request(
                SERVER_HOST,
                "/api/v1/tema/auth/logout",
                NULL, 
                cookies, cookies_count, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("%s\n", response);

            char *ptr = strstr(response, "error");
            if (ptr == NULL) {
                printf("LOGOUT SUCCESFULLY!\n");
            }

            flag_login = 0;
            
        /* Request access to the library */
        } else if (!strcmp(command, "enter_library\n")) {
            /*  */
            if (flag_login == 0) {
                printf("Error: Nu este logat ca sa  primesti acces\n");
                continue;
            } 

            message = compute_get_request(
                SERVER_HOST,
                "/api/v1/tema/library/access",
                NULL, 
                cookies, cookies_count, NULL);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            char *ptr = strstr(response, "error");
            if (ptr == NULL) {
                printf("ACCES LIBRBARY!\n");
            
                printf("MIAU: %s\n", response);

                token = strtok(strstr(response, "token"), ":");
                token = strtok(NULL, "\"");
                printf("OKUR: %s\n", token);
            }


        /* Request all books from the server */
        } else if (!strcmp(command, "get_books\n")) {
            if (flag_login == 0) {
                printf("Error: Nu este logat ca sa vezi carti\n");
                continue;
            } 

            message = compute_get_request(
                SERVER_HOST,
                "/api/v1/tema/library/books",
                NULL, NULL, 0, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("RESPONSE: %s\n", response);

            char *ptr = strstr(response, "Bad Request");
            if (ptr == NULL) {
                printf("ACCES LIBRBARY!\n");
            } 

        /* Request information about a book */
        } else if (!strcmp(command, "get_book\n")) {
            if (flag_login == 0) {
                printf("Error: Nu este logat ca sa vezi carti\n");
                continue;
            } 

            char *id;
            id = calloc(MAX_STDIN, sizeof(char));
            printf("id=");
            fgets(id, MAX_STDIN, stdin);

            id[strlen(id)-1] = '\0';

            char access_path[50] = "/api/v1/tema/library/books/";
            strcat(access_path, id);

            message = compute_get_request(
                SERVER_HOST,
                access_path,
                NULL, NULL, 0, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("RESPONSE: %s\n", response);

            char *ptr = strstr(response, "Bad Request");
            if (ptr == NULL) {
                printf("ACCES LIBRBARY!\n");
            } 

        /* Add a book */
        } else if (!strcmp(command, "add_book\n")) {
            char *title;
            char *author;
            char *genre;
            char *publisher;
            char *page_count;

            /* Initialize the buffers */
            title = calloc(MAX_STDIN, sizeof(char));
            author = calloc(MAX_STDIN, sizeof(char));
            genre = calloc(MAX_STDIN, sizeof(char));
            publisher = calloc(MAX_STDIN, sizeof(char));
            page_count = calloc(MAX_STDIN, sizeof(char));

            /* Get the data from the user */
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

            /* Remove the '\n' */
            title[strlen(title)-1] = '\0';
            author[strlen(author)-1] = '\0';
            genre[strlen(genre)-1] = '\0';
            publisher[strlen(publisher)-1] = '\0';
            page_count[strlen(page_count)-1] = '\0';

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
            message = compute_post_request(
                SERVER_HOST,
                "/api/v1/tema/library/books",
                "application/json", 
                &serialized_json, 0, NULL, 0, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("%s\n", response);

            free(title);
            free(author);
            free(genre);
            free(publisher);
            free(page_count);

        /* Delete a book */
        } else if (!strcmp(command, "delete_book\n")) {
            char *id;

            id = calloc(MAX_STDIN, sizeof(char));
            printf("id=");
            fgets(id, MAX_STDIN, stdin);
            id[strlen(id)-1] = '\0';

            printf("Inainte de seg\n");

            char access_path[50] = "/api/v1/tema/library/books/";
            strcat(access_path, id);

            message = compute_delete_request(
                SERVER_HOST,
                access_path,
                NULL, 
                NULL, 0, NULL, 0, token);

            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            printf("%s\n", response);

            char *ptr = strstr(response, "Bad Request");
            if (ptr == NULL) {
                printf("Ai sters cu succes!\n");
            }

            free(id);
        } 

        /* Close the connection with the server  */
        close_connection(sockfd);

        free(command);
    }

    return 0;
}
