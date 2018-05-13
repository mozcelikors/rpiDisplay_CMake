/*                                                                  
 * POSIX Real Time Example (PREEMPT-RT)
 * A good skeleton code for real-time development with preempt-rt
 *
 * Compile with -lpthread -lrt -lwiringPi
 *
 * Author: Mustafa Ozcelikors <mozcelikors@gmail.com>
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>

#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "common.h"

// GPIO library
#include <wiringPi.h>

// Our helper library
#include "rtlib.h"

// DEFS AND GLOBAL VARIABLES ================================================================


bool verbose_f;
volatile sig_atomic_t running_f;

sem_t button0_semaphore;
sem_t button1_semaphore;
sem_t button2_semaphore;
sem_t button3_semaphore;
sem_t button4_semaphore;

timer_t timer1;

pthread_t buttonInterrupt_thread;

// FORWARD DECLARATIONS ==================================================================
void timerCallback (union sigval timer_data);
void forceKillCallback (int signal);
void gpioInterruptCallback (void);

void register_system_signals (void);
void print_usage (char *argv0);
int setup_gpio (void);

static void *buttonInterruptThread (void *args); //static -> Only visible (global) to this file

// CALLBACK FUNCTIONS ====================================================================

void terminationCallback (int signal)
{
	// Be sure to destroy the semaphore to be able to gracefully kill app. Also take care of semaphore if destroying does cause problems!
	sem_post(&button0_semaphore);
	sem_close(&button0_semaphore);
	sem_post(&button1_semaphore);
	sem_close(&button1_semaphore);
	sem_post(&button2_semaphore);
	sem_close(&button2_semaphore);
	sem_post(&button3_semaphore);
	sem_close(&button3_semaphore);
	sem_post(&button4_semaphore);
	sem_close(&button4_semaphore);

	running_f = false;
	pthread_kill (buttonInterrupt_thread, SIGTERM); // SIGTERM -> request termination (15), SIGKILL -> force termination (9)
	// If possible, use pthread_kill to kill threads!
}

void button0InterruptCallback (void)
{
	// Trigger a thread as an example. No huge code in ISR!
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{
		if (digitalRead(BUTTON_0_PIN) == LOW)
			sem_post(&button0_semaphore);
	}
	last_interrupt_time = interrupt_time;
}

void button1InterruptCallback (void)
{
	// Trigger a thread as an example. No huge code in ISR!
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{
		if (digitalRead(BUTTON_1_PIN) == LOW)
			sem_post(&button1_semaphore);
	}
	last_interrupt_time = interrupt_time;
}

void button2InterruptCallback (void)
{
	// Trigger a thread as an example. No huge code in ISR!
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{
		if (digitalRead(BUTTON_2_PIN) == LOW)
			sem_post(&button2_semaphore);
	}
	last_interrupt_time = interrupt_time;
}

void button3InterruptCallback (void)
{
	// Trigger a thread as an example. No huge code in ISR!
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{
		if (digitalRead(BUTTON_3_PIN) == LOW)
			sem_post(&button3_semaphore);
	}
	last_interrupt_time = interrupt_time;
}

void button4InterruptCallback (void)
{
	// Trigger a thread as an example. No huge code in ISR!
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{
		if (digitalRead(BUTTON_4_PIN) == LOW)
			sem_post(&button4_semaphore);
	}
	last_interrupt_time = interrupt_time;
}

// HELPER FUNCTIONS ===============================================================

int setup_gpio (void)
{
	if (wiringPiSetup () < 0)
	{
		fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiISR (BUTTON_0_PIN, INT_EDGE_FALLING, &button0InterruptCallback) < 0) // wiringPi ISR has priority 55!
	{
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiISR (BUTTON_1_PIN, INT_EDGE_FALLING, &button1InterruptCallback) < 0) // wiringPi ISR has priority 55!
	{
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiISR (BUTTON_2_PIN, INT_EDGE_FALLING, &button2InterruptCallback) < 0) // wiringPi ISR has priority 55!
	{
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiISR (BUTTON_3_PIN, INT_EDGE_FALLING, &button3InterruptCallback) < 0) // wiringPi ISR has priority 55!
	{
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiISR (BUTTON_4_PIN, INT_EDGE_FALLING, &button4InterruptCallback) < 0) // wiringPi ISR has priority 55!
	{
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
		return 1 ;
	}
	return 0;
}

void register_system_signals (void)
{
	signal (SIGINT, terminationCallback);
}

void print_usage(char *argv0)
{
	printf("Usage:\n");
	printf("%s OPTIONS\n", argv0);
	printf("\n");
	printf("Options:\n");
	printf("  -r, --rx           Start RX thread\n");
	printf("  -t, --tx           Start TX thread\n");
	printf("  -v, --verbose      Enable debug prints.\n");
	printf("  -h, --help         Print this help and exit.\n");
	printf("\n");
	printf("Example:\n");
	printf("%s -tr\n", argv0);
	printf("\n");
	printf("Exit:\n");
	printf("Ctrl+C to safely exit\n");
	printf("\n");
}


// THREADS ================================================================================
static void *buttonInterruptThread(void *args)
{
	int button0_semval = -1, button1_semval = -1, button2_semval = -1, button3_semval = -1, button4_semval = -1;

	mqd_t mq;
	struct mq_attr attr;
	char buffer[BUTTON_INTERRUPT_MAX_SIZE + 1];

	/* initialize the queue attributes */
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = BUTTON_INTERRUPT_MAX_SIZE;
	attr.mq_curmsgs = 0;

	/* create the message queue */
	mq = mq_open(BUTTON_INTERRUPT_QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
	CHECK((mqd_t)-1 != mq);

	while (running_f)
	{
		CHECK ( sem_getvalue(&button0_semaphore, &button0_semval) == 0);

		if (button0_semval > 0)
		{
			sem_trywait(&button0_semaphore); // Only decrease without waiting, since we know its value is > 0
			snprintf(buffer, sizeof(buffer), "%d", BUTTON_0_PRESSED_MSG);

			printf("CLIENT: Sent \"%s\"\n", buffer);
			mq_send(mq, buffer, BUTTON_INTERRUPT_MAX_SIZE, 0);

			fflush(stdout);
		}

		CHECK ( sem_getvalue(&button1_semaphore, &button1_semval) == 0);

		if (button1_semval > 0)
		{
			sem_trywait(&button1_semaphore); // Only decrease without waiting, since we know its value is > 0
			snprintf(buffer, sizeof(buffer), "%d", BUTTON_1_PRESSED_MSG);

			printf("CLIENT: Sent \"%s\"\n", buffer);
			mq_send(mq, buffer, BUTTON_INTERRUPT_MAX_SIZE, 0);

			fflush(stdout);
		}

		CHECK ( sem_getvalue(&button2_semaphore, &button2_semval) == 0);

		if (button2_semval > 0)
		{
			sem_trywait(&button2_semaphore); // Only decrease without waiting, since we know its value is > 0
			snprintf(buffer, sizeof(buffer), "%d", BUTTON_2_PRESSED_MSG);

			printf("CLIENT: Sent \"%s\"\n", buffer);
			mq_send(mq, buffer, BUTTON_INTERRUPT_MAX_SIZE, 0);

			fflush(stdout);
		}

		CHECK ( sem_getvalue(&button3_semaphore, &button3_semval) == 0);

		if (button3_semval > 0)
		{
			sem_trywait(&button3_semaphore); // Only decrease without waiting, since we know its value is > 0
			snprintf(buffer, sizeof(buffer), "%d", BUTTON_3_PRESSED_MSG);

			printf("CLIENT: Sent \"%s\"\n", buffer);
			mq_send(mq, buffer, BUTTON_INTERRUPT_MAX_SIZE, 0);

			fflush(stdout);
		}

		CHECK ( sem_getvalue(&button4_semaphore, &button4_semval) == 0);

		if (button4_semval > 0)
		{
			sem_trywait(&button4_semaphore); // Only decrease without waiting, since we know its value is > 0
			snprintf(buffer, sizeof(buffer), "%d", BUTTON_4_PRESSED_MSG);

			printf("CLIENT: Sent \"%s\"\n", buffer);
			mq_send(mq, buffer, BUTTON_INTERRUPT_MAX_SIZE, 0);

			fflush(stdout);
		}

		delay_sec_msec (0, 100);
	}
	mq_close(mq);
	return NULL;
}

