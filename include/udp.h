#ifndef __DHCP__UDP__
#define __DHCP__UDP__
#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>

#define DEFAULT_PORT 546

int  createUdp6Socket(struct in6_addr * pAddr6,int  sport);
int  sendReqToServer(int sockfd,struct in6_addr * pAddr6,int dport,char * buff,int buffLen);
#endif