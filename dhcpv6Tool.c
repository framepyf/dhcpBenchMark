#include  <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <net/if.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include "dhcpv6Tool.h"
#include "threadPool.h"


int gSockFd = 0;
nThreadPool gPool;
dhcpv6_tool_para_t gPara;
int  gAckCount = 0;


static struct option long_options[] = {
	{"dip",		required_argument,	0,	'd'},
	{"dport",	required_argument,		0,	'p'},
	{"sip",		required_argument,	0,	'b'},
	{"sport",	required_argument,		0,	'r'},
	{"msg_type",	required_argument, 	0,	'm'},
	{"ipv6_address",	required_argument,		0,	'i'},
	{"client_duid",	required_argument,	0,	'c'},
	{"server_duid",	required_argument,	0,	's'},
	{"option",	required_argument,	0,	'o'},
	{"thread",	required_argument,	0,	'a'},
	{"count",	required_argument,	0,	't'},
	{"speed",	required_argument,	0,	'e'},
	{0,		0,			0,	0}
};


static void usage()
{

	fprintf(stderr, "usage: dhcpv6_test [--dip ipv6addr] [--dport dport] [--sip ipv6addr] [--sport sport]"
			"[--msg_type type] [--ipv6_address requestaddr]  [--client_duid clientid]  [--server_duid serverid] [--option data] [--thread threadNum]  [--count reqCount] [--speed count]\n");
	fprintf(stderr, "for example:\n");
	fprintf(stderr, "  ./dhcpv6Tool --dip 2001::1  --dport 547 --sport 547 --sip 2001::79 --msg_type 3  --ipv6_address 2001::b  --client_duid 00010001234ecc25005056b1703c"
	             "--server_duid 000100012a3f0e760050568de3bc  --option 000600020011\n");
	fprintf(stderr, "  ./dhcpv6Tool --dip 2001::1  --dport 547 --sport 547 --sip 2001::79  --msg_type 1  --server_duid 000100012a3f0e760050568de3bc  --thread 5 --count 10000 --speed 1000\n");
}

int get_hex_by_str( const char *key, int key_len,unsigned char * iaid_duid, int id_len)
{
	int offset = 0,byte = 0;

	for ( ; (byte < id_len) && (offset < key_len) ; ++byte) {
		sscanf(key + offset, "%02X", (unsigned int *)&iaid_duid[byte]);
		offset += 2;
	}

	return byte;
}

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
		printf("Can't set SO_REUSEADDR option on dhcp socket");
	}

	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin6_family = AF_INET6;

	if(sport != 0)
		bindAddr.sin6_port = htons(sport);
	else
		bindAddr.sin6_port = htons(DEFAULT_PORT);

	memcpy(&bindAddr.sin6_addr,pAddr6,IPV6_ADDR_LEN);

	if ((bind(sock, (struct sockaddr *) &bindAddr, sizeof(bindAddr))) == -1) {
		return -1;
	} 

	return sock;
}

int build_relay_msg_header(char * buffer,struct in6_addr * pAddr6,unsigned short ** ppLen)
{
	struct dhcpv6_relay_packet  *relay_packet = (struct dhcpv6_relay_packet  *)buffer;
	unsigned short * option = NULL;

	relay_packet->msg_type = DHCPV6_RELAY_FORW;
	relay_packet->hop_count = 0;
	memcpy(relay_packet->link_address,pAddr6,16);
	memcpy(relay_packet->peer_address,pAddr6,16);
	option = (unsigned short *)relay_packet->options;
	*option = htons(D6O_RELAY_MSG);
	*ppLen  = option + 1;

	return sizeof(struct dhcpv6_relay_packet) + TL_LEN;
}

void getRandomBytes(unsigned char *p, int len) {

	FILE *fp = fopen("/dev/urandom","r");

	if(fp == NULL){
		perror("fopen /dev/urandom error");
		exit(errno);
	}

	if(  fread(p,len,1,fp) < 1){
		perror("fread /dev/urandom error");
		exit(errno);
	}

	fclose(fp);

}



