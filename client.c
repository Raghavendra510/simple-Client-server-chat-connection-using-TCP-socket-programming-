#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
int sockfd;

void *recv_msg(void *arg)
{
    char message[BUFFER_SIZE];
    while (1)
    {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0)
        {
            message[receive] = '\0';

            pthread_mutex_lock(&print_mutex);
            printf("\n%s\n", message);

            fflush(stdout);
            pthread_mutex_unlock(&print_mutex);
        }
    }
}

void *send_msg(void *arg)
{
    char message[BUFFER_SIZE];
    while (1)
    {
        pthread_mutex_lock(&print_mutex);

        fflush(stdout);
        pthread_mutex_unlock(&print_mutex);

        fgets(message, BUFFER_SIZE, stdin);

        if (message[0] == '-' && message[1] == '1')
            exit(0);

        send(sockfd, message, strlen(message), 0);
    }
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    struct sockaddr_in serv_addr;
    pthread_t send_thread, recv_thread;
    char name[50];
    int portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    // take a note at the present commands

    printf("\nCommands:\n");
    printf("/list -> show active users\n");
    printf("@username msg -> private message\n");
    printf("@all msg -> broadcast\n");
    printf("-1 -> exit from connection\n\n");

    // enter a name for the client to refer
    printf("Enter your name: ");
    fgets(name, 50, stdin);
    name[strcspn(name, "\n")] = 0;

    send(sockfd, name, strlen(name), 0);

    pthread_create(&send_thread, NULL, send_msg, NULL);
    pthread_create(&recv_thread, NULL, recv_msg, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(sockfd);
    return 0;
}