static void *rxThread(void *args)
{
	while (running_f)
	{

		delay_sec_msec (1, 0);
	}

	return NULL;
}

// MAIN =====================================================================================
int main(int argc, char **argv)
{
	int ret, opt, optind = 0;

	verbose_f = false;
	running_f = 1;

	// For command line argument parsing
	struct option long_opts[] = {
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"tx", required_argument, 0, 't'},
		{"rx", required_argument, 0, 'r'},
		{NULL, 0, 0, 0},
	};

	// Register system signals
	register_system_signals();

	// Lock memory to reduce swapping latency
	setup_memory();

	// Setup GPIO
	CHECK ( setup_gpio() != 1 );

	// Initialize semaphores
	CHECK ( sem_init(&button0_semaphore, 0, 0) == 0 ); //Empty semaphore
	CHECK ( sem_init(&button1_semaphore, 0, 0) == 0 ); //Empty semaphore
	CHECK ( sem_init(&button2_semaphore, 0, 0) == 0 ); //Empty semaphore
	CHECK ( sem_init(&button3_semaphore, 0, 0) == 0 ); //Empty semaphore
	CHECK ( sem_init(&button4_semaphore, 0, 0) == 0 ); //Empty semaphore

	// Parse command line arguments
	while ((opt = getopt_long(argc, argv, "trvh", long_opts, &optind)) != -1)
	{
		switch (opt)
		{
			case 't':
				break;
			case 'r':
				break;
			case 'v':
				verbose_f = true;
				break;
			case 'h':
			default:
				print_usage(argv[0]);
				return 0;
			}
	}

	// Create threads
	ret = create_thread(&buttonInterrupt_thread, buttonInterruptThread, NULL, SCHED_RR, 80, PTHREAD_STACK_MIN, "butIntr");
	if (ret)
		goto out;

	// Join threads!
	ret = pthread_join(buttonInterrupt_thread, NULL);
	if (ret)
		printf("join TX thread failed: %d\n", ret);

out:
	return ret;
}
