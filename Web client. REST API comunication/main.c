#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h> 
#include "requests.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>  
#include "helpers.h"
#include "parson.h"

#define HOST "34.241.4.235"
#define port 8080

int main(int argc, char *argv[])
{
    int sockfd;
    char *cookies = malloc(sizeof(char) * 300);
    char *token = malloc(sizeof(char) * 300);
    strcpy(token, "Authorization: Bearer ");
    int isLogged = 0;
    int access = 0;
    int client_on = 1;
    char read_in[1500];
    
    while(client_on != 0)
    {
        fgets(read_in, 1500, stdin);
        read_in[strcspn(read_in, "\r\n")] = 0;
        
        if(strcmp(read_in, "register") == 0)
        {
            sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul
            char username[50];
            char password[50];
            printf("username: "); scanf("%s", username);
            printf("password: "); scanf("%s", password);
            //printf("%s %s\n", username, password);
            //fac JSON ul
            JSON_Value *user  = json_value_init_object();
            JSON_Object *user_cred = json_value_get_object(user);
            json_object_set_string(user_cred, "username", username);
            json_object_set_string(user_cred, "password", password);
            
            char *data[1]; //aici salvez username si parola
            data[0] = malloc(sizeof(char) * 300);
            strcpy(data[0], json_serialize_to_string(user));

            //fac request la server
            char *request = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", data, 1, NULL, 0, 0);
            free(data[0]);
          
            send_to_server(sockfd, request);//trimit mesajul la server
            free(request);

            char *receive = receive_from_server(sockfd);
            char *errors= strstr(receive, "error");
            if(errors) { printf("User este deja inregistrat!\n"); access = 0; }
            else { printf("Te-ai inregistrat cu succes!\n"); }
            //puts(receive);
            free(receive);
            close(sockfd);
            printf("\n");
        }

        if(strcmp(read_in, "login") == 0)
        {
            sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul
            char username[50];
            char password[50];
            printf("username="); scanf("%s", username);
            printf("password="); scanf("%s", password);
            //printf("%s %s\n", username, password);
            //fac JSON ul
            JSON_Value *user  = json_value_init_object();
            JSON_Object *user_cred = json_value_get_object(user);
            json_object_set_string(user_cred, "username", username);
            json_object_set_string(user_cred, "password", password);

            char *data[1]; //aici salvez username si parola
            data[0] = malloc(sizeof(char) * 250);
            strcpy(data[0], json_serialize_to_string(user));
            //fac request la server
            char *request = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", data, 1, NULL, 0, 0);
            //puts(request);
            free(data[0]);

            send_to_server(sockfd, request);//trimit mesajul la server
            free(request);

            char *receive = receive_from_server(sockfd);
            char *errors= strstr(receive, "error");
            if(errors) { printf("Userul nu exista!\n"); }
            else 
            { 
                //printf("%s\n", receive);
                printf("Te-ai logat cu succes! Bun venit!\n");
                char cookie[300];
                char *line = strtok(receive, "\n");
                while( line != NULL )
                {
                    if( strncmp(line, "Set-Cookie: ", 12) == 0 )
                    {
                        int i = 12;
                        while(line[i - 1] != ';')
                        {
                            cookie[i - 12] = line[i];
                            i++;
                        }
                        break;
                    }
                line = strtok(NULL, "\n");
            }
            
            //printf("%s", cookie);
            strcpy(cookies, cookie);
            isLogged = 1;
            } 
            close(sockfd);
            printf("\n");
        }

        if(strcmp(read_in, "enter_library") == 0)
        {
            if(isLogged == 1)
            {
                sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul
                char *cookie[1]; //pt cookie
                cookie[0] = malloc(sizeof(char) * 300);
                strcpy(cookie[0], cookies);

                char *request_cookie = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookie, 1);
                free(cookie[0]);
                
                send_to_server(sockfd, request_cookie);
                
                char *receive_cookie = receive_from_server(sockfd);
                //puts(receive_cookie);
                char *aux_token = malloc(sizeof(char) * 300);
                strcpy(aux_token,strstr(receive_cookie, "token"));
                
                char *p;
                p = strtok(aux_token, "\"");
                p = strtok(NULL, "\"");
                p = strtok(NULL, "\"");
                strcat(token, p);

                free(request_cookie);
                free(receive_cookie);
                               
                close(sockfd);
                access = 1;
                printf("Acces permis in librarie.\n\n");

            }
            else { printf("Logheaza-te ca sa poti primii acces in librarie!\n\n");continue; }     
        }

        if(strcmp(read_in, "get_books") == 0)
        {
            if(isLogged == 1 && access == 1)
            {
                sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul

                char *books_details_cookie[1];
                books_details_cookie[0] = malloc(sizeof(char) * 300);
                strncpy(books_details_cookie[0], cookies, strlen(cookies));

                char *request_books = compute_get_request(HOST, "/api/v1/tema/library/books", token, books_details_cookie, 1);

                free(books_details_cookie[0]);
                //puts(request_books);
                send_to_server(sockfd, request_books);
                char *receive_books = receive_from_server(sockfd);
                //puts(receive_books);
                char *print_books = strstr(receive_books, "[{\"id\"");
                printf("%s", print_books);
                close(sockfd);
                printf("\n\n");                
            }
            else { printf("Logheaza-te/cere acces ca sa poti obtine o cartile!\n\n"); continue; }
        }

        if(strcmp(read_in, "get_book") == 0)
        {
            if(isLogged == 1 && access == 1)
            {
                sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul

                int id;
                printf("id=");scanf("%d", &id);
                char id_convert[10];
                sprintf(id_convert, "%d", id);
                
                char path[50] = "/api/v1/tema/library/books/";
                strcat(path, id_convert);
                char *request_book = compute_get_request(HOST, path, token, NULL, 0);
                //puts(request_book);
                send_to_server(sockfd, request_book);
                char *receive_book = receive_from_server(sockfd);
                //puts(receive_book);
                char *errors= strstr(receive_book, "error");
                if(errors) { printf("Cartea cu id-ul %s nu exista!", id_convert); }
                else
                {
                    char *print_book = strstr(receive_book, "[{\"title\"");
                    printf("%s", print_book);
                }
                close(sockfd);
                printf("\n\n");
            }
            else { printf("Logheaza-te intai ca sa poti obtine o carte!\n"); continue; }   
        }


        if(strcmp(read_in, "add_book") == 0)
        {
            if(isLogged == 1 && access == 1)
            {
                sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul
                char title[50];
                char author[50];
                char genre[50];
                char publisher[50];
                char page_count[50];

                printf("title="); fgets(title, 50, stdin);
                title[strlen(title) - 1] = 0;
                printf("author="); fgets(author, 50, stdin);
                author[strlen(author) - 1] = 0;
                printf("genre="); fgets(genre, 50, stdin);
                genre[strlen(genre) - 1] = 0;
                printf("publisher="); fgets(publisher, 50, stdin);
                publisher[strlen(publisher) - 1] = 0;
                printf("page_count="); fgets(page_count, 50, stdin);
                page_count[strlen(page_count) - 1] = 0;

                int is_int = 0;
                for(int i = 0; i < strlen(page_count); i++)
                {
                    if(page_count[i] >= '0' && page_count[i] <= '9') is_int = 1;
                    else is_int = 0;
                }

                if(is_int == 0) { printf("Numarul de pagini trebuie sa fie un int!\n"); }
                else
                {
                    JSON_Value *book  = json_value_init_object();
                    JSON_Object *book_details = json_value_get_object(book);
                    json_object_set_string(book_details, "title", title);
                    json_object_set_string(book_details, "author", author);
                    json_object_set_string(book_details, "genre", genre);
                    json_object_set_string(book_details, "publisher", publisher);
                    json_object_set_string(book_details, "page_count", page_count);

                    char *data[1]; //aici salvez datele despre carte
                    data[0] = malloc(sizeof(char) * 300);
                    strcpy(data[0], json_serialize_to_string(book));

                    char *t0ken[1]; 
                    t0ken[0]= malloc(sizeof(char) * 300);
                    strncpy(t0ken[0], token, strlen(token));
                    char *request_add_book = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", data, 1, t0ken, 1, 1);
                    free(data[0]);
                    free(t0ken[0]);

                    //puts(request_add_book);
                    send_to_server(sockfd, request_add_book);//trimit mesajul la server
                    free(request_add_book);

                    char *receive_from_add_book = receive_from_server(sockfd);
                    //puts(receive_from_add_book);                    
                    char *error_delete = strstr(receive_from_add_book, "error");
                    if(error_delete == NULL) { printf("Cartea a fost adaugata cu succes!\n"); }
                    else { printf("Imi pare rau, adaugarea cartii a esuat!\n"); }
                }
                close(sockfd);
                printf("\n");
                continue;
            }
            else { printf("Logheaza-te intai ca sa adaugi o carte!\n"); continue; }  
        }

        if(strcmp(read_in, "delete_book") == 0)
        {
            if(isLogged == 1 && access == 1)
            {
                sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul 

                int id;
                printf("id=");scanf("%d", &id);
                char id_convert[10];
                sprintf(id_convert, "%d", id);
                //printf("%s", id_convert);
                char path[50] = "/api/v1/tema/library/books/";
                strcat(path, id_convert);
                char *request_delete_book = compute_delete_request(HOST, path, token, NULL, 0);

                send_to_server(sockfd, request_delete_book);
                char *receive_deleted_book = receive_from_server(sockfd);
                //puts(receive_deleted_book);
                char *error_delete = strstr(receive_deleted_book, "error");
                if(error_delete == NULL) { printf("Cartea a fost stearsa cu succes!\n"); }
                else { printf("Imi pare rau, cartea pe care vrei sa o stergi nu exista!\n"); }
                close(sockfd);
                printf("\n");
            }
            else { printf("Logheaza-te intai ca sa poti sterge o carte!\n"); continue; }     
        }
        
        if(strcmp(read_in, "logout") == 0)
        {
            if(isLogged == 1)
            {
            sockfd = open_connection(HOST, port, AF_INET, SOCK_STREAM, 0); //deschid conexiunea cu serverul 
            char *cookie[1];
            cookie[0] = malloc(sizeof(char) * 300);
            strncpy(cookie[0], cookies, strlen(cookies));
            char *request_logout = compute_get_request(HOST, "/api/v1/tema/auth/logout", token, cookie, 1);
            send_to_server(sockfd, request_logout);
            isLogged = 0;
            access = 0;
            //resetez cookie si token pt o urmatoare logare
            memset(cookies, 0, 300);
            memset(token, 0, 300);
            strcpy(token, "Authorization: Bearer ");

            printf("Te-ai delogat cu succes!\n");
            close(sockfd);
            printf("\n");
            }
            else { printf("Nu esti logat ca sa poti sa te deloghezi!\n"); continue; }
        }

        if(strcmp(read_in, "exit") == 0)
        { client_on = 0; printf("Ai inchis programul! Multumesc de utilizare!\n"); return 0; }
    } 
}