int build_dhcpv6_packet_header(char * buffer,unsigned char msgType)
{
	struct dhcpv6_packet  * packet = ( struct dhcpv6_packet  *)buffer;
	char transId[3] = {0};

	getRandomBytes(transId,3);

	packet->msg_type = msgType;
	memcpy(packet->transaction_id,&transId,3);

	return TL_LEN;
}



int build_dhcpv6_id_option(char * buffer,unsigned short optionCode,char * id,int * idLen)
{
	unsigned short * option = (unsigned short *)buffer;
	*option = htons(optionCode);

	if(*idLen == 0){
		*idLen = DEFAULT_DUID_LEN;
		getRandomBytes(id,*idLen);
	}

	option  = option + 1;
	*option = htons(*idLen);
	option  = option + 1;
	memcpy((unsigned char *)option,id,*idLen);

	return *idLen + TL_LEN;
}

int build_dhcpv6_iana_info(char * buffer,char * iaid,struct in6_addr * pAddr6)
{
	unsigned short * option = (unsigned short *)buffer;
	int  zero = 0;

	*option = htons(D6O_IA_NA);
	option  = option + 1;

	*option = htons(INIA_INFO_LEN);
	option  = option + 1;

	memcpy((unsigned char *)option,iaid,IAID_LEN);
	option += 2 ;
	memcpy((unsigned char *)option,&zero,4);
	option += 2 ;
	memcpy((unsigned char *)option,&zero,4);
	option  = option + 2;

	*option = htons(D6O_IAADDR);
	option  = option + 1;

	*option = htons(IAADDR_INFO_LEN);
	option  = option + 1;


	memcpy((unsigned char *)option,pAddr6,IPV6_ADDR_LEN);
	option  = option + 8;
	memcpy((unsigned char *)option,&zero,4);
	option  = option + 2;
	memcpy((unsigned char *)option,&zero,4);

	return INIA_INFO_LEN + TL_LEN;
}


int build_dhcpv6_option_data(char * buffer,char * optData,int optDataLen)
{
	if(optDataLen != 0)
		memcpy(buffer,optData,optDataLen);

	return optDataLen;
}

int sendReqToServer(int sockfd,struct in6_addr * pAddr6,int dport,char * buff,int buffLen)
{

	struct sockaddr_in6 s_addr;
	int addr_len;

	s_addr.sin6_family = AF_INET6;
	s_addr.sin6_port = htons(dport);
	memcpy(&s_addr.sin6_addr,pAddr6,IPV6_ADDR_LEN);
	addr_len = sizeof(s_addr);

	return sendto(sockfd, buff, buffLen, 0,(struct sockaddr *) &s_addr, addr_len); 
}


int build_dhcpv6_req(char * buff,dhcpv6_tool_para_t * pPara)
{

	int relayHeaderLen = 0;
	int totalLen = 0;
	unsigned short * pLen = NULL;

	relayHeaderLen  = build_relay_msg_header(buff,&pPara->s_addr,&pLen);
	totalLen += relayHeaderLen;

	totalLen += build_dhcpv6_packet_header(buff + totalLen,pPara->msgType);

	totalLen += build_dhcpv6_id_option(buff + totalLen,D6O_CLIENTID,pPara->clinetDuid,&(pPara->clinetIdLen));     

	if(pPara->msgType  != DHCPV6_SOLICIT){
		totalLen +=  build_dhcpv6_id_option(buff + totalLen,D6O_SERVERID,pPara->serverDuid,&(pPara->serverIdLen));    
	}    

	totalLen +=  build_dhcpv6_iana_info(buff + totalLen,pPara->clinetIdLen > IAID_LEN ? pPara->clinetDuid + pPara->clinetIdLen - IAID_LEN : NULL ,&pPara->r_addr);

	totalLen +=  build_dhcpv6_option_data(buff + totalLen,pPara->optData,pPara->optLen);

	// 设置D6O_RELAY_MSG内容长度
	*pLen = htons(totalLen - relayHeaderLen);

	return totalLen;
}


