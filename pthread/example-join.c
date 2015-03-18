#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define NTHREADS 10

struct thread_args {
	int rep;
	int *counter;
	pthread_mutex_t *mutex;
};

void* thread_function(void *ptr) {
	struct thread_args *arg = (struct thread_args*) ptr;
	int i;
	printf("Thread number %ld\n", pthread_self());
	for (i = 0; i < arg->rep; ++i) {
		pthread_mutex_lock(arg->mutex);
		++*(arg->counter);
		pthread_mutex_unlock(arg->mutex);
	}
}

int main() {
	pthread_t thread[NTHREADS];
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	int counter = 0, i;
	struct thread_args args = {100, &counter, &mutex};
	for (i = 0; i < NTHREADS; ++i)
		pthread_create(&thread[i], NULL, thread_function, &args);

	for (i = 0; i < NTHREADS; ++i)
		pthread_join(thread[i], NULL);

	assert(counter == 1000);

	return 0;
}

