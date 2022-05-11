#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

/*****************************************
//CS149 Spring 2022
//Template for assignment 6
//San Jose State University
//originally prepared by Bill Andreopoulos
*****************************************/


//thread mutex lock for access to the log index
//TODO you need to use this mutexlock for mutual exclusion
//when you print log messages from each thread
pthread_mutex_t tlock1 = PTHREAD_MUTEX_INITIALIZER;

//thread mutex lock for critical sections of allocating THREADDATA
//TODO you need to use this mutexlock for mutual exclusion
pthread_mutex_t tlock2 = PTHREAD_MUTEX_INITIALIZER; 

//thread mutex lock for access to the sum number variable
//TODO you need to use this mutexlock for mutual exclusion
pthread_mutex_t tlock3 = PTHREAD_MUTEX_INITIALIZER; 


void* thread_runner(void*);
pthread_t tid1, tid2, tid3;

//struct points to the thread that created the object. 
//This is useful for you to know which is thread1. Later thread1 will also deallocate.
struct THREADDATA_STRUCT
{
  pthread_t creator;
};
typedef struct THREADDATA_STRUCT THREADDATA;

THREADDATA* p=NULL;


//variable for indexing of messages by the logging function.
int logindex=0;
int *logip = &logindex;


//A sum for the sum of numbers.
int *sum;


/*********************************************************
// function main 
*********************************************************/
int main()
{
  //TODO similar interface as A2: give as command-line arguments three filenames of numbers (the numbers in the files are newline-separated).

  printf("create first thread");
  pthread_create(&tid1,NULL,thread_runner,NULL);
  
  printf("create second thread");
  pthread_create(&tid2,NULL,thread_runner,NULL);
  
  printf("create third thread");
  pthread_create(&tid3,NULL,thread_runner,NULL);

  printf("wait for first thread to exit");
  pthread_join(tid1,NULL);
  printf("first thread exited");

  printf("wait for second thread to exit");
  pthread_join(tid2,NULL);
  printf("second thread exited");

  printf("wait for third thread to exit");
  pthread_join(tid3,NULL);
  printf("third thread exited");

  //TODO print out the sum variable with the sum of all the numbers

  exit(0);

}//end main


/**********************************************************************
// function thread_runner runs inside each thread 
**********************************************************************/
void* thread_runner(void* x)
{
  pthread_t me;

  me = pthread_self();
  printf("This is thread %ld (p=%p)",me,p);
  
  pthread_mutex_lock(&tlock2); // critical section starts
  if (p==NULL) {
    p = (THREADDATA*) malloc(sizeof(THREADDATA));
    p->creator=me;
  }
  pthread_mutex_unlock(&tlock2);  // critical section ends

  if (p!=NULL && p->creator==me) {
    printf("This is thread %ld and I created THREADDATA %p",me,p);
  } else {
    printf("This is thread %ld and I can access the THREADDATA %p",me,p);
  }


  /**
   * //TODO implement any thread summation functionality. 
   * Assign one file per thread. Hint: you can either pass each argv filename as a thread_runner argument from main.
   * Or use the logindex to index argv, since every thread will increment the logindex anyway 
   * when it opens a file to print a log message (e.g. logindex could also index argv)....
   * //Make sure to use any mutex locks appropriately
   */



  // TODO use mutex to make this a start of a critical section 
  if (p!=NULL && p->creator==me) {
    printf("This is thread %ld and I delete THREADDATA",me);
  /**
   * TODO Free the THREADATA object.
   * Freeing should be done by the same thread that created it.
   * See how the THREADDATA was created for an example of how this is done.
   */

  } else {
    printf("This is thread %ld and I can access the THREADDATA",me);
  }
  // TODO critical section ends

  pthread_exit(NULL);
  return NULL;

}//end thread_runner

