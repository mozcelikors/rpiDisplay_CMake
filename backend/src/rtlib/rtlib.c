/*
 * Preempt-Rt and Posix-based development functions
 * Compile with -lpthread -lrt
 *
 * Author: Mustafa Ozcelikors <mozcelikors@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include "rtlib.h"

/*
Policies: SCHED_FIFO or SCHED_RR
SCHED_FIFO: Tasks of same priority finish to completion
SCHED_RR: Tasks of same priority can be interrupted with a time quanta for fairness (0.1 default time slice - not adjustable in Real-timeLinux)
*/
int create_thread(pthread_t *thread,
			 void *thread_func(void *),
			 void *arg,
			 int scheduling_policy,
			 int priority,
			 int stack_size,
			 const char * thread_abbreviation)
{
	struct sched_param param;
	void *stack_buf;
	pthread_attr_t attr;
	int ret;

	stack_buf = mmap(NULL, stack_size, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (stack_buf == MAP_FAILED) {
		printf("mmap failed\n");
		ret = -1;
		goto out;
	}
	memset(stack_buf, 0, stack_size);

	/* Initialize pthread attributes (default values) */
	ret = pthread_attr_init(&attr);
	if (ret) {
		printf("init pthread attributes failed\n");
		goto out;
	}

	/* Set pthread stack to already pre-faulted stack */
	ret = pthread_attr_setstack(&attr, stack_buf, stack_size);
	if (ret) {
		printf("pthread setstack failed\n");
		goto out;
	}

	/* Set scheduler policy and priority of pthread */
	ret = pthread_attr_setschedpolicy(&attr, scheduling_policy);
	if (ret) {
		printf("pthread setschedpolicy failed\n");
		goto out;
	}
	param.sched_priority = priority;
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret) {
		printf("pthread setschedparam failed\n");
		goto out;
	}
	/* Use scheduling parameters of attr */
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		printf("pthread setinheritsched failed\n");
		goto out;
	}

	/* Create a pthread with specified attributes */
	ret = pthread_create(thread, &attr, thread_func, arg);
	if (ret) {
		printf("create pthread failed\n");
		goto out;
	}
	else
	{
		pthread_setname_np(*thread, thread_abbreviation); //If name is too long, this function silently fails.
	}

out:
	return ret;
}


void setup_memory (void)
{
	int i, page_size;
	char *buf;

	/* Lock all current and future pages */
	if (mlockall(MCL_CURRENT | MCL_FUTURE))
		printf("mlockall failed\n");

	/* Turn off malloc trimming. */
	mallopt(M_TRIM_THRESHOLD, -1);

	/* Turn off malloc mmap usage. */
	mallopt(M_MMAP_MAX, 0);

	page_size = sysconf(_SC_PAGESIZE);
	buf = (char*) malloc(HEAP_TOUCH_SZ);

	/* Touch each memory page in order to make sure we get all page faults
	 * now and not later on during execution
	 */
	for (i = 0; i < HEAP_TOUCH_SZ; i += page_size)
		buf[i] = 0;

	free(buf);
}

//static unsigned int pass_value_by_pointer = 42;
//void timerCallback (union sigval timer_data)
//{
	//printf("Timer expiration handler function; %d\n", *(int *) timer_data.sival_ptr);
//	printf ("Timer expired. Maybe give a semaphore to trigger an event in a task.. \n");
//}
void create_timer (timer_t *timer_var, void (*timer_callback)(union sigval))
{
	struct sigevent timer_signal_event;

	struct itimerspec timer_period;

	printf("Create timer\n");
	timer_signal_event.sigev_notify = SIGEV_THREAD;
	timer_signal_event.sigev_notify_function = timer_callback;       // This function will be called when timer expires
	// Note that the following is a union. Assign one or the other (preferably by pointer)
	//timer_signal_event.sigev_value.sival_int = 38;                        // This argument will be passed to the function
	//timer_signal_event.sigev_value.sival_ptr = (void *) &pass_value_by_pointer;     // as will this (both in a structure)
	timer_signal_event.sigev_notify_attributes = NULL;
	timer_create(CLOCK_MONOTONIC, &timer_signal_event, timer_var); // If you want to compute the elapsed time between two events observed on the one machine without an intervening reboot,
																   //CLOCK_MONOTONIC is the best option as opposed to CLOCK_REALTIME.
}

void start_timer (timer_t *timer_var, int period_sec, int period_msec)
{
	struct itimerspec timer_period;
	timer_period.it_value.tv_sec = period_sec;  // period_sec + period_msec timer!
	timer_period.it_value.tv_nsec = period_msec * 1000000;
	timer_period.it_interval.tv_sec = 0;   // non-repeating timer
	timer_period.it_interval.tv_nsec = 0;

	timer_settime(timer_var, 0, &timer_period, NULL);
}

void delete_timer (timer_t *timer_var)
{
	timer_delete (timer_var);
}

void delay_sec_msec (int sec, int msec)
{
	struct timespec tim;
	tim.tv_sec = sec;
	tim.tv_nsec = msec * 1000000;
	nanosleep(&tim , NULL);
}
