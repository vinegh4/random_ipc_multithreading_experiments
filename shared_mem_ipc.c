#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define SHARED_OBJ_NAME "/somename"

typedef struct message_s {
    int pid;
    int counter;
} message_t;

bool write_message(int pid, int value) {
    //delcare shared memeory file descriptor object with a name, create the object and give this
    //process read/write access, this process is owner and has read/write permissions
    int shmFd = shm_open(SHARED_OBJ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    //truncates the shared memoery object to the specific length of our message
    ftruncate(shmFd, sizeof(message_t));
    //gets a virtual memory address to the shared memeory file from the linux kernel
    message_t *msg_ptr = (message_t *)mmap(NULL, sizeof(message_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

    printf("Process %d: Increase the counter.\n", pid);
    msg_ptr->pid = pid;
    msg_ptr->counter = value;

    munmap(msg_ptr, sizeof(message_t));

    close(shmFd);

    return true;
}

bool read_message(int curr_pid, int *curr_value) {
    int shmFd = shm_open(SHARED_OBJ_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmFd, sizeof(message_t));
    message_t *msg_ptr = (message_t *)mmap(NULL, sizeof(message_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

    if(msg_ptr->pid == curr_pid) {
        printf("Process %d: No new msg available.\n", curr_pid);
        return false;
    }
    else {
        printf("Process %d: Receive %d, from PID %d.\n", curr_pid, msg_ptr->counter, msg_ptr->pid);
        *curr_value = msg_ptr->counter;
        munmap(msg_ptr, sizeof(message_t));
    }

    close(shmFd);

    return true;
}

int main(int argc, char **argv) {
    printf("Init the initial value.\n");
    write_message(-1, 0);

    //create child process
    //program forks at this point, original parent process gets a non-zero pid
    //child gets a 0 pid
    pid_t pid = fork();

    //parent
    if(pid != 0) {
        for(int i = 0; i < 5; i++) {
            int value;
            if(read_message(pid, &value)) {
                write_message(pid, ++value);
            }
            sleep(0.1);
        }
    }
    //child
    else {
        for(int j = 0; j < 5; j++) {
            int value;
            if(read_message(pid, &value)) {
                write_message(pid, ++value);
            }
            sleep(0.1);
        }
    }

    printf("==================== End of process %d\n", pid);

    return 0;
}



