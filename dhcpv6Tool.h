#ifndef __DHCPV6_TOOL__
#define __DHCPV6_TOOL__

/* DHCPv6 Option codes: */

#define D6O_CLIENTID				1 /* RFC3315 */
#define D6O_SERVERID				2
#define D6O_IA_NA				3
#define D6O_IA_TA				4
#define D6O_IAADDR				5
#define D6O_ORO					6
#define D6O_PREFERENCE				7
#define D6O_ELAPSED_TIME			8
#define D6O_RELAY_MSG				9
/* Option code 10 unassigned. */
#define D6O_AUTH				11
#define D6O_UNICAST				12
#define D6O_STATUS_CODE				13
#define D6O_RAPID_COMMIT			14
#define D6O_USER_CLASS				15
#define D6O_VENDOR_CLASS			16
#define D6O_VENDOR_OPTS				17
#define D6O_INTERFACE_ID			18
#define D6O_RECONF_MSG				19
#define D6O_RECONF_ACCEPT			20
#define D6O_SIP_SERVERS_DNS			21 /* RFC3319 */
#define D6O_SIP_SERVERS_ADDR			22 /* RFC3319 */
#define D6O_NAME_SERVERS			23 /* RFC3646 */
#define D6O_DOMAIN_SEARCH			24 /* RFC3646 */
#define D6O_IA_PD				25 /* RFC3633 */
#define D6O_IAPREFIX				26 /* RFC3633 */
#define D6O_NIS_SERVERS				27 /* RFC3898 */
#define D6O_NISP_SERVERS			28 /* RFC3898 */
#define D6O_NIS_DOMAIN_NAME			29 /* RFC3898 */
#define D6O_NISP_DOMAIN_NAME			30 /* RFC3898 */
#define D6O_SNTP_SERVERS			31 /* RFC4075 */
#define D6O_INFORMATION_REFRESH_TIME		32 /* RFC4242 */
#define D6O_BCMCS_SERVER_D			33 /* RFC4280 */
#define D6O_BCMCS_SERVER_A			34 /* RFC4280 */
/* 35 is unassigned */
#define D6O_GEOCONF_CIVIC			36 /* RFC4776 */
#define D6O_REMOTE_ID				37 /* RFC4649 */
#define D6O_SUBSCRIBER_ID			38 /* RFC4580 */
#define D6O_CLIENT_FQDN				39 /* RFC4704 */
#define D6O_PANA_AGENT				40 /* paa-option */
#define D6O_NEW_POSIX_TIMEZONE			41 /* RFC4833 */
#define D6O_NEW_TZDB_TIMEZONE			42 /* RFC4833 */


#define DHCPV6_SOLICIT		    1
#define DHCPV6_ADVERTISE	    2
#define DHCPV6_REQUEST		    3
#define DHCPV6_CONFIRM		    4
#define DHCPV6_RENEW		    5
#define DHCPV6_REBIND		    6
#define DHCPV6_REPLY		    7
#define DHCPV6_RELEASE		    8
#define DHCPV6_DECLINE		    9
#define DHCPV6_RECONFIGURE	   10
#define DHCPV6_INFORMATION_REQUEST 11
#define DHCPV6_RELAY_FORW	   12
#define DHCPV6_RELAY_REPL	   13
#define DHCPV6_LEASEQUERY	   14	/* RFC5007 */
#define DHCPV6_LEASEQUERY_REPLY	   15	/* RFC5007 */
#define DHCPV6_LEASEQUERY_DONE	   16	/* RFC5460 */
#define DHCPV6_LEASEQUERY_DATA	   17	/* RFC5460 */
#define DHCPV6_RECONFIGURE_REQUEST 18	/* RFC6977 */
#define DHCPV6_RECONFIGURE_REPLY   19	/* RFC6977 */
#define DHCPV6_DHCPV4_QUERY	   20	/* RFC7341 */
#define DHCPV6_DHCPV4_RESPONSE	   21	/* RFC7341 */


#define DHCPV6_RELAY_FORW	   12
#define DHCPV6_RELAY_REPL	   13

#define DEFAULT_PORT 546

#define INIA_INFO_LEN      40
#define IAADDR_INFO_LEN    24
#define IPV6_ADDR_LEN      16
#define IAID_LEN           4
#define TL_LEN              4
#define DEFAULT_DUID_LEN    14

#define DHO_PAD					0


#define ASYNC_CLIENT_NUM		1024

typedef struct  dhcpv6_tool_para{
	//char  dip[128] ;
	struct in6_addr d_addr;
	int  dport ;
	struct in6_addr s_addr;
	int  sport ;
	unsigned char msgType ;
	//char  reqIp[128] ;
	struct in6_addr r_addr;
	char  clinetDuid[128] ;
	int  clinetIdLen ;
	char  serverDuid[128] ;
	int  serverIdLen ;
	char optData[1024] ;
	int  optLen ;
	int threadNum ;
	int reqCount ;
	int speed ;
	unsigned char transaction_id[3];
}dhcpv6_tool_para_t;

typedef struct async_context {
	int epfd;
}async_context_t;

typedef struct dhcpv6_para {
	 unsigned char msg_type;
     unsigned char transaction_id[3];
	 unsigned char ipAddr[16];
	 char  clinetDuid[128];
	 int  clinetIdLen ;
}dhcpv6_para_t;

typedef void (*async_result_cb)(dhcpv6_para_t * dhcpv6Para);

typedef struct ep_arg {
	int sockfd;
	async_result_cb cb;
}ep_arg_t;



#pragma pack(1)
struct dhcpv6_relay_packet {
	unsigned char msg_type;
	unsigned char hop_count;
	unsigned char link_address[16];
	unsigned char peer_address[16];
	unsigned char options[0];
};

struct dhcpv6_packet {
	unsigned char msg_type;
	unsigned char transaction_id[3];
	unsigned char options[0];
};

#endif //__DHCPV6_TOOL__