#ifndef	_LIBRARY_H
#define	_LIBRARY_H

#include "apue.h"
#include <time.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <threads.h>
#include <semaphore.h>

#define MAX_BUF 4096
#define COLS 4
#define N 30


typedef struct lines{
	long admission;
	long start_trial;
	long end_trial;
	long start_medic;
	long end_medic; 
}LINES;

typedef struct product{
	long tmp_stmp, ocup;
	char sala[30];
}PRODUCT;


#endif	/* _LIBRARY_H */
