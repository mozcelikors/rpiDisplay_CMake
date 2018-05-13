#include "BackendReceiveThread.h"
#include <QObject>
#include "QDebug"

#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"

#include "main.h"

void BackendReceiveThread::run()
{
	mqd_t mq;
	ssize_t bytes_read;

	char buffer[BUTTON_INTERRUPT_MAX_SIZE];

	/* open the mail queue */
	mq = mq_open(BUTTON_INTERRUPT_QUEUE_NAME, O_RDONLY);
	CHECK((mqd_t)-1 != mq);

	while(1) {

		memset(buffer, 0x00, sizeof(buffer));
		bytes_read = mq_receive(mq, buffer, BUTTON_INTERRUPT_MAX_SIZE, NULL);
		if(bytes_read >= 0)
		{
			printf("SERVER: Received \"%s\"\n", buffer);

			if (buffer[0] == BUTTON_0_PRESSED_MSG+'0')
			{
				gpio_interrupt = BUTTON_0_PRESSED_MSG;
			}
			else if (buffer[0] == BUTTON_1_PRESSED_MSG + '0')
			{
				gpio_interrupt = BUTTON_1_PRESSED_MSG;
			}
			else if (buffer[0] == BUTTON_2_PRESSED_MSG + '0')
			{
				gpio_interrupt = BUTTON_2_PRESSED_MSG;
			}
			else if (buffer[0] == BUTTON_3_PRESSED_MSG + '0')
			{
				gpio_interrupt = BUTTON_3_PRESSED_MSG;
			}
			else if (buffer[0] == BUTTON_4_PRESSED_MSG + '0')
			{
				gpio_interrupt = BUTTON_4_PRESSED_MSG;
			}

		} else {
			printf("SERVER: None \n");
		}

		fflush(stdout);
		usleep(100 * 1e3); //100ms
	}

	/* cleanup */
	mq_close(mq);
	mq_unlink(BUTTON_INTERRUPT_QUEUE_NAME);
	this->sleep(1);
}


