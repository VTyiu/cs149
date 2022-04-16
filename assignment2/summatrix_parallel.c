#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


int main (int argc, char* argv[]) {
	
	int totalSum = 0; // sum of all files
	int numCol; // int numCol variable for 2nd argument for number of columns
	pid_t pid; // store pid_t data type into pid

	if(argc==1) // if program is run with no arguments, exit with 0
		exit(0);
   
	sscanf(argv[argc - 1], "%d", &numCol); //put last argument into int numCol
	
	// pipe for sum value
	int fd[2];
	
	// open pipe
	if(pipe(fd) < 0) {
		fprintf(stderr, "pipe error");
		return 1;
	}
	
	// each opening new file and find sum of file is it's own process
	for (int i = 1; i < argc - 1; i++) {
		// fork
		if ((pid = fork()) < 0) { // make sure fork was successful
			fprintf(stderr, "fork error");
			return 1;
		}

		// child
		if(pid == 0) { // pid will equal to 0 for child process but not for the parent so only the child process goes through this code
			close(fd[0]); // close reading end
			
			FILE *fp; // declares file pointer
			char str[300]; // uses array of char to hold each row
			int sum = 0; // sum of THIS file
			
			/* opening file for reading */
			char* filename = argv[i]; // put ith argument into char* filename
			fp = fopen(filename, "r"); // r is to open just for reading
			if(fp == NULL) { // if filename does not exist, exit with 1
				printf("range: cannot open file\n"); // prints error message to stderr
				exit(1);
			}

			while ( fgets (str, 300, fp)!=NULL ) { // fgets: reads a line from stream and stores into string pointed to by str, this while loop reads one line each loop, maximum number of chars to be read is 300, reads until the line is NULL
				char *data = str; // str (the row of char) is stored in data
				int offset; // second part of format for sscanf, number of chars until next space aka the start of the next int in the row, the max number of chars to be read and put into n as an int
				int n; // first part of format for sscanf, the int to add
				for (int i = 0; i < numCol; i++){ // for loop to add each column of the current row
					sscanf(data, " %d%n", &n, &offset); // reads input from data
					if(n<0){ // if n is negative, print warning message to stderr but continue adding as if negative number is zero
					sum = sum + 0;
					fprintf(stderr, "Warning: Value %d found\n", n); // prints message to stderr
					}
					else
						sum += n; // n is 0 or positive so add to sum
				data += offset; // removes offset number of chars from data to move onto next column in the row
				n = 0;
				offset = 0;
				}
			}
			fclose(fp); // Closes the file
			
			//write sum to pipe
			write(fd[1], &sum, sizeof(sum));
			close(fd[1]);

			exit(0);
		}
	}

	// parent
	close(fd[1]); // close writing end

	while(wait(NULL) != -1) { // aggregate results into total sum
		int fileSum; // sum of 1 file
		read(fd[0], &fileSum, sizeof(fileSum)); // read sum
		totalSum += fileSum; // add file sum to total
	}

	close(fd[0]); // close reading end
	printf("sum = %d\n", totalSum); // Prints the sum		   
	exit(0);
}
