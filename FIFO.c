#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>


#define MAX_BUF_SIZE 1024

int main() {
    int pipe_fd[2]; // Pipe for communication from process 1 to process 2
    int pipe_fd_reverse[2]; // Pipe for communication from process 2 to process 1

    if (pipe(pipe_fd) == -1 || pipe(pipe_fd_reverse) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        // Child process (process 2)
        close(pipe_fd[1]); // Close write-end of the first pipe
        close(pipe_fd_reverse[0]); // Close read-end of the reverse pipe

        FILE* output_file = fopen("output.txt", "w");
        if (output_file == NULL) {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_BUF_SIZE];
        int num_chars = 0, num_words = 0, num_lines = 0;
        while (1) {
            ssize_t bytes_read = read(pipe_fd[0], buffer, MAX_BUF_SIZE);
            if (bytes_read <= 0)
                break;
            
            buffer[bytes_read] = '\0';
            fprintf(output_file, "%s", buffer);

            for (int i = 0; i < bytes_read; i++) {
                if (buffer[i] == '\n') {
                    num_lines++;
                }
                if (isspace(buffer[i]) || buffer[i] == '\n') {
                    num_words++;
                }
                num_chars++;
            }
        }
        
        fprintf(output_file, "\nNumber of characters: %d\nNumber of words: %d\nNumber of lines: %d\n", num_chars, num_words, num_lines);
        fclose(output_file);

        // Write the processed output to the reverse pipe
        output_file = fopen("output.txt", "r");
        while (1) {
            ssize_t bytes_read = fread(buffer, 1, MAX_BUF_SIZE, output_file);
            if (bytes_read <= 0)
                break;
            write(pipe_fd_reverse[1], buffer, bytes_read);
        }
        fclose(output_file);

        close(pipe_fd[0]);
        close(pipe_fd_reverse[1]);
    } else {
        // Parent process (process 1)
        close(pipe_fd[0]); // Close read-end of the first pipe
        close(pipe_fd_reverse[1]); // Close write-end of the reverse pipe

        char input[MAX_BUF_SIZE];

        printf("Enter sentences (Ctrl-D to end input):\n");
        while (fgets(input, MAX_BUF_SIZE, stdin) != NULL) {
            write(pipe_fd[1], input, strlen(input));
        }
        
        close(pipe_fd[1]);

        char buffer[MAX_BUF_SIZE];
        while (1) {
            ssize_t bytes_read = read(pipe_fd_reverse[0], buffer, MAX_BUF_SIZE);
            if (bytes_read <= 0)
                break;
            
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }
        close(pipe_fd_reverse[0]);

        int status;
        waitpid(child_pid, &status, 0);
    }

    return 0;
}

