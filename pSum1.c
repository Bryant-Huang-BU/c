#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_CHILDREN 10

void child_process(int read_pipe, int num_values);
void parent_process(int write_pipes[], int num_children);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <num_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    int num_children = atoi(argv[2]);

    if (num_children <= 0 || num_children > MAX_CHILDREN) {
        fprintf(stderr, "Invalid number of children. Must be between 1 and %d\n", MAX_CHILDREN);
        exit(EXIT_FAILURE);
    }

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Count the number of integers in the file
    off_t file_size = lseek(fd, 0, SEEK_END);
    int num_values = file_size / sizeof(int);

    if (num_values < num_children) {
        fprintf(stderr, "Not enough values in the file for the specified number of children.\n");
        exit(EXIT_FAILURE);
    }

    // Calculate the number of values each child will process
    int values_per_child = num_values / num_children;

    // Create pipes for communication between parent and children
    int pipes[MAX_CHILDREN][2];

    for (int i = 0; i < num_children; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error creating pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Fork child processes
    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            close(pipes[i][0]); // Close read end of the pipe
            child_process(pipes[i][1], values_per_child);
            exit(EXIT_SUCCESS);
        }
    }

    // Parent process
    parent_process(pipes, num_children);

    // Close file descriptor
    close(fd);

    return 0;
}

void child_process(int write_pipe, int num_values) {
    int sum = 0;
    int value;

    for (int i = 0; i < num_values; i++) {
        // Read integer values from the file
        if (read(STDIN_FILENO, &value, sizeof(int)) == -1) {
            perror("Error reading from file");
            exit(EXIT_FAILURE);
        }
        sum += value;
    }

    // Write the sum to the pipe
    if (write(write_pipe, &sum, sizeof(int)) == -1) {
        perror("Error writing to pipe");
        exit(EXIT_FAILURE);
    }

    close(write_pipe); // Close write end of the pipe
}

void parent_process(int write_pipes[], int num_children) {
    int total_sum = 0;
    int child_sum;

    for (int i = 0; i < num_children; i++) {
        close(write_pipes[i][1]); // Close write end of the pipe for this child

        // Read the sum from the pipe
        if (read(write_pipes[i][0], &child_sum, sizeof(int)) == -1) {
            perror("Error reading from pipe");
            exit(EXIT_FAILURE);
        }

        total_sum += child_sum;

        close(write_pipes[i][0]); // Close read end of the pipe for this child
    }

    printf("Total sum: %d\n", total_sum);

    // Wait for all child processes to finish
    for (int i = 0; i < num_children; i++) {
        wait(NULL);
    }
}