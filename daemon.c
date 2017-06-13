/*
 * Author:         Peter Nagy
 *
 * File:           daemon.c
 * Project:        Simple linux daemon
 * Description:    Simple linux daemon communicating over network
 * Date:           13.6.2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UNUSED(x) (void)(x)

/* Error codes */
#define ERR_OK 0
#define ERR_MEM 1
#define ERR_INTERNAL 2
#define ERR_ARG 3
#define ERR_COMM 4

#define PORT_NUM 5001

char *get_cpu_usage()
{
    return NULL;
}

char *get_memory_usage()
{
    return NULL;
}

void parse_command()
{

}

static void daemonize()
{
    pid_t pid;
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    /* Catch, ignore and handle signals */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork for the second time */
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Set new file permissions */
    // umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    // openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}

int create_connection(const int port_num, struct sockaddr_in *sin, int *listenfd)
{
    // Create socket
    if ( (*listenfd = socket(PF_INET, SOCK_STREAM, 0 ) ) < 0) {
        fprintf(stderr, "Error creating socket\n");
        return ERR_COMM;
    }
    sin->sin_family = PF_INET;              /* set protocol family to Internet */
    sin->sin_port = htons(port_num);               /* set port no. */
    sin->sin_addr.s_addr  = INADDR_ANY;     /* set IP addr to any interface */

    // Bind socket
    if (bind(*listenfd, (struct sockaddr *)sin, sizeof(*sin) ) < 0 ) {
        fprintf(stderr, "Error on bind\n");
        return ERR_COMM;
    }

    // Pasive open - listen
    if (listen(*listenfd, 5)) { 
        fprintf (stderr, "Error on listen\n");
        return ERR_COMM;
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    int listenfd = 0, connfd = 0;
    unsigned int sinlen;
    struct sockaddr_in sin;

    /* Parse arguments - no agruments needed or alowed */
    if (argc > 1) {
        fprintf(stderr, "No arguments allowed! Run without arguments.\n");
        return ERR_ARG;
    }
    UNUSED(argv);

    /* Daemonize process */
    daemonize();

    /* Create and bind socket */
    if (create_connection(PORT_NUM, &sin, &listenfd) != ERR_OK) {
        return ERR_COMM;
    }
    sinlen = sizeof(sin);

    /* Server infinite loop */
    while(1) {

        /* accepting new connection request from client,
        socket id for the new connection is returned in connfd */
        if ((connfd = accept(listenfd, (struct sockaddr *) &sin, &sinlen)) < 0) {
          fprintf(stderr, "Error on accept\n");
          return ERR_COMM;
        }

        /* Create a new thread */

        int socket_id = connfd;
        char recv_buffer[1024];
        memset(recv_buffer, '0', sizeof(recv_buffer));

        /* Read message from client */
        if (read(socket_id, recv_buffer, sizeof(recv_buffer) ) < 0) {
            fprintf(stderr, "error on read\n");
            return ERR_COMM;
        }

        /* Send a reply to a client */
        char *send_buffer = "Test message\n";
        if ( write(socket_id, send_buffer, strlen(send_buffer) * sizeof(char) ) < 0 ) {
            fprintf(stderr, "error on write\n");
            return ERR_COMM;
        }
        
        /* close connection, clean up sockets */
        if (close(socket_id) < 0) {
            fprintf(stderr, "error on close");
            return ERR_COMM;
        } 

        // Free memory
        // free(send_buffer);

    }

    return EXIT_SUCCESS;
}