void sendSpecReqCb(nJob *job) 
{
	char buff[1024] = {0};
	int buffLen = 0;
     struct timespec start, end;
    double elapsed_time;
	double  singlePktTime =  0;

	(void)job->user_data;

	if(gPara.speed > 0){
		singlePktTime =  1e9 / gPara.speed;

		/*控制每个线程的发包速率*/
		clock_gettime(CLOCK_REALTIME, &start);
	}

	buffLen = build_dhcpv6_req(buff,&gPara);

	if( sendReqToServer(gSockFd,&gPara.d_addr,gPara.dport,buff,buffLen) < 0 ){
		 perror("send Solicit error:");
	}

	if(gPara.speed > 0){
		clock_gettime(CLOCK_REALTIME, &end);

		elapsed_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		
		if (elapsed_time < singlePktTime) {
				usleep(singlePktTime - elapsed_time);
		}
	}

	if(job->user_data)
		free(job->user_data);
	free(job);
}



int build_request(char * buff,dhcpv6_para_t * dhcpv6Para)
{
	dhcpv6_tool_para_t para;

	memcpy(&para,&gPara,sizeof(gPara));
	para.msgType = DHCPV6_REQUEST;
	para.clinetIdLen = dhcpv6Para->clinetIdLen;
	memcpy(para.clinetDuid,dhcpv6Para->clinetDuid,dhcpv6Para->clinetIdLen);
	memcpy(&para.r_addr,dhcpv6Para->ipAddr,IPV6_ADDR_LEN);
	memcpy(para.transaction_id,dhcpv6Para->transaction_id,sizeof(dhcpv6Para->transaction_id));

	return build_dhcpv6_req(buff,&para);
}

void sendDhcpv6Req(nJob *job) 
{
	char buff[1024] = {0};
	int buffLen = 0;
	dhcpv6_para_t * dhcpv6Para = job->user_data;

	if(dhcpv6Para->msg_type == DHCPV6_ADVERTISE){
		buffLen = build_request(buff,dhcpv6Para);

		if(sendReqToServer(gSockFd,&gPara.d_addr,gPara.dport,buff,buffLen) < 0){
			perror("send request error:");
		}
	}

	if(job->user_data)
		free(job->user_data);
	free(job);
}

int createTask(int reqCount)
{
	int i = 0;
	if(reqCount < 1) reqCount = 1 ;
	for (i = 0;i < reqCount;i ++) {
		nJob *job = (nJob*)malloc(sizeof(nJob));
		if (job == NULL) {
			perror("malloc");
			exit(1);
		}

		job->job_function = sendSpecReqCb;
		job->user_data = NULL;


		threadPoolQueue(&gPool, job);

	}
}


void async_result_proc_cb(dhcpv6_para_t * dhcpv6Para)
{
	nJob *job = (nJob*)malloc(sizeof(nJob));
	if (job == NULL) {
		perror("malloc");
		exit(1);
	}

	job->job_function = sendDhcpv6Req;
	job->user_data = dhcpv6Para;

	threadPoolQueue(&gPool, job);
}

