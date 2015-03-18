#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

struct thread_args {
	int *counter;
	int rep;
	pthread_mutex_t *counter_mutex;
	pthread_mutex_t *condition_mutex;
	pthread_cond_t *condition;
};

void* thread_function1(void *ptr) {
	struct thread_args *arg = (struct thread_args*) ptr;
	for ( ; ; ) {
		pthread_mutex_lock(arg->condition_mutex);
		while (*arg->counter >= 10 && *arg->counter <= 30)
			pthread_cond_wait(arg->condition, arg->condition_mutex);
		pthread_mutex_unlock(arg->condition_mutex);

		pthread_mutex_lock(arg->counter_mutex);
		++*(arg->counter);
		printf("thread_function1: %d\n", *arg->counter);
		pthread_mutex_unlock(arg->counter_mutex);

		if (*arg->counter >= arg->rep)
			return NULL;
	}
}

void* thread_function2(void *ptr) {
	struct thread_args *arg = (struct thread_args*) ptr;
	for ( ; ; ) {
		pthread_mutex_lock(arg->condition_mutex);
		if (*arg->counter < 10 || *arg->counter > 30)
			pthread_cond_signal(arg->condition);
		pthread_mutex_unlock(arg->condition_mutex);

		pthread_mutex_lock(arg->counter_mutex);
		++*(arg->counter);
		printf("thread_function2: %d\n", *arg->counter);
		pthread_mutex_unlock(arg->counter_mutex);

		if (*arg->counter >= arg->rep)
			return NULL;
	}
}

int main() {
	pthread_t thread1, thread2;
	pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER,
	                condition_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

	int counter = 0, rep = 50;
	struct thread_args args = {&counter, rep, &counter_mutex, &condition_mutex, &condition};
	pthread_create(&thread1, NULL, thread_function1, &args);
	pthread_create(&thread2, NULL, thread_function2, &args);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	assert(counter >= rep);

	return 0;
}

