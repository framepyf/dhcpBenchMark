#ifndef __DHCP_UTIL__
#define __DHCP_UTIL__
#include <stdio.h>
#include <string.h>

void getRandomBytes(unsigned char *p, int len);
int  get_hex_by_str( const char *key, int key_len,unsigned char * str, int strLen);

#endif //__DHCP_UTIL__