#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
	FILE *fp; //declares file pointer
	char str[300]; //uses array of char to hold each row
	int sum = 0; //int sum variable to use to collect sum
	int numCol; //int numCol variable for 2nd argument for number of columns
	
	if(argc==1) // if program is run with no arguments, exit with 0
		exit(0);
   
	if(argc != 3){ // if program is run with incorrect number of arguments, exit with 2
   		fprintf(stderr, "Warning: Incorrect arguments\n"); // prints error message to stderr
		exit(2);
	}
   
	sscanf(argv[2], "%d", &numCol); //put 2nd argument into int numCol

	/* opening file for reading */
	char* filename = argv[1]; //put 1st argument into char* filename
	fp = fopen(filename, "r"); // r is to open just for reading
	if(fp == NULL) { // if filename does not exist, exit with 1
		printf("range: cannot open file\n"); // prints error message to stderr
		exit(1);
	}

	while ( fgets (str, 300, fp)!=NULL ) { // fgets: reads a line from stream and stores into string pointed to by str, this while loop reads one line each loop, maximum number of chars to be read is 300, reads until the line is NULL
	char *data = str; // str (the row of char) is stored in data
	int offset; // second part of format for sscanf, number of chars until next space aka the start of the next int in the row, the max number of chars to be read and put into n as an int
	int n; //first part of format for sscanf, the int to add
		for (int i = 0; i < numCol; i++){ // for loop to add each column of the current row
			sscanf(data, " %d%n", &n, &offset); //reads input from data
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
	printf("sum = %d\n", sum); // Prints the sum		   
	exit(0);
}