dhcpv6_para_t * parse_option_buffer (unsigned char *optBuff,unsigned len,unsigned char * pMsgType)
{
	int offset = 0;
	unsigned int code = 0 ;
	dhcpv6_para_t *dhcpv6Para = NULL;
	int once = 0;
	char * val = NULL;
	char  flag = 0;

	
	for(offset = 0 ; offset < len ;){
		code = ntohs(*(unsigned short *)(optBuff + offset));

		offset += sizeof(unsigned short);//// skip T

		if (code == DHO_PAD)
			continue;

		if(code == D6O_RELAY_MSG && once == 0){
			once == 1;
			offset += sizeof(unsigned short); // skip L

			struct dhcpv6_packet * packt = (struct dhcpv6_packet *)(optBuff + offset);

			*pMsgType = DHCPV6_REPLY;

			if((packt->msg_type != DHCPV6_ADVERTISE) && (packt->msg_type != DHCPV6_REPLY)){
				return NULL;
			}

			if(packt->msg_type == DHCPV6_REPLY){
				return NULL;
			}


			dhcpv6Para = malloc(sizeof(dhcpv6_para_t));
			if(dhcpv6Para == NULL){
				printf("malloc failed\n");
				exit(errno);
			}

			dhcpv6Para->msg_type = packt->msg_type;
			memcpy(dhcpv6Para->transaction_id,packt->transaction_id,sizeof(packt->transaction_id));

			offset += sizeof(struct dhcpv6_packet);

			flag |= 1;
			continue;
		}

		switch(code){
			case D6O_CLIENTID:
				dhcpv6Para->clinetIdLen = ntohs(*(unsigned short *)(optBuff + offset));
				val = optBuff + offset + sizeof(unsigned short);
				memcpy(dhcpv6Para->clinetDuid,val,dhcpv6Para->clinetIdLen);
				flag |= 2;
				break;
			case D6O_IA_NA:
				offset += 14; //skip 
				continue;
				break;
			case D6O_IAADDR:
				val = optBuff + offset + sizeof(unsigned short);
				memcpy(dhcpv6Para->ipAddr,val,IPV6_ADDR_LEN);
				flag |= 4;
				break;
			default:
				break;

		}

		offset  += (ntohs(*(unsigned short *)(optBuff + offset)) + sizeof(unsigned short));//skip LV
		if((flag & 0x7) == 0x7){
			break;
		}
	}

	return ((flag & 0x7) == 0x7) ? dhcpv6Para : NULL;
}

int  dhcpv6_parse_reply(char * buffer, int buffLen,dhcpv6_para_t **dhcpv6Para)
{
	unsigned char msgType = buffer[0];
	static struct timespec start, end;
	static int ackCntStart = 0;
    double elapsed_time;


     if(start.tv_sec == 0 && start.tv_nsec == 0){
		  clock_gettime(CLOCK_REALTIME, &start);
		  ackCntStart = gAckCount;
	 }


	if ( msgType == DHCPV6_RELAY_REPL){
		msgType = 0;

		*dhcpv6Para = parse_option_buffer(buffer + sizeof(struct dhcpv6_relay_packet),buffLen - sizeof(struct dhcpv6_relay_packet),&msgType);
		
		if(msgType == DHCPV6_REPLY){
				++gAckCount;

				clock_gettime(CLOCK_REALTIME, &end);
				if(end.tv_sec - start.tv_sec > 1 ){
					elapsed_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
					memset(&start,0,sizeof(start));
					printf("=========>lps %lf\n", ((gAckCount - ackCntStart)* (1e9 /(double)elapsed_time)));
				}
		}
	}else{
		/*TODO*/
	}


	return 0;
}

static void* dhcpv6_async_task_proc(void *arg) 
{
	async_context_t *ctx = (async_context_t*)arg;
	int i = 0;

	int epfd = ctx->epfd;

	while (1) {

		struct epoll_event events[ASYNC_CLIENT_NUM] = {0};

		int nready = epoll_wait(epfd, events, ASYNC_CLIENT_NUM, -1);
		if (nready < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				continue;
			} else {
				break;
			}
		} else if (nready == 0) {
			continue;
		}

		//printf("nready:%d\n", nready);
	
		for (i = 0;i < nready;i ++) {

			struct ep_arg *data = (struct ep_arg*)events[i].data.ptr;
			int sockfd = data->sockfd;

			char buffer[1024] = {0};
			struct sockaddr_in addr;
			size_t addr_len = sizeof(struct sockaddr_in);
			int buffLen = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);

			dhcpv6_para_t * dhcpv6Para = NULL;
			dhcpv6_parse_reply(buffer,buffLen,&dhcpv6Para);
			if(dhcpv6Para != NULL)
				data->cb(dhcpv6Para); //call cb

		}

	}

}

