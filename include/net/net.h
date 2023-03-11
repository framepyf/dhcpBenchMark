#ifndef __DHCP_NET_H__
#define __DHCP_NET_H__

#include "raw.h"
#include "udp.h"

extern int set_block(int fd, int block);

int	 createSpecSocket(int type ,struct in6_addr * pSAddr6,int  sport);
int	 sendSpecPktToServer(int type ,int sockfd,struct in6_addr * pDAddr6,int dport,char * buff,int buffLen);

#endif //__DHCP_NET_H__