
#include <errno.h>
#include "util.h"
#include "log.h"



int get_hex_by_str( const char *key, int key_len,unsigned char * str, int strLen)
{
	int offset = 0,byte = 0;

	for ( ; (byte < strLen) && (offset < key_len) ; ++byte) {
		sscanf(key + offset, "%02X", (unsigned int *)&str[byte]);
		offset += 2;
	}

	return byte;
}


void getRandomBytes(unsigned char *p, int len) 
{
	FILE *fp = fopen("/dev/urandom","r");

	if(fp == NULL){
		DHCP_LOG_ERROR("fopen /dev/urandom error");
		return;
	}

	if(  fread(p,len,1,fp) < 1){
		DHCP_LOG_ERROR("fread /dev/urandom error");
		return;
	}

	fclose(fp);
}
