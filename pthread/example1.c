#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct thread_args {
	int id;
	char *msg;
};

void* thread_function(void *ptr) {
	struct thread_args *arg = (struct thread_args*) ptr;
	printf("Thread %d: %s\n", arg->id, arg->msg);
}

int main() {
	pthread_t thread1, thread2;
	char *msg1 = "Hello world",
			 *msg2 = "This is great";
	int ret1, ret2;

	struct thread_args arg1 = {1, msg1}, arg2 = {2, msg2};
	ret1 = pthread_create(&thread1, NULL, thread_function, &arg1);
	ret2 = pthread_create(&thread2, NULL, thread_function, &arg2);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	printf("Thread 1 returns %d\n", ret1);
	printf("Thread 2 returns %d\n", ret2);

		return 0;
}

