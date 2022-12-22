// Done by Corinna Chang & Vanessa Tang
// CS 149-3
// Assignment #3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


int main (int argc, char* argv[]) {

    FILE * fp; //declares file pointer
    char * line = NULL; // the row of char
    size_t len = 0;
    ssize_t read;
    int numOfCommands = 0; // number of commands to carry out

    char newString[20][20]; // 2D array for storing words
    int i,j,ctr; // counters for storing words in each line
    
    if(argc != 2) { // if program is run with incorrect number of arguments, exit with 1
		fprintf(stderr, "incorrect number of arguments\n"); // prints error message to stderr
        exit(1);
    }

    // opening file for reading
    char* filename = argv[1]; //put first argument into char* filename
    fp = fopen(filename, "r"); // r is to open just for reading
    if(fp == NULL) { // if filename does not exist, exit with 1
        printf("range: cannot open file\n"); // prints error message to stderr
        exit(1);
    }

    /* read file line by line */
    while ((read = getline(&line, &len, fp)) != -1) {

        line[read - 1] = '\0'; // remove trailing \n
        numOfCommands++; // update number of commands per line read

        // input filler 0 in newString[][]
        for (int i=0; i<20; i++)
		    for (int j=0; j<20; j++)
			    newString[i][j]=0;

	    j=0; //init index to 0
        ctr=0; //number of words in newString[][]
    
        // add each character of the line to newString[][]
        for(i=0; i<=(strlen(line)); i++)
        {
            // if space or NULL found, assign NULL into newString[ctr]
            if(line[i]==' '||line[i]=='\0') {
                newString[ctr][j]='\0';
                ctr++;  //for next word
                j=0;    //for next word, init index to 0
            }
            else {
                newString[ctr][j]=line[i];
                j++; // go to next char in line
            }
        }

        // fork
        pid_t pid;
		if ((pid = fork()) < 0) {
			fprintf(stderr, "fork error\n"); // prints error message to stderr
			exit(1);
		}
        
        // child process
        if (pid == 0) {
            char *buf[ctr+1]; // new buf as pointer to each word

            // points to beginning of each word
            for(i=0;i < ctr;i++)
                buf[i] = newString[i];
            
            buf[ctr] = NULL; // NULL indicates the end of line

            // create .out and .err
            char outFile[25];
            char errFile[25];
            sprintf(outFile, "%d.out", getpid());
            sprintf(errFile, "%d.err", getpid());

            // create file handler 1 (stdout) and 2 (stderr)
            int fd1 = -1;
            int fd2 = -1;
            fd1 = open(outFile, O_RDWR | O_CREAT | O_APPEND, 0777);
            fd2 = open(errFile, O_RDWR | O_CREAT | O_APPEND, 0777);

            // check for file handler error
            if(fd1 == -1 || fd2 == -1) {
                fprintf(stderr, "file handler error\n");
			    exit(1);
            }

            // write starting message to PID.out
            char message[250];
            sprintf(message, "Starting command %d: child %d pid of parent %d\n", numOfCommands, getpid(), getppid()); // starting message
            write(fd1, message, strlen(message)); // write starting message to .out file

            // use dup2() to make file handle 1 (stdout) go to a file PID.out and 
            // file handle 2 (stderr) go to a file PID.err
            dup2(fd1, 1);
            dup2(fd2, 2);
            
            // execute command
            // if exec fails, exit with exit code of 2
            if (execvp(buf[0], buf) < 0) 
            {     
                fprintf(stderr, "couldn't execute: %s\n", buf[0]); // prints error message to stderr
                exit(2);
            }

            exit(0);
        }
    }
    
    // close file
    fclose(fp);
    if (line)
        free(line);
    
    // parent
    int status; // status of the child process
    pid_t p; // pid of child process

    // wait for child processes to finish
    while ((p = wait(&status)) > 0) {

        // create .out and .err
        char outFile[25];
        char errFile[25];
        sprintf(outFile, "%d.out", p);
        sprintf(errFile, "%d.err", p);

        // create file handler 1 (stdout) and 2 (stderr)
        int fd1 = -1;
        int fd2 = -1;
        fd1 = open(outFile, O_RDWR | O_CREAT | O_APPEND, 0777);
        fd2 = open(errFile, O_RDWR | O_CREAT | O_APPEND, 0777);

        // check for file handler error
            if(fd1 == -1 || fd2 == -1) {
                fprintf(stderr, "file handler error\n");
			    exit(1);
            }

        // write finished message to PID.out
        char message[200];
        sprintf(message, "Finished child %d pid of parent %d\n\n", p, getpid()); // exit message
        write(fd1, message, strlen(message)); // write exit message to .out file

        // normal exit
        if(WIFEXITED(status)) {
            sprintf(message, "Exited with exitcode = %d\n\n", WEXITSTATUS(status)); // exitcode
            write(fd2, message, strlen(message)); // write exitcode to PID.err
        }

        // killed by signal from user
        else if(WIFSIGNALED(status)) {
            sprintf(message, "Killed with signal %d\n\n", WTERMSIG(status)); // signal
            write(fd2, message, strlen(message)); // write signal numaber to PID.err
        }
    }

    exit(0);
}
