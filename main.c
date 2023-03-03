#include  <stdio.h>
#include <sys/types.h>          
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include "include/threadPool.h"
#include "include/log.h"
#include "include/udp.h"
#include "include/async.h"
#include "include/util.h"
#include "include/dhcpv6.h"

typedef struct  dhcpv6_tool_para{
	struct in6_addr d_addr;
	int  dport ;
	int threadNum ;
	int reqCount ;
	int speed ;
	dhcpv6_para_t dhcpPara;
}dhcpv6_tool_para_t;



int gSockFd = 0;
nThreadPool gPool;
dhcpv6_tool_para_t gPara;



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
	{"dbglevel",	required_argument,	0,	'l'},
	{"--help",	no_argument,	0,	'h'},
	{0,		0,			0,	0}
};


static void usage(char * pName)
{
    DHCP_LOG_SHOW("usage: %s [--dip ipv6addr] [--dport dport] [--sip ipv6addr] [--sport sport]"
	        "[--msg_type type] [--ipv6_address requestaddr]  [--client_duid clientid]  [--server_duid serverid] [--option data] [--thread threadNum]  [--count reqCount] [--speed count]\n",pName);
    DHCP_LOG_SHOW(" \t-d, --dip           dhcp server IP,for example:2001::1");
    DHCP_LOG_SHOW(" \t-p, --dport         dhcp server port,for example:547");
    DHCP_LOG_SHOW(" \t-b, --sip           tool host ip, for example:2001::80");
    DHCP_LOG_SHOW(" \t-r, --sport         tool host port , for example:547\n");
    DHCP_LOG_SHOW(" \t-m, --msg_type      send request type , for example: 1 (socilit) 3(request)");
	DHCP_LOG_SHOW(" \t-i, --ipv6_address  request ip addr, for example:2001::5\n");
	DHCP_LOG_SHOW(" \t-c, --client_duid   client duid , for example:00010001234ecc25005056b1703c");
	DHCP_LOG_SHOW(" \t-s, --server_duid   server duid , for example:000100012a3f0e760050568de3bc");
	DHCP_LOG_SHOW(" \t-o, --option        option data , for example:000600020011");
	DHCP_LOG_SHOW(" \t-a, --thread    use thread num , for example:10");
	DHCP_LOG_SHOW(" \t-t, --count  send request num, for example:10000000");
	DHCP_LOG_SHOW(" \t-e, --speed  thread packet sending rate, for example:10000");
	DHCP_LOG_SHOW(" \t-l, --dbglevel  debug level, for example: 0(DEBUG) 1(INFO), 2(WARN), 3(STATS), 4(SHOW),5(ERROR) default: INFO");
    DHCP_LOG_SHOW(" \t-h, --help          Display this help and exit\n");
	DHCP_LOG_SHOW("for example:");
	DHCP_LOG_SHOW("  %s --dip 2001::1  --dport 547 --sport 547 --sip 2001::79 --msg_type 3  --ipv6_address 2001::b  --client_duid 00010001234ecc25005056b1703c"
	             "--server_duid 000100012a3f0e760050568de3bc  --option 000600020011",pName);
	DHCP_LOG_SHOW("  %s --dip 2001::1  --dport 547 --sport 547 --sip 2001::79  --msg_type 1  --server_duid 000100012a3f0e760050568de3bc  --thread 5 --count 10000 --speed 1000",pName);
}





void sendSpecReqCb(nJob *job) 
{
	char buff[1024] = {0};
	int buffLen = 0;
     struct timespec start, end;
    double elapsed_time;
	double  singlePktTime =  0;


	if(gPara.speed > 0){
		singlePktTime =  1e9 / gPara.speed;

		/*控制每个线程的发包速率*/
		clock_gettime(CLOCK_REALTIME, &start);
	}

	buffLen = build_dhcpv6_req(buff,&gPara.dhcpPara);

	if( sendReqToServer(gSockFd,&gPara.d_addr,gPara.dport,buff,buffLen) < 0 ){
		 DHCP_LOG_ERROR("send request error: %s",strerror(errno));
	}

	if(gPara.speed > 0){
		clock_gettime(CLOCK_REALTIME, &end);

		elapsed_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
		
		if (elapsed_time < singlePktTime) {
				usleep((singlePktTime - elapsed_time)/1000);
		}
	}

	if(job->user_data)
		free(job->user_data);
	free(job);
}





