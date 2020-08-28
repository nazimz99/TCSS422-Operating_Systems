/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 * 
 *  Producer consumer bounded buffer program to produce random matrices in parallel
 *  and consume them while searching for valid pairs for matrix multiplication.
 *  Matrix multiplication requires the first matrix column count equal the 
 *  second matrix row count.  
 *
 *  A matrix is consumed from the bounded buffer.  Then matrices are consumed
 *  from the bounded buffer, one at a time, until an eligible matrix for multiplication
 *  is found.
 *
 *  Totals are tracked using the ProdConsStats Struct for:
 *  - the total number of matrices multiplied (multtotal from consumer threads)
 *  - the total number of matrices produced (matrixtotal from producer threads)
 *  - the total number of matrices consumed (matrixtotal from consumer threads)
 *  - the sum of all elements of all matrices produced and consumed (sumtotal from producer and consumer threads)
 *
 *  Correct programs will produce and consume the same number of matrices, and
 *  report the same sum for all matrix elements produced and consumed. 
 *
 *  For matrix multiplication only ~25% may be e
 *  and consume matrices.  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2018
 *
 * @authors Nazim Zerrouki, Jaskaran Mangat
 * // EXTRA CREDIT FEATURES: EC1, EC2, EC3 implemented
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

int main (int argc, char * argv[])
{
  // Process command line arguments
  int numw = NUMWORK;

  if (argc==1)
  {
    BOUNDED_BUFFER_SIZE=MAX;
    NUMBER_OF_MATRICES=LOOPS;
    MATRIX_MODE=DEFAULT_MATRIX_MODE;
    printf("USING DEFAULTS: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  else
  {
    if (argc==2)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=MAX;
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==3)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==4)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==5)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=atoi(argv[4]);
    }
    printf("USING: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);

  time_t t;
  // Seed the random number generator with the system time
  srand((unsigned) time(&t));

//-----------------------------------------------------------------------------

  printf("Producing %d matrices in mode %d.\n",NUMBER_OF_MATRICES,MATRIX_MODE);
  printf("Using a shared buffer of size=%d\n", BOUNDED_BUFFER_SIZE);
  printf("With %d producer and consumer thread(s).\n",numw);
  printf("\n");

  //pthread_t pr;
  //pthread_t co;

  int prodsum = 0;
  int conssum = 0;
  int prodtot = 0;
  int constot = 0;
  int consmul = 0;

  // consume ProdConsStats from producer and consumer threads
  // add up total matrix stats in prs, cos, prodtot, constot, consmul 

  // Create numw threads to make producer and consumer threads.
  pthread_t prod_threads[numw];
  pthread_t cons_threads[numw];
  // Create pointers to store data for each thread.
  Prodcons* prodstats = malloc(sizeof(Prodcons));
  Prodcons* consstats = malloc(sizeof(Prodcons));

  // Initialize the pointers data.
        prodstats->sumtotal=0, prodstats->multtotal=0, prodstats->matrixtotal=0;
	consstats->sumtotal=0, consstats->multtotal=0, consstats->matrixtotal=0;
  // Create NUMWORK amt. of producer and consumer threads using prod_worker and cons_worker and passing function arg.
  for (int i = 0; i < numw; i++) {
	pthread_create(&prod_threads[i], NULL, prod_worker, prodstats);
	pthread_create(&cons_threads[i], NULL, cons_worker, consstats);
  }

  // Wait for parent before continuing.
  for (int i = 0; i < numw; i++) {
	pthread_join(prod_threads[i], NULL);
	pthread_join(cons_threads[i], NULL);
  }
  // Set counters for producer&consumer threads.
	prodsum = prodstats->sumtotal;
	prodtot = prodstats->matrixtotal;
	conssum = consstats->sumtotal;
	constot = consstats->matrixtotal;
	consmul = consstats->multtotal;

  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n", prodsum, conssum);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n",prodtot, constot, consmul);

  // Free pointers.
  /*free(bigmatrix);
  free(prod_threads);
  free(cons_threads);
  free(prodstats);
  free(consstats);*/

  return 0;
}
