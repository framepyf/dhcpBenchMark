#include "net/udp.h"
#include "log.h"


int  createUdp6Socket(struct in6_addr * pSAddr6,int  sport)
{
	int sockfd = 0,flag =0;


	if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
		return -1;
	} 

	//设置为非阻塞
	set_block(sockfd,0);

	flag = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
				(char *)&flag, sizeof(flag)) < 0) {
		DHCP_LOG_ERROR("Can't set SO_REUSEADDR option on dhcp socket");
	}

	flag = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
				(char *)&flag, sizeof(flag)) < 0) {
		DHCP_LOG_ERROR("Can't set SO_BROADCAST option on raw socket");
	}

	return sockfd;
}

int   bindUdp6Addr(int sockfd,struct in6_addr * pSAddr6,int  sport)
{

	struct sockaddr_in6 bindAddr;
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin6_family = AF_INET6;

	if(sport != 0)
		bindAddr.sin6_port = htons(sport);
	else
		bindAddr.sin6_port = htons(DEFAULT_PORT);

	memcpy(&bindAddr.sin6_addr,pSAddr6,sizeof(struct in6_addr));

	if ((bind(sockfd, (struct sockaddr *) &bindAddr, sizeof(bindAddr))) == -1) {
		return -1;
	}  

}


int sendUdpReqToServer(int sockfd,struct in6_addr * pDAddr6,int dport,char * buff,int buffLen)
{

	struct sockaddr_in6 d_addr;
	int addr_len;

	d_addr.sin6_family = AF_INET6;
	d_addr.sin6_port = htons(dport);
	memcpy(&d_addr.sin6_addr,pDAddr6,sizeof(struct in6_addr));
	addr_len = sizeof(d_addr);

	return sendto(sockfd, buff, buffLen, 0,(struct sockaddr *) &d_addr, addr_len); 
}


socketProto_t   gUdpSocket = {
     .createSocket  = createUdp6Socket,
	 .bindAddr      = bindUdp6Addr,
	 .sendPkt       = sendUdpReqToServer,
};