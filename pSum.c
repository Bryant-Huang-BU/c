#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_NUMBERS 1000000
#define BUFFER_SIZE 1024

void parent(char* filename, int fd[], int n);
void child(char* filename, int fd[], int i, int start, int end);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <number of children>\n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    int n = atoi(argv[2]);

    int fd[2 * n];

    for (int i = 0; i < n; i++) {
        if (pipe(fd + 2 * i) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    parent(filename, fd, n);

    return 0;
}

void parent(char* filename, int fd[], int n) {
    int status;
    int sum = 0;

    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            child(filename, fd, i, i * MAX_NUMBERS / n, (i + 1) * MAX_NUMBERS / n);
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        close(fd[2 * i + 1]);
    }

    for (int i = 0; i < n; i++) {
        int temp;
        read(fd[2 * i], &temp, sizeof(int));
        sum += temp;
    }

    for (int i = 0; i < n; i++) {
        close(fd[2 * i]);
    }

    printf("The sum is: %d\n", sum);
}

void child(char* filename, int fd[], int i, int start, int end) {
    close(fd[2 * i]);
    close(fd[2 * i + 1]);

    int fd2 = open(filename, O_RDONLY);
    if (fd2 == -1) {
        perror("open");
        exit(1);
    }

    int sum = 0;

    lseek(fd2, start * sizeof(int), SEEK_SET);

    for (int j = start; j < end; j++) {
        int temp;
        read(fd2, &temp, sizeof(int));
        sum += temp;
    }

    close(fd2);

    write(fd[2 * i + 1], &sum, sizeof(int));
    close(fd[2 * i + 1]);
}