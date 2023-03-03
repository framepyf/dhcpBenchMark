#include "include/udp.h"
#include "include/log.h"

static int set_block(int fd, int block) {
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


int  createUdp6Socket(struct in6_addr * pAddr6,int  sport)
{
	int sock = 0,flag =0;
	struct sockaddr_in6 bindAddr;

	if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
		return -1;
	} 

	//设置为非阻塞
	set_block(sock,0);

	flag = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
				(char *)&flag, sizeof(flag)) < 0) {
		DHCP_LOG_ERROR("Can't set SO_REUSEADDR option on dhcp socket");
	}

	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin6_family = AF_INET6;

	if(sport != 0)
		bindAddr.sin6_port = htons(sport);
	else
		bindAddr.sin6_port = htons(DEFAULT_PORT);

	memcpy(&bindAddr.sin6_addr,pAddr6,sizeof(struct in6_addr));

	if ((bind(sock, (struct sockaddr *) &bindAddr, sizeof(bindAddr))) == -1) {
		return -1;
	} 

	return sock;
}


int sendReqToServer(int sockfd,struct in6_addr * pAddr6,int dport,char * buff,int buffLen)
{

	struct sockaddr_in6 s_addr;
	int addr_len;

	s_addr.sin6_family = AF_INET6;
	s_addr.sin6_port = htons(dport);
	memcpy(&s_addr.sin6_addr,pAddr6,sizeof(struct in6_addr));
	addr_len = sizeof(s_addr);

	return sendto(sockfd, buff, buffLen, 0,(struct sockaddr *) &s_addr, addr_len); 
}