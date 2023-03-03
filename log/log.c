
#include "include/log.h"

#define DHCP_STREAM_TYPE (stdout)
#define DHCP_STREAM_COLOR_STATE (1)

int gDbgLevel = LOG_INFO;

void  dhcpSetDebugLevel(unsigned int level)
{
    if(level > LOG_ERROR){
         return;
    }
    
    gDbgLevel  = level;
}

void dhcpPrintLogShow(LogLevel level, const char* msg)
{
    if (level >= gDbgLevel) {
        switch (level) {
            case LOG_DEBUG: {
                fprintf(DHCP_STREAM_TYPE, "[Debug] %s\n", msg);
                break;
            }
            case LOG_WARN: {
                if (DHCP_STREAM_COLOR_STATE) {
                    fprintf(DHCP_STREAM_TYPE, "\033[33m[Warning] %s\n\033[0m", msg);
                }
                else {
                    fprintf(DHCP_STREAM_TYPE, "[Warning] %s\n", msg);
                }
                break;
            }
            case LOG_INFO: {
                if (DHCP_STREAM_COLOR_STATE) {
                    fprintf(DHCP_STREAM_TYPE, "\033[32m[Info] %s\n\033[0m", msg);
                }
                else {
                    fprintf(DHCP_STREAM_TYPE, "[Info] %s\n", msg);
                }
                break;
            }
            case LOG_ERROR: {
                if (DHCP_STREAM_COLOR_STATE) {
                    fprintf(DHCP_STREAM_TYPE, "\033[31m[Error] %s\n\033[0m", msg);
                }
                else {
                    fprintf(DHCP_STREAM_TYPE, "[Error] %s\n", msg);
                }
                break;
            }
            case LOG_STATS: {
                if (DHCP_STREAM_COLOR_STATE) {
                    fprintf(DHCP_STREAM_TYPE, "\033[34m[Stats] %s\n\033[0m", msg);
                }
                else {
                    fprintf(DHCP_STREAM_TYPE, "[Stats] %s\n", msg);
                }
                break;
            }
            case LOG_SHOW: {
                if (DHCP_STREAM_COLOR_STATE) {
                    fprintf(DHCP_STREAM_TYPE, "\033[34m%s\n\033[0m", msg);
                }else{
                    fprintf(DHCP_STREAM_TYPE, "%s\n", msg);
                }
                break;
            }
            default: break;
        }
        fflush(DHCP_STREAM_TYPE);
    }
}


void dhcpDebugPrintHex( const char * name , const unsigned char* data, const int len)
{
    if (LOG_DEBUG < gDbgLevel) {
        return;
    }

    if (len == 0 || data == NULL || len > 63) {
        return;
    }

    char buff[128] = {0};
    int index  = 0,offset = 0;

    if(name != NULL){
        offset +=sprintf(buff + offset, "%s", name);
    }

    for (; index < len; ++index) {
        offset +=sprintf(buff + offset, "%02x", data[index]);
    }

    dhcpPrintLogShow(LOG_DEBUG, buff);

}



