/*
 *  prodcons module
 *  Producer Consumer module
 * 
 *  Implements routines for the producer consumer module based on 
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"


// Define Locks and Condition variables here

// Producer consumer data structures

// Bounded buffer bigmatrix defined in prodcons.h
//Matrix ** bigmatrix;

// Bounded buffer put() get()

// Initialize counters for put() and get()
int count = 0;
int fill_ptr = 0;
int use_ptr = 0;
int multiplied = 1;

void put(Matrix* value)
{
	bigmatrix[fill_ptr] = value;
	fill_ptr = (fill_ptr + 1) % BOUNDED_BUFFER_SIZE;
	count++;

}

Matrix * get() 
{
  	Matrix* tmp = bigmatrix[use_ptr];
	use_ptr = (use_ptr + 1) % BOUNDED_BUFFER_SIZE;
	count--;
	return tmp;

}

pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
	int i;
	Matrix* matrix = NULL;
	Prodcons* stats = (Prodcons*) arg;
	for (i = 0; i < NUMBER_OF_MATRICES; i++) {
		matrix = GenMatrixRandom(matrix);
		pthread_mutex_lock(&mutex);
		while (count == BOUNDED_BUFFER_SIZE) 
			pthread_cond_wait(&empty, &mutex);
		put(matrix);

		//Increment counters for matrixtotal and sumtotal for producer threads.
		stats->matrixtotal++;
		stats->sumtotal = stats->sumtotal + SumMatrix(matrix);

		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}


// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
	int i;
	Matrix* m1 = NULL;
	Matrix* m2 = NULL;
	Matrix* m3 = NULL;
	Prodcons* stats = (Prodcons*) arg;
	for (i = 0; i < NUMBER_OF_MATRICES; i++) {
		pthread_mutex_lock(&mutex);
		while (count == 0) 
			pthread_cond_wait(&fill, &mutex);
		
		//Retrieve 2 matrices in the buffer
		//Compute matrix product using both matrices.
		if (m1 == NULL && m2 == NULL) {
			m1 = get();
		// Increment counters when a new m1 is fetched.
			stats->sumtotal = stats->sumtotal + SumMatrix(m1);
			stats->matrixtotal++;
		}
		// Initialize m2 and increment new counters.
		else if (m1 != NULL && m2 == NULL) {
			m2 = get();
			stats->sumtotal = stats->sumtotal + SumMatrix(m2);
			stats->matrixtotal++;
			m3 = MatrixMultiply(m1, m2);
			// Increment total of matrix products if matrix product is not null.
			if (m3 == NULL) {
				FreeMatrix(m2); // Discards m2 if null for next retrieval.
				multiplied = 0;
				m2 = NULL;
			}
			else {
				multiplied = 1;
				stats->multtotal++;
				DisplayMatrix(m1, stdout);
				printf("      X\n");
				DisplayMatrix(m2, stdout);
				printf("     =\n");
				DisplayMatrix(m3, stdout);
				printf("\n");
				FreeMatrix(m3);
				FreeMatrix(m2);
				FreeMatrix(m1);
				m1 = NULL;
				m2 = NULL;
				m3 = NULL;
			}
		}
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

