/*
 * Preempt-Rt and Posix-based development functions
 * Compile with -lpthread -lrt
 *
 * Author: Mustafa Ozcelikors <mozcelikors@gmail.com>
 */

// Below is necessary with its end, if you are compiling .C files with .Cpp files for C-style linkage
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

// Nice idea for checking for a error in UNIX, giving error with perror if it does not hold
// Example usage:
//    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
//    CHECK((mqd_t)-1 != mq);
#define CHECK(x) \
	do { \
		if (!(x)) { \
			fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
			perror(#x); \
			exit(-1); \
		} \
	} while (0) \

/* Extra heap size intended to cover all dynamic memory this process
 * might use.
 * If the process allocates more than this limit, there is a risk of
 * page faults.
 */
#define HEAP_TOUCH_SZ (1024 * 1024)

int create_thread(pthread_t *thread,
			 void *thread_func(void *),
			 void *arg,
			 int scheduling_policy,
			 int priority,
			 int stack_size,
			 const char * thread_abbreviation);
void setup_memory (void);
void create_timer (timer_t *timer_var, void (*timer_callback)(union sigval));
void start_timer (timer_t *timer_var, int period_sec, int period_msec);
void delete_timer (timer_t *timer_var);
void delay_sec_msec (int sec, int msec);

#ifdef __cplusplus
}
#endif
