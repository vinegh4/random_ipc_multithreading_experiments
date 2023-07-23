#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>

#define NAMED_PIPE "bruh"

typedef struct message_s {
    int pid;
    int counter;
} message_t;

int main(int argc, char **argv)
{
    //create named pipe (FIFO) with read/write perms
    int ret = mkfifo(NAMED_PIPE, 0666);
    if (ret < 0) {
        printf("Error creating fifo. %s\n", strerror(errno));
    }

    //fork here, pid for parent is nomn-zero, pid for child is 0
    pid_t pid = fork();

    //only the parent writes to the pipe in this example
    if(pid != 0)
    {
        //parent opens the file descriptor for the fifo with write-only access
        int fd = open(NAMED_PIPE, O_WRONLY);
        for (int i = 0; i < 5; i++) {
            message_t msg;
            msg.pid = pid;
            msg.counter = i;
            printf("Process %d: Write %d.\n", pid, i);
            ret = write(fd, &msg, sizeof(message_t));
            if (ret < 0) {
                printf("Process %d: Error while writing message. %s\n", pid, strerror(errno));
            }
            sleep(0.1);
        }
        close(fd);
    }
    //child process only reads
    else {
        int fd = open(NAMED_PIPE, O_RDONLY);
        for(int i = 0; i < 5; i ++) {
            message_t msg;
            ret = read(fd, &msg, sizeof(message_t));
            printf("here\n");
            if(ret < 0) {
                printf("Process %d: error reading message. %s\n.", pid, strerror(errno));
            }
            else if(ret == 0) {
                printf("Write process closed the pipe.\n");
            }
            else {
                printf("Process %d: Received value %d from the parent process %d.\n", pid, msg.counter, msg.pid);
            }
            sleep(0.1);
        }
        close(fd);
    }

    unlink(NAMED_PIPE);
    return 0;
}
