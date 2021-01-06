#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <sys/file.h>
#include <time.h>

#define http500 "HTTP/1.1 500 SERVER ERROR"
#define http404 "HTTP/1.1 404 NOT FOUND"
#define http200 "HTTP/1.1 200 OK"
#define htmlMIME "text/html"
#define fileName "page.html"
#define maxResponseSize 8192

int sendResponce(int fd, char *header, char *contentType, void *body, int contentLength)
{

    char response[maxResponseSize];

    int response_length = sprintf(response,
                                  "%s\n"
                                  "Connection: close\n"
                                  "Content-Length: %d\n"
                                  "Content-Type: %s\n"
                                  "\n",
                                  header,
                                  contentLength,
                                  contentType);

    memcpy(response + response_length, body, contentLength);
    return send(fd, response, response_length + contentLength, 0);
}

char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void readFile(int fd, char requestFile[])
{

    char *path = concat("public", requestFile);

    char *mime_type = htmlMIME;
    char *source = NULL;
    FILE *file;
    if (!(file = fopen(path, "r")))
    {
        file = fopen("404.html", "r");
    }

    long buffer;

    if (file != NULL)
    {
        if (fseek(file, 0L, SEEK_END) == 0)
        {
            buffer = ftell(file);
            source = malloc(sizeof(char) * (buffer + 1));
            fseek(file, 0L, SEEK_SET);
            size_t newLen = fread(source, sizeof(char), buffer, file);
        }
        fclose(file);
        sendResponce(fd, http200, mime_type, source, buffer);
    }
    else
    {
        char *res = "500 Not found";
        sendResponce(fd, http500, mime_type, res, sizeof(source));
    }
    free(source);
}

int printError(int server, char *message)
{
    printf("%s: %ld\n", message, WSAGetLastError());
    closesocket(server);
    WSACleanup();
    return -1;
}

int main()
{

    WSADATA wsaData;
    char *address = "127.0.0.1";
    int port = 80; //Default port for HTTP
    SOCKET server, client;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
    {
        printf("Error starting winsock\n", WSAGetLastError());
        return -1;
    }

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        printError(server, "Error opening socket\n");
        return -1;
    }

    struct sockaddr_in server_data;
    server_data.sin_family = AF_INET;
    server_data.sin_addr.s_addr = inet_addr(address);
    server_data.sin_port = htons(port);

    if (bind(server, (SOCKADDR *)&server_data, sizeof(server_data)) == SOCKET_ERROR)
    {
        printError(server, "bind failed\n");
        return -1;
    }

    if (listen(server, 1) == SOCKET_ERROR)
    {
        printError(server, "Error listening\n");
        return -1;
    }

    printf("Server has been started at port %d\nListening : %s\n", port, address);

    while (1)
    {

        client = accept(server, NULL, NULL);

        if (client == INVALID_SOCKET)
        {
            printError(server, "Client refued to accept.");
            break;
        }
        else
        {
            char request[maxResponseSize] = {0};
            printf("Client connected.\n");
            int data = recv(client, request, sizeof(request), 0);
            // printf("%d", client);
            // printf("%s", request);

            char *p;
            char requestType[8];
            char requestpath[1024];
            char requestProtocol[128];
            sscanf(request, "%s %s %s", requestType, requestpath, requestProtocol);
            // printf("%s \n", requestpath);

            if (!strcmp(requestType, "GET"))
            {
                readFile(client, requestpath);
            }

            else if (!strcmp(requestType, "POST"))
            {
                char *data = "Post request";
                sendResponce(client, http200, htmlMIME, data, strlen(data));
            }

            else if (!strcmp(requestType, "PUT"))
            {
                char *data = "Put request";
                sendResponce(client, http200, htmlMIME, data, strlen(data));
            }

            else if (!strcmp(requestType, "Delete"))
            {
                char *data = "Deletet request";
                sendResponce(client, http200, htmlMIME, data, strlen(data));
            }
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
