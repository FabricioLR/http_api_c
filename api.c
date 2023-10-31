//gcc api.c -o api -lws2_32
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void showError(){
    DWORD errorCode = GetLastError();
    LPSTR messageBuffer = NULL;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        0,
        (LPTSTR)&messageBuffer,
        0,
        NULL
    );

    printf(messageBuffer);
}

const char* getString(int pos, int len, int c, char string[]){
    char *substring;

    substring = malloc(len * 2);

    while (c < len) {
        substring[c] = string[pos + c - 1];
        c++;
    }

    substring[len] = '\0';

    return substring;
}

const char* getRequestPath(char string[]){
    const char *ptr = string;
    int count = 0, length = 0, length1 = 0;

    while ((ptr = strstr(ptr, " ")) != NULL) {
        ptr += 1;
        count++;
        length1 = strlen(ptr) - 1;
        if (count == 1){
            ptr = strstr(ptr, " ");
            ptr += 1;
            length = length1 - strlen(ptr);
            break;
        }
    }

    return getString(5, length, 0, string);
}

void connection(){
    
}

void main(){
    int server_fd, client_fd, received, client_size, file_fd;
    const char *requestPath;
    char *response;
    char *header;
    char *response_data;
    char buffer[1024];
    char file_data[1000];
    struct sockaddr_in server_addr, client_addr;
    WSADATA wsaData;
    FILE *html;

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        printf("WSAStartup failed\n");
        exit(0);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0){
        showError();
        printf("Server creation error\n");
        exit(0);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4000);

    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("Server bind error\n");
        exit(0);
    }

    if(listen(server_fd, 10) < 0){
        printf("Server listen error\n");
        exit(0);
    }
    
    while(1){
        client_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_size);

        if (client_fd >= 0){
            printf("Client connected\n");
        }

        received = recv(client_fd, buffer, sizeof(buffer), 0);

        if(received > 0){
            if (strcmp(getString(1, 3, 0, buffer), "GET") == 0){
                requestPath = getRequestPath(buffer);
                printf("request path: %s\n", requestPath);
                if (strcmp(requestPath, "/") == 0){
                    snprintf(header, 1000,
                        "HTTP/1.1 200 OK\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-type: text/html; charset=utf-8\r\n"
                        "Connection: Keep-Alive\r\n"
                        "\r\n"
                    );

                    html = fopen("index.html", "r");

                    fread(&file_data, sizeof(char), 1000, html);

                    sprintf(response, "%s %s", header, file_data);

                    if(send(client_fd, response, strlen(response), 0) < 0){
                        printf("Error at send(): %ld\n", WSAGetLastError());
                    }

                    fclose(html);
                } else if (strcmp(getString(1, 6, 0, (char *)requestPath), "/users") == 0){
                    if (strcmp(getString(1, 6, 0, (char *)requestPath), "/users/register") == 0){
                        snprintf(header, 1000,
                            "HTTP/1.1 200 OK\r\n"
                            "Access-Control-Allow-Origin: *\r\n"
                            "Content-type: text/plain; charset=utf-8\r\n"
                            "Connection: Keep-Alive\r\n"
                            "\r\n"
                        );

                        response_data = "{\"users\": [\"user1\", \"user2\", \"user3\", \"user4\",]}";

                        sprintf(response, "%s %s", header, response_data);

                        if(send(client_fd, response, strlen(response), 0) < 0){
                            printf("Error at send(): %ld\n", WSAGetLastError());
                        }
                    } else if(strcmp(getString(1, 6, 0, (char *)requestPath), "/users/authenticate") == 0){

                    }
                } else {
                    snprintf(header, 1000,
                        "HTTP/1.1 404 Not Found\r\n"
                        "Access-Control-Allow-Origin: *\r\n"
                        "Content-type: text/plain; charset=utf-8\r\n"
                        "Connection: Keep-Alive\r\n"
                        "\r\n"
                    );

                    response_data = "404 Not Found";

                    sprintf(response, "%s %s", header, response_data);

                    if(send(client_fd, response, strlen(response), 0) < 0){
                        printf("Error at send(): %ld\n", WSAGetLastError());
                    } 
                }
                closesocket(client_fd);
            }
            if (strcmp(getString(1, 4, 0, buffer), "POST") == 0){
                requestPath = getRequestPath(buffer);
            }
        }

    }

    closesocket(server_fd);
    WSACleanup();
    
    return;
}