async_context_t * async_task_init(void) 
{
	int ret = 0;
	pthread_t thread_id;
	struct epoll_event ev;

	int epfd = epoll_create(1); 
	if (epfd < 0) return NULL;

	async_context_t *ctx = calloc(1, sizeof(struct async_context));
	if (ctx == NULL) {
		close(epfd);
		return NULL;
	}
	ctx->epfd = epfd;

	struct ep_arg *eparg = (struct ep_arg*)calloc(1, sizeof(struct ep_arg));
	if (eparg == NULL) return NULL;
	eparg->sockfd = gSockFd;
	eparg->cb = async_result_proc_cb;

	ev.data.ptr = eparg;
	ev.events = EPOLLIN;

	ret = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, gSockFd, &ev); 


	ret = pthread_create(&thread_id, NULL, dhcpv6_async_task_proc, ctx);
	if (ret) {
		perror("pthread_create");
		return NULL;
	}

	return ctx;
}

int main(int argc, char **argv)
{

	char buff[1024] = {0};
	int ret = 0;
	int totalLen = 0;
	int  index         = 0;
	int serFlag = 0;

	optind = 1;

	while(-1 != ( ret = getopt_long(argc, argv, "d:p:b:r:m:i:c:s:o:a:t:", long_options, &index)) ){
		switch(ret){
			case 'd':
				if(1 != inet_pton(AF_INET6, optarg, (void*)&gPara.d_addr)){
					printf("para --dip %s error\n",optarg);
					exit(errno);
				}
				serFlag = 1;
				break;
			case 'p':
				gPara.dport = atoi(optarg);
				break;
			case 'b':
				if(1 != inet_pton(AF_INET6, optarg, (void*)&gPara.s_addr)){
					printf("para --sip %s error\n",optarg);
					exit(errno);
				}
				break;
			case 'r':
				gPara.sport = atoi(optarg);
				break;
			case 'm':
				gPara.msgType = atoi(optarg);	 
				break;
			case 'i':
				if(1 != inet_pton(AF_INET6, optarg, (void*)&gPara.r_addr)){
					printf("para --ipv6_address %s error\n",optarg);
					exit(errno);
				}
				break;
			case 'c':
				gPara.clinetIdLen =  get_hex_by_str(optarg,strlen(optarg),gPara.clinetDuid,128);	
				break;
			case 's':
				gPara.serverIdLen	= get_hex_by_str(optarg,strlen(optarg),gPara.serverDuid,128);
				break;
			case 'o':
				gPara.optLen = get_hex_by_str(optarg,strlen(optarg),gPara.optData,1023);
				break;	
			case 'a':
				gPara.threadNum = atoi(optarg);
				break;
			case 't':
				gPara.reqCount = atoi(optarg);
				break;
			case  'e':
			     gPara.speed  = atoi(optarg);
				 break;		     
			default:
				usage();
				exit(0);
				break;  
		}

	}

	if(argc < 5 || gPara.serverIdLen == 0 || gPara.dport == 0 || serFlag == 0){
		usage();
		exit(0);
	}

	gSockFd = createUdp6Socket(&gPara.s_addr,gPara.sport);
	if(gSockFd < 0){
		perror("create sockt error");
		exit(errno);
	}

	//创建线程池
	threadPoolCreate(&gPool, gPara.threadNum);
	async_task_init();


	createTask(gPara.reqCount);

    printf("press any key to exit\n");
	getchar();
	printf("\n");
	threadPoolShutdown(&gPool);
	close(gSockFd);

	return 0;

}
