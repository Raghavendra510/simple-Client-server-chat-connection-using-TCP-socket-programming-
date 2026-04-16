#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

// structure to store both name and file descriptor of clients
typedef struct
{
    int sockfd;
    char name[50];
} client_t;

client_t *clients[MAX_CLIENTS];
// mutex used to prevent race condition
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(client_t *cl)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i])
        {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int sockfd)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i])
        {
            if (clients[i]->sockfd == sockfd)
            {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast(char *msg, char *sender_name, int sender_sock)
{
    char buffer[BUFFER_SIZE];

    sprintf(buffer, "%s: %s", sender_name, msg); // stores sender name and msg into the string buffer in the specified format

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i])
        {
            if (clients[i]->sockfd != sender_sock)
            {
                write(clients[i]->sockfd, buffer, strlen(buffer));
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

bool private_msg(char *msg, char *sender_name, char *receiver)
{
    bool found = 0;
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "[PRIVATE] %s: %s", sender_name, msg); // puts sender name and msg in specified format into buffer
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] && strcmp(clients[i]->name, receiver) == 0)
        {
            write(clients[i]->sockfd, buffer, strlen(buffer));
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    return found;
}
void send_user_list(int sockfd)
{
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "Active users:\n");

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i])
        {
            strcat(buffer, clients[i]->name);
            strcat(buffer, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    write(sockfd, buffer, strlen(buffer));
}

void *handle_client(void *arg)
{
    char buffer[BUFFER_SIZE];
    client_t *cli = (client_t *)arg;

    printf("client %s connected\n", cli->name);

    while (1)
    {
        int receive = recv(cli->sockfd, buffer, BUFFER_SIZE, 0);
        if (receive <= 0)
        {
            printf("%s disconnected\n", cli->name);
            close(cli->sockfd);
            remove_client(cli->sockfd);
            free(cli);
            pthread_exit(NULL);
        }

        buffer[receive] = '\0';

        if (strncmp(buffer, "/list", 5) == 0)
        {
            send_user_list(cli->sockfd);
        }

        else if (buffer[0] == '@')
        {
            char receiver[50], msg[BUFFER_SIZE];

            sscanf(buffer, "@%s %[^\n]", receiver, msg); // puts reciever name for the string reciever and remaing data message in msg string

            if (!private_msg(msg, cli->name, receiver))
            {
                char err[150];
                sprintf(err, "User '%s' not found or offline.\n", receiver);
                send(cli->sockfd, err, strlen(err), 0);
            }
        }
        else
        {
            broadcast(buffer, cli->name, cli->sockfd);
        }
    }
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t tid;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    int portno = atoi(argv[1]);
    serv_addr.sin_port = htons(portno);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, SOMAXCONN);

    printf("Server started on port %d...\n", portno);

    while (1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);

        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->sockfd = connfd;

        int n = recv(connfd, cli->name, 50, 0);
        if (n > 0)
        {
            cli->name[n] = '\0';
        }

        add_client(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);
    }

    return 0;
}