/*
 * Author:         Peter Nagy
 *
 * File:           daemon.c
 * Project:        Simple linux daemon
 * Description:    Simple linux daemon communicating over network
 * Date:           10.4.2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define UNUSED(x) (void)(x)

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

int create_connection(int port_num)
{
    
}

int main(int argc, char const *argv[])
{
    /* Parse arguments */


    /* Daemonize process */
    daemonize();

    /* Create and bind socket */

    /* Server infinite loop */
    while(1) {

    }

    return EXIT_SUCCESS;
}
