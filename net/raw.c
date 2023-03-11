#include "net/raw.h"
#include "log.h"

struct sockaddr_in6 gBindAddr;

// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t
checksum (uint16_t *addr, int len)
{
	int count = len;
	register uint32_t sum = 0;
	uint16_t answer = 0;
	// Sum up 2-byte values until none or only one byte left.
	while (count > 1) {
		sum += *(addr++);
		count -= 2;
	}
	// Add left-over byte, if any.
	if (count > 0) {
		sum += *(uint8_t *) addr;
	}
	// Fold 32-bit sum into 16 bits; we lose information by doing this,
	// increasing the chances of a collision.
	// sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	// Checksum is one's compliment of sum.
	answer = ~sum;
	return (answer);
}
// Build IPv6 UDP pseudo-header and call checksum function (Section 8.1 of RFC 2460).
uint16_t
udp6_checksum (struct ipv6hdr * iphdr, struct udphdr * udphdr, uint8_t *payload, int payloadlen)
{
	char buf[1024];
	char *ptr;
	int chksumlen = 0;
	int i;
	ptr = &buf[0];  // ptr points to beginning of buffer buf
	// Copy source IP address into buf (128 bits)
	memcpy (ptr,&iphdr->saddr, sizeof ( iphdr->saddr));
	ptr += sizeof (iphdr->saddr);
	chksumlen += sizeof ( iphdr->saddr);
	// Copy destination IP address into buf (128 bits)
	memcpy (ptr, &iphdr->daddr, sizeof ( iphdr->daddr));
	ptr += sizeof (iphdr->daddr);
	chksumlen += sizeof (iphdr->daddr);
	// Copy UDP length into buf (32 bits)
	memcpy (ptr, &udphdr->len, sizeof (udphdr->len));
	ptr += sizeof (udphdr->len);
	chksumlen += sizeof (udphdr->len);
	// Copy zero field to buf (24 bits)
	*ptr = 0; ptr++;
	*ptr = 0; ptr++;
	*ptr = 0; ptr++;
	chksumlen += 3;
	// Copy next header field to buf (8 bits)
	memcpy (ptr, &iphdr->nexthdr, sizeof (iphdr->nexthdr));
	ptr += sizeof (iphdr->nexthdr);
	chksumlen += sizeof (iphdr->nexthdr);
	// Copy UDP source port to buf (16 bits)
	memcpy (ptr, &udphdr->source, sizeof (udphdr->source));
	ptr += sizeof (udphdr->source);
	chksumlen += sizeof (udphdr->source);
	// Copy UDP destination port to buf (16 bits)
	memcpy (ptr, &udphdr->dest, sizeof (udphdr->dest));
	ptr += sizeof (udphdr->dest);
	chksumlen += sizeof (udphdr->dest);
	// Copy UDP length again to buf (16 bits)
	memcpy (ptr, &udphdr->len, sizeof (udphdr->len));
	ptr += sizeof (udphdr->len);
	chksumlen += sizeof (udphdr->len);
	// Copy UDP checksum to buf (16 bits)
	// Zero, since we don't know it yet
	*ptr = 0; ptr++;
	*ptr = 0; ptr++;
	chksumlen += 2;
	// Copy payload to buf
	memcpy (ptr, payload, payloadlen * sizeof (uint8_t));
	ptr += payloadlen;
	chksumlen += payloadlen;
	// Pad to the next 16-bit boundary
	for (i = 0; i < payloadlen % 2; i++, ptr++) {
		*ptr = 0;
		ptr++;
		chksumlen++;
	}
	return checksum ((uint16_t *) buf, chksumlen);
}

int  createRaw6Socket()
{
	int sock = 0;
	int flag =0;
	struct sockaddr_in6 bindAddr;

	if ((sock = socket(AF_INET6, SOCK_RAW, IPPROTO_RAW)) == -1) {
		return -1;
	} 

	//设置为非阻塞
	set_block(sock,0);

	flag = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
				(char *)&flag, sizeof(flag)) < 0) {
		DHCP_LOG_ERROR("Can't set SO_BROADCAST option on raw socket");
	}

	return sock;
}


int   bindRawAddr(int sockfd,struct in6_addr * pSAddr6,int  sport)
{

	
	memset(&gBindAddr, 0, sizeof(gBindAddr));
	gBindAddr.sin6_family = AF_INET6;

	if(sport != 0)
		gBindAddr.sin6_port = htons(sport);
	else
		gBindAddr.sin6_port = htons(DEFAULT_PORT);

	memcpy(&gBindAddr.sin6_addr,pSAddr6,sizeof(struct in6_addr));


	return 0;

}


int sendRawReqToServer(int sockfd,struct in6_addr * pDAddr6,int dport,char * dhcpBuff,int buffLen)
{

	struct sockaddr_in6 d_addr;
	int addr_len;
	char snedBuff[1024] = {0};
	 char *data  = NULL;
	struct ipv6hdr *ipv6Hdr =  (struct ipv6hdr *)snedBuff;
	struct udphdr *udph = NULL;

    // /*ip hdr*/
	ipv6Hdr->priority = 0;
	ipv6Hdr->version  = 6;
	memset(ipv6Hdr->flow_lbl,0,sizeof(ipv6Hdr->flow_lbl));
	ipv6Hdr->payload_len = htons(buffLen + sizeof(struct udphdr));
	ipv6Hdr->nexthdr    = IPPROTO_UDP; 
	ipv6Hdr->hop_limit  = 64;
	ipv6Hdr->saddr    = gBindAddr.sin6_addr;
	ipv6Hdr->daddr    = *pDAddr6;

     /*ip udr*/
	udph = (struct udphdr *)(ipv6Hdr + 1);
	udph->source =  gBindAddr.sin6_port;
	udph->dest   = htons(dport);
	udph->len    =  ipv6Hdr->payload_len;
	udph->check  =  udp6_checksum(ipv6Hdr,udph,dhcpBuff,buffLen);

    data = (char *)(udph + 1);

	memcpy(data,dhcpBuff,buffLen);





	d_addr.sin6_family = AF_INET6;
	d_addr.sin6_port = 0;
	memcpy(&d_addr.sin6_addr,pDAddr6,sizeof(struct in6_addr));
	addr_len = sizeof(d_addr);

	return sendto(sockfd, snedBuff, buffLen + sizeof(struct ipv6hdr) + sizeof(struct udphdr), 0,(struct sockaddr *) &d_addr, addr_len); 
}


socketProto_t   gRawSocket = {
     .createSocket  = createRaw6Socket,
	 .bindAddr      = bindRawAddr,
	 .sendPkt       = sendRawReqToServer,

};