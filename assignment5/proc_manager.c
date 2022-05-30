// Done by Corinna Chang & Vanessa Tang
// CS 149-3
// Assignment #5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>


// modified given dictionary_hashtable.c
struct nlist { // table entry:
    struct nlist *next; // next entry in chain
    struct timespec start, finish; // start and finish time of process
    int index; // this is the line index in the input text file
    int pid;  // the process id. you can use the pid result of wait to lookup in the hashtable
    char *command; // command
};


#define HASHSIZE 101
static struct nlist *hashtab[HASHSIZE]; /* pointer table */


/* This is the hash function: form hash value for process id pid */
unsigned hash(int pid)
{
    return pid % HASHSIZE;
}


/* lookup: look for pid in hashtab */
/* lookup by pid */
/* This is traversing the linked list under a slot of the hash table. The array position to look in is returned by the hash function */
struct nlist *lookup(int pid) //char *s)
{
    struct nlist *np;
    for (np = hashtab[hash(pid)]; np != NULL; np = np->next)
        if (np->pid == pid)
          return np; /* found */
    return NULL; /* not found */
}


/* insert in hash table the info for a new pid and its command */
/* This insert returns a nlist node. */
struct nlist *insert(char *command, int pid, int index) //char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;
    // lookup by pid. There are 2 cases:
    if ((np = lookup(pid)) == NULL) { /* case 1: the pid is not found, so you have to create it with malloc. Then you want to set the pid, command and index */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->command = strdup(command)) == NULL)
          return NULL;
        np->pid = pid;
        np->index = index;

        hashval = hash(pid);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;

    } else { /* case 2: the pid is already there in the hashslot, i.e. lookup found the pid. In this case you can either do nothing, or you may want to set again the command and index (depends on your implementation). */
        free((void *) np->command); /*free previous command */
        if ((np->command = strdup(command)) == NULL) {
            return NULL;
        }
        np->index = index;
    }
    return np;
}


int main (int argc, char* argv[]) {

    FILE * fp; //declares file pointer
    char * line = NULL; // the row of char
    size_t len = 0;
    ssize_t read;
    int numOfCommands = 0; // number of commands to carry out

    char newString[20][20]; // 2D array for storing words
    int i,j,ctr; // counters for storing words in each line
    
    struct timespec start, finish; // start and finish time of process

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

        // start time of process
        clock_gettime(CLOCK_MONOTONIC, &start);
        
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

        else if (pid > 0) { // parent process
        
            // store process data in hash table
            struct nlist* curNode = insert(line, pid, numOfCommands);
            curNode->start = start;
        }
    }
    
    // close file
    fclose(fp);
    if (line)
        free(line);
    
    // parent
    int status; // status of the child process
    pid_t p; // pid of child process
    struct nlist* curNode; // node in linked list in hash table
    double duration; // duration of process (finish - start)

    // wait for child processes to finish
    while ((p = wait(&status)) > 0) {

        clock_gettime(CLOCK_MONOTONIC, &finish); // finish time of process
        curNode = lookup(p); // look up pid in hash table

        // if node is found
        if(curNode != NULL) {
            curNode->finish = finish; // update node.finish
            duration = ((double)curNode->finish.tv_sec + 1.0e-9 * curNode->finish.tv_nsec)
                        - ((double)curNode->start.tv_sec + 1.0e-9 * curNode->start.tv_nsec); // calculate duration of process
        } else { // exit with 1 if node is not found
            printf("curNode is NULL \n");
            exit(1);
        }

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
        sprintf(message, "Finished child %d pid of parent %d\n", p, getpid()); // exit message
        write(fd1, message, strlen(message)); // write exit message to .out file

        sprintf(message, "Finished at %ld, runtime duration %.5f\n", curNode->finish.tv_sec, duration); // time message
        write(fd1, message, strlen(message)); // write time message to .out file

        // normal exit
        if(WIFEXITED(status)) {
            sprintf(message, "Exited with exitcode = %d\n", WEXITSTATUS(status)); // exitcode
            write(fd2, message, strlen(message)); // write exitcode to PID.err
            
            // if duration is less than 2 seconds, add message
            if(duration < 2) {
                sprintf(message, "spawning too fast\n");
                write(fd2, message, strlen(message)); // write message to PID.err
            }
        }

        // killed by signal from user
        else if(WIFSIGNALED(status)) {
            sprintf(message, "Killed with signal %d\n", WTERMSIG(status)); // signal
            write(fd2, message, strlen(message)); // write signal numaber to PID.err
        }

        /* We couldn't figure out how to restart the process if the command took more than two seconds
           below is our attempt */
        
        /*
        // restart process if duration is more than 2 seconds
        if(duration > 2) {
            
            // start time of process
            clock_gettime(CLOCK_MONOTONIC, &start);
            
            // fork
            pid_t pid;
            if ((pid = fork()) < 0) {
                fprintf(stderr, "fork error\n"); // prints error message to stderr
                exit(1);
            }
            
            // child process
            if (pid == 0) {

                char *buf2[ctr+1]; // new buf as pointer to each word

                // points to beginning of each word
                for(i=0;i < ctr;i++)
                    buf2[i] = newString[i];
                
                buf2[ctr] = NULL; // NULL indicates the end of line

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

                sprintf(message, "RESTARTING\n");
                write(fd1, message, strlen(message)); // write starting message to .out file
                write(fd2, message, strlen(message)); // write starting message to .err file
                
                sprintf(message, "Starting command %d: child %d pid of parent %d\n", numOfCommands, getpid(), getppid()); // starting message
                write(fd1, message, strlen(message)); // write starting message to .out file

                // use dup2() to make file handle 1 (stdout) go to a file PID.out and 
                // file handle 2 (stderr) go to a file PID.err
                dup2(fd1, 1);
                dup2(fd2, 2);
                
                // execute command
                // if exec fails, exit with exit code of 2
                if (execvp(buf2[0], buf2) < 0) 
                {     
                    fprintf(stderr, "couldn't execute: %s\n", buf2[0]); // prints error message to stderr
                    exit(2);
                }

                exit(0);
            }

            else if (pid > 0) { // parent process
            
                // store process data in hash
                struct nlist* curNode2 = insert(curNode->command, pid, curNode->index);
                curNode2->start = start;
            }
        } */
    }
    exit(0);
}
