#include <errno.h>
#include "net/net.h"
#include "log.h"

static socketProto_t * gNetArray[] =
{
	       &gUdpSocket,
	       &gRawSocket     
};


int	 createSpecSocket(int type ,struct in6_addr * pSAddr6,int  sport)
{  
	   int sockfd = 0 ;

	   if(type > sizeof(gNetArray)/sizeof(socketProto_t * )){
		  return -1;
	   }

	   sockfd = gNetArray[type]->createSocket();

	   if(sockfd < 0){
		  DHCP_LOG_ERROR("createSpecSocket failed,type %d",type);
		  return sockfd;
	   }

       if(gNetArray[type]->bindAddr(sockfd,pSAddr6,sport) < 0){
		   DHCP_LOG_ERROR("bind error: %s",strerror(errno));
		   return -1;
	   }
     
	   return sockfd;
}


int	 sendSpecPktToServer(int type ,int sockfd,struct in6_addr * pDAddr6,int dport,char * buff,int buffLen)
{  
	 
	   if(type > sizeof(gNetArray)/sizeof(socketProto_t * )){
		  return -1;
	   }

	   return gNetArray[type]->sendPkt(sockfd,pDAddr6,dport,buff,buffLen);
}

