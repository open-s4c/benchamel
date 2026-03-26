/*
 * the algorithm is quite simple, but has a bug.
 *
 *
 * link to the bug: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=246475
 *
 * "Other ring-buffer implementations such as DPDK [3] take a slightly
 * different approach, ... this would be to let prod_head take values across
 * the whole uint32 range and use the mask only when reading or writing
 * entries. For the ABA to occur, the whole uint32 range would need to be
 * wrapped-around -which looks unrealistic-."
 */

/*******************************************************************************
 * configuration
 ******************************************************************************/
#define NTHREADS 3
#define LOOPS 2
#define BUF_SIZE 2

/* Both bug fixes are necessary for making the ring buffer work */
//#define BUGFIX1        /* if defined, fixes their module bug in buf_ring */
//#define BUGFIX2        /* if defined, fixes our module bug in buf_ring */

/*******************************************************************************
 * includes
 ******************************************************************************/
#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "utils.h"
#include "buf_ring.h"
#include "buf_ring.c"

/*******************************************************************************
 * test case
 ******************************************************************************/
#define MESSAGES NTHREADS*LOOPS
struct buf_ring *br;
int x = 0;
int consumed[MESSAGES];

static void*
run(void *arg)
{
	uint32_t tid = (intptr_t)arg;

	for (int i = 0; i < LOOPS; i++) {
		int *buf = malloc(sizeof(int));
		assert (buf != NULL);
		*buf = i*NTHREADS + tid;

		if (buf_ring_enqueue(br, buf) != 0) {
			cpu_spinwait();
			free(buf);
		}

		while ((buf = buf_ring_dequeue_mc(br)) != NULL) {
			consumed[*buf]++;
			free(buf);
		}
	}
	return NULL;
}

int main()
{
	br = buf_ring_alloc(BUF_SIZE, NULL, 0, NULL);
	pthread_t t[NTHREADS];
	for (intptr_t i = 0; i < NTHREADS; i++)
		pthread_create(t+i, 0, run, (void*)i);
	for (intptr_t i = 0; i < NTHREADS; i++)
		pthread_join(t[i], NULL);

	for (int i = 0; i < MESSAGES; i++)
		assert(consumed[i] == 0 || consumed[i] == 1);

	return 0;
}

