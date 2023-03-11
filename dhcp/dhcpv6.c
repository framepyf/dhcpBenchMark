#include <time.h>
#include <stdlib.h>
#include "log.h"
#include "dhcpv6.h"

int  gAckCount = 0;

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

int is_zero_tran_id(char * transId)
{
    return (transId[0] == 0 &&  transId[1] == 0 && transId[2] == 0) ;
}

int build_dhcpv6_packet_header(char * buffer,unsigned char msgType,char * transId)
{
	struct dhcpv6_packet  * packet = ( struct dhcpv6_packet  *)buffer;
    char id[3] = {0};

   if(is_zero_tran_id(transId))
	   getRandomBytes(id,3);
    else
       memcpy(id,transId,3);

	packet->msg_type = msgType;
	memcpy(packet->transaction_id,id,3);

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

			*pMsgType = packt->msg_type;

			DHCP_LOG_DEBUG("recv packet type:%d",packt->msg_type);

			if((packt->msg_type != DHCPV6_ADVERTISE) && (packt->msg_type != DHCPV6_REPLY)){
                DHCP_LOG_INFO("The dhcp packet type  is not advertise or reply\n");
				goto out;
			}

			if(packt->msg_type == DHCPV6_REPLY){
				goto out;
			}


			dhcpv6Para = malloc(sizeof(dhcpv6_para_t));
			if(dhcpv6Para == NULL){
				DHCP_LOG_ERROR("malloc failed\n");
                goto out;
			}

			dhcpv6Para->msg_type = packt->msg_type;
			memcpy(dhcpv6Para->transaction_id,packt->transaction_id,sizeof(packt->transaction_id));
			dhcpDebugPrintHex("transid:",packt->transaction_id,3);

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
			    dhcpDebugPrintHex("client duid:",dhcpv6Para->clinetDuid,dhcpv6Para->clinetIdLen);
				break;
			case D6O_SERVERID:
				dhcpv6Para->serverIdLen = ntohs(*(unsigned short *)(optBuff + offset));
				val = optBuff + offset + sizeof(unsigned short);
				memcpy(dhcpv6Para->serverDuid,val,dhcpv6Para->serverIdLen);
				flag |= 4;
			    dhcpDebugPrintHex("server duid:",dhcpv6Para->serverDuid,dhcpv6Para->serverIdLen);
				break;
			case D6O_IA_NA:
				offset += 14; //skip 
				continue;
				break;
			case D6O_IAADDR:
				val = optBuff + offset + sizeof(unsigned short);
				memcpy(&dhcpv6Para->r_addr,val,sizeof(dhcpv6Para->r_addr));
				dhcpDebugPrintHex("ip addr:",(const unsigned char *)&dhcpv6Para->r_addr,sizeof(dhcpv6Para->r_addr));
				flag |= 8;
				break;
			default:
				break;

		}

		offset  += (ntohs(*(unsigned short *)(optBuff + offset)) + sizeof(unsigned short));//skip LV
		if((flag & 0xf) == 0xf){
			break;
		}
	}

    DHCP_LOG_DEBUG("flag:%d",flag);
out:
    if((flag & 0xf) != 0xf){
         if(dhcpv6Para){
            free(dhcpv6Para);
            dhcpv6Para = NULL;
         }

    } 

	return dhcpv6Para;
}


int build_dhcpv6_req(char * buff,dhcpv6_para_t * pPara)
{

	int relayHeaderLen = 0;
	int totalLen = 0;
	unsigned short * pLen = NULL;
     char  clinetDuid[128] = {0};
	 int  clinetIdLen  = 0;


	relayHeaderLen  = build_relay_msg_header(buff,&pPara->s_addr,&pLen);
	totalLen += relayHeaderLen;

	totalLen += build_dhcpv6_packet_header(buff + totalLen,pPara->msg_type,pPara->transaction_id);

    if(pPara->clinetIdLen != 0){
        memcpy(clinetDuid,pPara->clinetDuid,pPara->clinetIdLen);
        clinetIdLen = pPara->clinetIdLen;
    }
	totalLen += build_dhcpv6_id_option(buff + totalLen,D6O_CLIENTID,clinetDuid,&clinetIdLen);     

	if(pPara->msg_type  != DHCPV6_SOLICIT){
		totalLen +=  build_dhcpv6_id_option(buff + totalLen,D6O_SERVERID,pPara->serverDuid,&(pPara->serverIdLen));    
	}    

	totalLen +=  build_dhcpv6_iana_info(buff + totalLen,clinetIdLen > IAID_LEN ? clinetDuid + clinetIdLen - IAID_LEN : NULL ,&pPara->r_addr);

	totalLen +=  build_dhcpv6_option_data(buff + totalLen,pPara->optData,pPara->optLen);

	// 设置D6O_RELAY_MSG内容长度
	*pLen = htons(totalLen - relayHeaderLen);

	return totalLen;
}

int build_request(char * buff,dhcpv6_para_t * dhcpv6Para)
{

	dhcpv6Para->msg_type = DHCPV6_REQUEST;

	return build_dhcpv6_req(buff,dhcpv6Para);
}


void *  dhcpv6_parse_cb(char * buffer, int buffLen)
{
	unsigned char msgType = buffer[0];
	static struct timespec start, end;
    double elapsed_time;
	dhcpv6_para_t *dhcpv6Para = NULL;


     if(start.tv_sec == 0 && start.tv_nsec == 0){
		  clock_gettime(CLOCK_REALTIME, &start);
	 }


	if ( msgType == DHCPV6_RELAY_REPL){
		msgType = 0;

		dhcpv6Para = parse_option_buffer(buffer + sizeof(struct dhcpv6_relay_packet),buffLen - sizeof(struct dhcpv6_relay_packet),&msgType);
		
		if(msgType == DHCPV6_REPLY){
				++gAckCount;

				clock_gettime(CLOCK_REALTIME, &end);
				if(end.tv_sec - start.tv_sec > 1 ){
					elapsed_time = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
					memset(&start,0,sizeof(start));

					DHCP_LOG_INFO("=========>lps %lf\n", ((gAckCount)* (1e9 /(double)elapsed_time)));
					gAckCount = 0;
				}
		}
	}else{
		/*TODO*/
	}


	return dhcpv6Para;
}

