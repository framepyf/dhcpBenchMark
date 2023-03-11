#ifndef __DHCP_COMMON_H__
#define __DHCP_COMMON_H__
#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>


#define DEFAULT_PORT 546


typedef struct socketProto {
	int			(*createSocket)();
    int         (*bindAddr)(int sockfd,struct in6_addr * pSAddr6,int  sport);
	int			(*sendPkt)(int sockfd,struct in6_addr * pDAddr6,int dport,char * buff,int buffLen);

}socketProto_t;


static inline int set_block(int fd, int block) 
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return flags;

	if (block) {        
		flags &= ~O_NONBLOCK;    
	} else {        
		flags |= O_NONBLOCK;    
	}

	if (fcntl(fd, F_SETFL, flags) < 0) return -1;

	return 0;
}

#endif