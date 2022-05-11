#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include <err.h>

/*
Authors: Corinna Chang and Vanessa Tang
Date: May 11, 2022
*/
int main(int argc, char *argv[]) {
    // If less than 2 arguments are provided by the user, exit with 0.
    if (argc < 3) {
        exit(0);
    }

    // Gets the last command line argument, which is the number of columns.
    char *colsStr = argv[argc - 1];

    // If the last command line argument can't be converted to an integer, exit with 0.
    if (atoi(colsStr) == 0) {
        exit(0);
    }

    // Converts the last command line argument to an integer.
    int cols = atoi(colsStr);

    // Stores the total sum.
    int totalSum = 0;

    pid_t p;
	int *fd;
    // Starts the child processes (one per file).
    for (int i = 1; i < argc - 1; i++) {
	// initialize mmap
	fd = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
	
	if (fd == MAP_FAILED)
		errx(1, "mmap failed");

        // Creates a child process.
        p = fork();

        // Failure to create a child process.
        if (p < 0) {
            fprintf(stderr, "Fork failed");
            exit(1);
        }

        // Parent process.
        else if (p > 0) {
            //int sum;

            // Waits for the child process to send its matrix sum value.
            wait(NULL);

		// Reads the matrix sum from the child process.
		totalSum += *fd;
        }

        // Child process.
        else {
            int sum = 0;

            // Opens the file.
            FILE *fp = fopen(argv[i], "r");

            // If the child process fails to open a file, prints an error message, writes the matrix sum, and exits with 1.
            if (fp == NULL) {
                printf("range: cannot open file\n");
		*fd = sum;
                exit(1);
            }

            else {
                fseek(fp, 0, SEEK_END); // Goes to the end of the file.

                // If the file is empty with no numbers, writes the matrix sum, and exits with 0.
                if (ftell(fp) == 0) {
                    *fd = sum;
                    exit(0);
                }

                fseek(fp, 0, SEEK_SET); // Goes to the beginning of the file.

                char str[5000]; // Stores the lines in the file.

                // Goes through the file line by line.
                while (fgets(str, 5000, fp) != NULL) {
                    // If the line ends in a new line '\n', replace it with '\0' (null).
                    if (str[strlen(str) - 1] == '\n') {
                        str[strlen(str) - 1] = 0;
                    }

                    char *data = str;
                    int offset;
                    int num;

                    // Reads up to N numbers, and then goes to the next line once finished.
                    for (int i = 0; i < cols && sscanf(data, " %d%n", &num, &offset) == 1; i++) {
                        // If a number is less than 0, set it to 0 and print a warning message to stderr.
                        if (num < 0) {
                            fprintf(stderr, "Warning: Value %d found.\n", num);
                            num = 0;
                        }

                        sum += num; // Adds the number to the matrix sum.
                        data += offset; // Goes to the next number in the line.
                    }
                }
            }

            // Closes the file.
            fclose(fp);

            // Writes the matrix sum and deallocates memory.
		*fd = sum;
		munmap(fd, sizeof(int));
            exit(0);
        }
    }

    printf("Sum: %d\n", totalSum); // Prints the total sum from all of the matrices.
    exit(0);
}