void sendDhcpv6Req(nJob *job) 
{
	char buff[1024] = {0};
	int buffLen = 0;
	dhcpv6_para_t * dhcpv6Para = job->user_data;

	if(dhcpv6Para->msg_type == DHCPV6_ADVERTISE){
		/*参数copy*/
		memcpy(&dhcpv6Para->s_addr,&gPara.dhcpPara.s_addr,sizeof(gPara.dhcpPara.s_addr));
		memcpy(dhcpv6Para->serverDuid,gPara.dhcpPara.serverDuid,gPara.dhcpPara.serverIdLen);
		dhcpv6Para->serverIdLen = gPara.dhcpPara.serverIdLen;
		memcpy(dhcpv6Para->optData,gPara.dhcpPara.optData,gPara.dhcpPara.optLen);
		dhcpv6Para->optLen  = gPara.dhcpPara.optLen;

		buffLen = build_request(buff,dhcpv6Para);

		if(sendReqToServer(gSockFd,&gPara.d_addr,gPara.dport,buff,buffLen) < 0){
			DHCP_LOG_ERROR("send request error: %s",strerror(errno));
		}
	}

	if(job->user_data)
		free(job->user_data);
	free(job);
}


void async_result_proc_cb(void * para)
{
	nJob *job = (nJob*)malloc(sizeof(nJob));
	if (job == NULL) {
		DHCP_LOG_ERROR("malloc failed");
		exit(1);
	}

	job->job_function = sendDhcpv6Req;
	job->user_data = para;

	threadPoolQueue(&gPool, job);
}


int createTask(int reqCount,task_cb cb)
{
	int i = 0;
	if(reqCount < 1) reqCount = 1 ;
	for (i = 0;i < reqCount;i++) {
		nJob *job = (nJob*)malloc(sizeof(nJob));
		if (job == NULL) {
			DHCP_LOG_ERROR("malloc failed");
			exit(1);
		}

		job->job_function = cb;
		job->user_data = NULL;


		threadPoolQueue(&gPool, job);

	}
}

int main(int argc, char **argv)
{

	char buff[1024] = {0};
	int ret = 0;
	int totalLen = 0;
	int  index         = 0;
	int serFlag = 0;

	optind = 1;

	while(-1 != ( ret = getopt_long(argc, argv, "d:p:b:r:m:i:c:s:o:a:t:l:h", long_options, &index)) ){
		switch(ret){
			case 'd':
				if(1 != inet_pton(AF_INET6, optarg, (void*)&gPara.d_addr)){
					DHCP_LOG_ERROR("para --dip %s error",optarg);
					exit(errno);
				}
				serFlag = 1;
				break;
			case 'p':
				gPara.dport = atoi(optarg);
				break;
			case 'b':
				if(1 != inet_pton(AF_INET6, optarg, (void*)&gPara.dhcpPara.s_addr)){
					DHCP_LOG_ERROR("para --sip %s error",optarg);
					exit(errno);
				}
				break;
			case 'r':
				gPara.dhcpPara.sport = atoi(optarg);
				break;
			case 'm':
				gPara.dhcpPara.msg_type = atoi(optarg);	 
				break;
			case 'i':
				if(1 != inet_pton(AF_INET6, optarg, (void*)&gPara.dhcpPara.r_addr)){
					DHCP_LOG_ERROR("para --ipv6_address %s error\n",optarg);
					exit(errno);
				}
				break;
			case 'c':
				gPara.dhcpPara.clinetIdLen =  get_hex_by_str(optarg,strlen(optarg),gPara.dhcpPara.clinetDuid,128);	
				break;
			case 's':
				gPara.dhcpPara.serverIdLen	= get_hex_by_str(optarg,strlen(optarg),gPara.dhcpPara.serverDuid,128);
				break;
			case 'o':
				gPara.dhcpPara.optLen = get_hex_by_str(optarg,strlen(optarg),gPara.dhcpPara.optData,1023);
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
			case  'l':
			      dhcpSetDebugLevel(atoi(optarg));
                  break;
			case  'h':    
			default:
				usage(argv[0]);
				exit(0);
				break;  
		}

	}

	if(argc < 5 || gPara.dhcpPara.serverIdLen == 0 || gPara.dport == 0 || serFlag == 0){
		usage(argv[0]);
		exit(0);
	}

	gSockFd = createUdp6Socket(&gPara.dhcpPara.s_addr,gPara.dhcpPara.sport);
	if(gSockFd < 0){
		DHCP_LOG_ERROR("create sockt error %s",strerror(errno));
		return -1;
	}

	//创建线程池
	if( 0 != threadPoolCreate(&gPool, gPara.threadNum)){
		 DHCP_LOG_ERROR("thread pool created failed");
		 return -1;
	}


	async_task_init(gSockFd,async_result_proc_cb,dhcpv6_parse_cb);

	createTask(gPara.reqCount,sendSpecReqCb);

    DHCP_LOG_INFO("press any key to exit");
	getchar();
	DHCP_LOG_INFO("end");
	threadPoolShutdown(&gPool);
	close(gSockFd);

	return 0;

}
