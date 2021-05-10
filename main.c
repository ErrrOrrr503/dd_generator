#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define BUFSIZE 512

int fifofd;
char* fifoname;

void sig_handler (int sig);

int main (int argc, char *argv[])
{
    if (argc != 3) {
        printf ("usage: %s < 8-bit (symbols) pattern > <fifo name>\n", argv[0]);
        return 1;
    }
    if (strlen (argv[1]) != 8) {
        printf ("usage: %s < 8-bit (symbols) pattern > <fifo name>; pattern is incorrect\n", argv[0]);
        return 2;
    }
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sig_handler;
    if (sigaction(SIGTERM, &sa, NULL) || sigaction(SIGPIPE, &sa, NULL) || sigaction(SIGINT, &sa, NULL)) {
        perror (NULL);
        return 6;
    }


    char pattern = 0;
    for (int i = 0; i < 8; i++) 
    {
        if (argv[1][i] == '1')
            pattern |= 1 << i;
        if (!(argv[1][i] == '0' || argv[1][i] == '1')) {
            printf ("usage: %s < 8-bit (symbols) pattern > <fifo name>; pattern is incorrect\n", argv[0]);
        return 3;
        }
    }
    char buf[BUFSIZE];
    for (int i = 0; i < BUFSIZE; i++)
        buf[i] = pattern;
    if (mkfifo (argv[2], 0644)) {
        perror (NULL);
        return 4;
    }
    fifoname = argv[2];
    fifofd = open (fifoname, O_WRONLY);
    if (fifofd == -1) {
        perror (NULL);
        return 5;
    }
    while (1) {
        write (fifofd, buf, BUFSIZE);
    }
    return 0;
}

void sig_handler (int sig)
{
    if (sig == SIGPIPE) {
        fifofd = open (fifoname, O_WRONLY);
        if (fifofd == -1) {
            perror (NULL);
            unlink (fifoname);
            exit (5);
        }
        return;
    }
    if (sig == SIGTERM || sig == SIGINT) {
        unlink (fifoname);
        exit (0);
    }
}