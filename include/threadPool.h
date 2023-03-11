#ifndef __THREAD__POOL__
#define __THREAD__POOL__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "log.h"


#define LL_ADD(item, list) do { 	\
    if(list != NULL) { \
		list->prev->next = item ; \
		item->prev = list->prev ; \
		item->next = list ; \
		list->prev = item; \
	 } \
	 else {    \
		 list = item->next = item->prev = item ; \
	 } \
} while(0)

#define LL_REMOVE(item, list) do {						\
    if(item == item->next) { \
	    list = NULL; \
	} else {    \
	   item->prev->next = item->next;	\
	   item->next->prev = item->prev;	\
	   if(list == item) list = item->next ; \
	} \
	item->prev = item->next = NULL;							\
} while(0)

struct NJOB;

typedef void (*task_cb)(struct NJOB *job);

typedef struct NWORKER {
	pthread_t thread;
	int terminate;
	struct NWORKQUEUE *workqueue;
	struct NWORKER *prev;
	struct NWORKER *next;
} nWorker;

typedef struct NJOB {
	task_cb job_function;
	void *user_data;
	struct NJOB *prev;
	struct NJOB *next;
} nJob;

typedef struct NWORKQUEUE {
	struct NWORKER *workers;
	struct NJOB *waiting_jobs;
	pthread_mutex_t jobs_mtx;
	pthread_cond_t jobs_cond;
} nWorkQueue;

typedef nWorkQueue nThreadPool;


extern int  threadPoolCreate(nThreadPool *workqueue, int numWorkers);
extern void threadPoolShutdown(nThreadPool *workqueue); 
extern void threadPoolQueue(nThreadPool *workqueue, nJob *job);

#endif //__THREAD__POOL__