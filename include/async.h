#ifndef __DHCP_ASYNC__
#define __DHCP_ASYNC__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <net/if.h>
#include <arpa/inet.h>

#define ASYNC_CLIENT_NUM		1024

typedef void (*async_result_cb)(void * para);
typedef   void * (*packet_parse_cb)(char * buffer, int buffLen);

typedef struct ep_arg {
	int sockfd;
	async_result_cb cb;
}ep_arg_t;


typedef struct async_context {
	int epfd;
    packet_parse_cb cb;
}async_context_t;


async_context_t * async_task_init(int sockfd,async_result_cb asyncCb,packet_parse_cb packetCb) ;

#endif //__DHCP_ASYNC__