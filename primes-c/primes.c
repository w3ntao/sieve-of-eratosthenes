#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void sieve(int port_in) {
    int pid = -1;
    int fd[2];
    pipe(fd);

    int divisor;
    read(port_in, &divisor, sizeof(divisor));
    printf("prime %d\n", divisor);
    fflush(stdout);

    int number;
    while (read(port_in, &number, sizeof(number)) != 0) {
        if (number % divisor == 0) {
            continue;
        }

        if (pid == -1) {
            // execute only once to create child
            pid = fork();
            if (pid == 0) {
                // child process
                close(fd[1]);
                sieve(fd[0]);
                close(fd[0]);
                exit(0);
            } else {
                // parent process
                close(fd[0]);
            }
        }
        // send number to next sieve
        write(fd[1], &number, sizeof(number));
    }

    close(fd[1]);
    wait(&pid);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("error: primes expects 1 argument\n");
        exit(1);
    }

    int fd[2];
    pipe(fd);

    int pid = fork();
    if (pid == 0) {
        // child process
        close(fd[1]);
        sieve(fd[0]);
        close(fd[0]);
        exit(0);
    } else {
        // parent process
        close(fd[0]);
        for (int number = 2; number < atoi(argv[1]); number++) {
            write(fd[1], &number, sizeof(number));
        }
        close(fd[1]);
        wait(&pid);
    }

    return 0;
}
