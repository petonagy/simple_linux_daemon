/*
 * Author:         Peter Nagy
 *
 * File:           daemon.c
 * Project:        Simple linux daemon
 * Description:    Simple linux daemon communicating over network using TCP protocol
 * Date:           13.6.2017
 */

#define _GNU_SOURCE

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
#define BUFFER_SIZE 512
#define TOK_DELIM " \t\r\n\a"

/* Global variables */
int listenfd = 0, connfd = 0;

/* Memory info - parsed from /proc/meminfo */
struct sys_mem_info {
    unsigned int mem_total;
    unsigned int mem_free;
    unsigned int mem_available;
    unsigned int mem_buffered;
    unsigned int mem_cached;
};

/* Get memory usage number from a line in kB */
int get_mem_usage_num(char *line)
{
    char *token;
    int position = 0;
    int result = 0;

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {

        /* Number is in second column in /proc/meminfo */
        if (position == 1) {
            result = atoi(token);
        }

        position++;
        token = strtok(NULL, TOK_DELIM);
    }
    return result;
}

char *get_cpu_usage()
{
    return NULL;
}

char *get_memory_usage()
{
    FILE *mem_f;
    char *line = NULL;
    size_t len = 0;
    struct sys_mem_info meminfo;
    int mem_used;
    char *result;

    if ((mem_f = fopen("/proc/meminfo", "r")) == NULL) {
        return NULL;
    }

    /* Parsing only the first five lines of /proc/meminfo */
    for (int i = 0; i < 5; ++i) {
        if (getline(&line, &len, mem_f) == -1) {
            fprintf(stderr, "Error on parsing a /proc/meminfo file");
            fclose(mem_f);
            return NULL;
        }

        switch(i) {
            case 0:
                meminfo.mem_total = get_mem_usage_num(line);
                break;
            case 1:
                meminfo.mem_free = get_mem_usage_num(line);
                break;
            case 2:
                meminfo.mem_available = get_mem_usage_num(line);
                break;
            case 3:
                meminfo.mem_buffered = get_mem_usage_num(line);
                break;
            case 4:
                meminfo.mem_cached = get_mem_usage_num(line);
                break;
        }
    }

    /* Compute memory usage */
    mem_used = meminfo.mem_total - meminfo.mem_free - meminfo.mem_buffered - meminfo.mem_cached;
    asprintf(&result, "%d", mem_used);

    fclose(mem_f);
    return result;
}

char *par_exec_command(char *buffer)
{

    if (strncmp(buffer, "cpu\r", 4) == 0) {
        return get_cpu_usage();
    } else if (strncmp(buffer, "mem\r", 4) == 0) {
        return get_memory_usage();
    }

    return NULL;
}

void *server_run()
{
    int socket_id = connfd;
    char recv_buffer[BUFFER_SIZE];
    memset(recv_buffer, '\0', sizeof(recv_buffer));

    /* Read message from client */
    if (read(socket_id, recv_buffer, sizeof(recv_buffer)) < 0) {
        fprintf(stderr, "Error on read\n");
        return NULL;
    }

    /* Parse and execute received command */
    char *send_buffer = par_exec_command(recv_buffer);
    if (send_buffer == NULL) {
        asprintf(&send_buffer, "Invalid command!\n");
    }

    /* Send a reply to a client */
    if ( write(socket_id, send_buffer, strlen(send_buffer) * sizeof(char) ) < 0 ) {
        fprintf(stderr, "Error on write\n");
        free(send_buffer);
        return NULL;
    }
    free(send_buffer);
    
    // /* close connection, clean up socket */
    if (close(socket_id) < 0) {
        fprintf(stderr, "Error on close");
        return NULL;
    }

    return NULL;
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

    return ERR_OK;
}

int main(int argc, char const *argv[])
{
    unsigned int sinlen;
    struct sockaddr_in sin;

    /* Thread variables */
    pthread_t thread_id;
    int status;
    void *result;
    int res;
    pthread_attr_t attr;

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

    /* Creating implicit attribute */
    if ((res = pthread_attr_init(&attr)) != 0) {
        printf("pthread_attr_init() err %d\n", res);
        return 1;
    }

    /* Type of thread in atributes */
    if ((res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
        printf("pthread_attr_setdetachstate() err %d\n", res);
        return 1;
    } 

    /* Server infinite loop */
    while(1) {

        /* accepting new connection request from client,
        socket id for the new connection is returned in connfd */
        if ((connfd = accept(listenfd, (struct sockaddr *) &sin, &sinlen)) < 0) {
          fprintf(stderr, "Error on accept\n");
          return ERR_COMM;
        }

        /* Create a new thread */
        status = pthread_create(&thread_id, &attr, &server_run, NULL);
        if (status != 0) {
            return ERR_MEM;
        }

    }

    return ERR_OK;
}
