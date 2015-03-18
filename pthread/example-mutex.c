#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

struct thread_args {
	int rep;
	int *counter;
	pthread_mutex_t *mutex;
};

void* thread_function(void *ptr) {
	struct thread_args *arg = (struct thread_args*) ptr;
	int i;
	for (i = 0; i < arg->rep; ++i) {
		pthread_mutex_lock(arg->mutex);
		++*(arg->counter);
		pthread_mutex_unlock(arg->mutex);
	}
}

int main() {
	pthread_t thread1, thread2;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	int counter = 0, ret1, ret2;
	struct thread_args arg1 = {100, &counter, &mutex}, arg2 = {150, &counter, &mutex};
	ret1 = pthread_create(&thread1, NULL, thread_function, &arg1);
	ret2 = pthread_create(&thread2, NULL, thread_function, &arg2);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	assert(counter == 250);

	printf("Thread 1 returns %d\n", ret1);
	printf("Thread 2 returns %d\n", ret2);

	return 0;
}

