#ifndef __DHCPV6__
#define __DHCPV6__

#include <net/if.h>
#include <arpa/inet.h>
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



#define INIA_INFO_LEN      40
#define IAADDR_INFO_LEN    24
#define IPV6_ADDR_LEN      16
#define IAID_LEN           4
#define TL_LEN              4
#define DEFAULT_DUID_LEN    14

#define DHO_PAD					0



typedef struct dhcpv6_para {
     struct in6_addr s_addr;
	 int  sport ;
	 unsigned char msg_type;
	 struct in6_addr l_addr;
     struct in6_addr r_addr;
     unsigned char transaction_id[3];
	 char  clinetDuid[128];
	 int  clinetIdLen ;
    char  serverDuid[128] ;
	int  serverIdLen ;
    char optData[1024] ;
	int  optLen ;
}dhcpv6_para_t;



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


extern int build_request(char * buff,dhcpv6_para_t * dhcpv6Para);
extern int build_dhcpv6_req(char * buff,dhcpv6_para_t * pPara);
extern void *  dhcpv6_parse_cb(char * buffer, int buffLen);
#endif //__DHCPV6__