#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>


// TRACE_NODE_STRUCT is a linked list of
// pointers to function identifiers
// TRACE_TOP is the head of the list is the top of the stack
struct TRACE_NODE_STRUCT {
char* functionid;                // ptr to function identifier (a function name)
struct TRACE_NODE_STRUCT* next;  // ptr to next frama
};
typedef struct TRACE_NODE_STRUCT TRACE_NODE;
static TRACE_NODE* TRACE_TOP = NULL;       // ptr to the top of the stack


/* --------------------------------*/
/* function PUSH_TRACE */
/* The purpose of this stack is to trace the sequence of function calls,
just like the stack in your computer would do. The "global" string denotes the start of the 
function call trace */
void PUSH_TRACE(char* p)          // push p on the stack
{
    TRACE_NODE* tnode;
    static char glob[]="global";

    if (TRACE_TOP==NULL) {

// initialize the stack with "global" identifier
        TRACE_TOP=(TRACE_NODE*) malloc(sizeof(TRACE_NODE));

// no recovery needed if allocation failed, this is only
// used in debugging, not in production
        if (TRACE_TOP==NULL) {
          printf("PUSH_TRACE: memory allocation error\n");
          exit(1);
      }

      TRACE_TOP->functionid = glob;
      TRACE_TOP->next=NULL;
  }

// create the node for p
  tnode = (TRACE_NODE*) malloc(sizeof(TRACE_NODE));

// no recovery needed if allocation failed, this is only
// used in debugging, not in production
  if (tnode==NULL) {
    printf("PUSH_TRACE: memory allocation error\n");
    exit(1);
}

tnode->functionid=p;
tnode->next = TRACE_TOP;  // insert fnode as the first in the list
TRACE_TOP=tnode;          // point TRACE_TOP to the first node

}/*end PUSH_TRACE*/

/* --------------------------------*/
/* function POP_TRACE */
/* Pop a function call from the stack */
void POP_TRACE()    // remove the op of the stack
{
    TRACE_NODE* tnode;
    tnode = TRACE_TOP;
    TRACE_TOP = tnode->next;
    free(tnode);

}/*end POP_TRACE*/



/* ---------------------------------------------- */
/* function PRINT_TRACE prints out the sequence of function calls that are on the stack at this instance */
/* For example, it returns a string that looks like: global:funcA:funcB:funcC. */
/* Printing the function call sequence the other way around is also ok: funcC:funcB:funcA:global */
char* PRINT_TRACE()
{
    int depth = 50; //A max of 50 levels in the stack will be combined in a string for printing out.
    int i, length, j;
    TRACE_NODE* tnode;
    static char buf[100];

    if (TRACE_TOP==NULL) {     // stack not initialized yet, so we are
        strcpy(buf,"global");   // still in the `global' area
        return buf;
    }

/* peek at the depth(50) top entries on the stack, but do not
 go over 100 chars and do not go over the bottom of the
 stack */

    sprintf(buf,"%s",TRACE_TOP->functionid);
    length = strlen(buf);                  // length of the string so far
    for(i=1, tnode=TRACE_TOP->next;
        tnode!=NULL && i < depth;
        i++,tnode=tnode->next) {
    j = strlen(tnode->functionid);             // length of what we want to add
    if (length+j+1 < 100) {              // total length is ok
        sprintf(buf+length,":%s",tnode->functionid);
        length += j+1;
    }else                                // it would be too long
        break;
    }
    return buf;
} /*end PRINT_TRACE*/

// -----------------------------------------
// function REALLOC calls realloc
// TODO REALLOC should also print info about memory usage.
// For instance, example of print out:
// "File tracemem.c, line X, function F reallocated the memory segment at address A to a new size S"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void* REALLOC(void* p,int t,char* file,int line)
{
    p = realloc(p,t);
    // print info about memory usage
    printf("File %s, line %d, function %s reallocated the memory segment at address %p to a new size %d\n", file, line, PRINT_TRACE(), p, t);
    return p;
}

// -------------------------------------------
// function MALLOC calls malloc
// TODO MALLOC should also print info about memory usage.
// For instance, example of print out:
// "File tracemem.c, line X, function F allocated new memory segment at address A to size S"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void* MALLOC(int t,char* file,int line)
{
    void* p;
    p = malloc(t);
    printf("File %s, line %d, function %s allocated new memory segment at address %p to size %d\n", file, line, PRINT_TRACE(), p, t);
    return p;
}

// ----------------------------------------------
// function FREE calls free
// TODO FREE should also print info about memory usage.
// For instance, example of print out:
// "File tracemem.c, line X, function F deallocated the memory segment at address A"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void FREE(void* p,char* file,int line)
{
    free(p);
    printf("File %s, line %d, function %s deallocated the memory segment at address %p\n", file, line, PRINT_TRACE(), p);
}

