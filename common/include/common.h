#ifndef COMMON_H
#define COMMON_H

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

// GPIO =================================================================================

#define BUTTON_0_PIN 0
#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 3 //BCM22
#define BUTTON_3_PIN 21 //BCM5
#define BUTTON_4_PIN 22 //BCM6

// BUTTON, MESSAGE QUEUES =======================================================================

#define BUTTON_DEBOUNCE_PERIOD 170 //millisecond
#define BUTTON_INTERRUPT_QUEUE_NAME  "/gpio_intr"
#define BUTTON_INTERRUPT_MAX_SIZE    1024
#define BUTTON_INTERRUPT_MSG_STOP    "exit"

#define BUTTON_IDLE_MSG -1
#define BUTTON_0_PRESSED_MSG 0
#define BUTTON_1_PRESSED_MSG 1
#define BUTTON_2_PRESSED_MSG 2
#define BUTTON_3_PRESSED_MSG 3
#define BUTTON_4_PRESSED_MSG 4


#endif // COMMON_H
