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


#endif // COMMON_H