#define realloc(a,b) REALLOC(a,b,__FILE__,__LINE__)
#define malloc(a) MALLOC(a,__FILE__,__LINE__)
#define free(a) FREE(a,__FILE__,__LINE__)


// -----------------------------------------
// function add_column will add an extra column to a 2d array of ints.
// This function is intended to demonstrate how memory usage tracing of realloc is done
// Returns the number of new columns (updated)
int add_column(int** array,int rows,int columns)
{
    PUSH_TRACE("add_column");
    int i;

    for(i=0; i<rows; i++) {
        array[i]=(int*) realloc(array[i],sizeof(int)*(columns+1));
        array[i][columns]=10*i+columns;
    }
    POP_TRACE();
    return (columns+1);
}// end add_column


// ------------------------------------------
// function make_extend_array
// Example of how the memory trace is done
// This function is intended to demonstrate how memory usage tracing of malloc and free is done
void make_extend_array()
{
 PUSH_TRACE("make_extend_array");
 int i, j;
 int **array;
 int ROW = 4;
 int COL = 3;

    //make array
array = (int**) malloc(sizeof(int*)*4);  // 4 rows
for(i=0; i<ROW; i++) {
array[i]=(int*) malloc(sizeof(int)*3);  // 3 columns
for(j=0; j<COL; j++)
    array[i][j]=10*i+j;
}

    //display array
for(i=0; i<ROW; i++)
    for(j=0; j<COL; j++)
        printf("array[%d][%d]=%d\n",i,j,array[i][j]);

// and a new column
    int NEWCOL = add_column(array,ROW,COL);

// now display the array again
    for(i=0; i<ROW; i++)
        for(j=0; j<NEWCOL; j++)
            printf("array[%d][%d]=%d\n",i,j,array[i][j]);

//now deallocate it
        for(i=0; i<ROW; i++)
           free((void*)array[i]);
       free((void*)array);

       POP_TRACE();
       return;
}//end make_extend_array


// function: create array of type char **, initialize array mem with alloc to initial size of 10 char * pointers
void createArray (char ***array, int row, int col){
    PUSH_TRACE("create array");

    // row will be 10 and col will be 20, col is max char per command and row is number of commands
    char **arr = (char **)malloc(sizeof(char *) * row);
    for(int i = 0; i < row; i++){
        arr[i]=(char*)malloc(sizeof(char)*col);
    }
    //POP_TRACE();
    *array = arr;
    POP_TRACE();
}

void addRow(char *** arr, int rows, int col){
    PUSH_TRACE("adding row, reallocating more memory");
    *arr = (char **)realloc((*arr), sizeof(char *) * rows);
    *arr[rows-1] = (char *)malloc(sizeof(char) * col);
}

// function to print the array
void printArray(char **arr, int row) 
{
  PUSH_TRACE("printArray");                              
  for (int i = 0; i < row; i++)
  {
    printf("array[%d] = %s\n", i, arr[i]);
  }
  POP_TRACE();                                     
}

// ----------------------------------------------
// function main
int main(int argc, char *argv[])
{
    PUSH_TRACE("main");
// make_extend_array();

// make sure correct number of arguments
    if (argc != 2){
        printf("Incorrect number of arguments: need two arguments: mem_tracer and the cmdfile to run\n");
        exit(0);
    }

// open file with commands
    FILE * fp = fopen(argv[1], "r");
// error: if fp fails to open
    if(fp == NULL){
        fclose(fp);
        printf("range: cannot open file\n");
        exit(1);
    }

// redirect stdout (fd 1) to memtrace.out file using dup2
int fd = open("memtrace.out", O_RDWR | O_CREAT | O_TRUNC, 0777); // create memtrace.out file
dup2(fd, fileno(stdout));

// create an initial array with 10 rows
char** array = NULL;
int row = 10, col = 20;
createArray(&array, row, col);

// read from file and put each line onto the array
char * line = NULL;
size_t len = 0;
ssize_t read;

// start reading from file
int ctr = 0; // keep track of number of rows in file
while((read = getline(&line, &len, fp)) != -1){
    ctr++; // increment counter, one line read

    // edge case: last line of the cmd file will have a newline, fix by replacing with char
    if(line[strlen(line) - 1] == '\n'){
        line[strlen(line) - 1] = '\0';
    }

    if(ctr > row){ // if lines read is greater than number of rows, need to reallocate more rows
        // add row, can make it a function, will keep as manual code for now
        array = (char **) realloc(array, sizeof(char *)*ctr); // reallocate for row
        array[ctr - 1] = (char *) malloc(sizeof(char)*(strlen(line)+1)); // allocate for column
    }

    // copy value of string into allocated memory
    strncpy(array[ctr - 1], line, (strlen(line)+1));

}

printArray(array, ctr);

// deallocate line
if (line){
    free(line);
}

// deallocate the array
for(int i = 0; i < ctr; i++){
    free(array[i]);
}
free(array);



// close memtrace.out
close(fd);


// end
POP_TRACE();
return(0);
}// end main