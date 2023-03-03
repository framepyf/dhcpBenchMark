#ifndef __LOG_H__
#define __LOG_H__
#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <string.h>

#define LOG_MESSAGE_MAX_LENGTH (4096)

typedef enum  {LOG_DEBUG = 0, LOG_INFO, LOG_WARN, LOG_STATS, LOG_SHOW,LOG_ERROR } LogLevel;

void dhcpPrintLogShow(LogLevel level, const char* msg);
void  dhcpSetDebugLevel(unsigned int level);
void  dhcpDebugPrintHex(const char * name , const unsigned char* data, const int len);


#    define DHCP_LOG_INLINE(level, _fmt_, ...)                                                                                                                                                                                                                                                         \
        do {                                                                                                                                                                                                                                                                                               \
            char log_buf[LOG_MESSAGE_MAX_LENGTH] = {0};                                                                                                                                                                                                                                       \
            snprintf(log_buf,LOG_MESSAGE_MAX_LENGTH, _fmt_, ##__VA_ARGS__);                                                                                                                                                                                                                  \
            dhcpPrintLogShow(level, log_buf);                                                                                                                                                                                                                                                            \
        } while (0)

#    define DHCP_LOG_DEBUG(_fmt_, ...) DHCP_LOG_INLINE(LOG_DEBUG, _fmt_,  ##__VA_ARGS__);
#    define DHCP_LOG_WARN(_fmt_, ...) DHCP_LOG_INLINE(LOG_WARN, _fmt_,   ##__VA_ARGS__);
#    define DHCP_LOG_INFO(_fmt_, ...) DHCP_LOG_INLINE(LOG_INFO, _fmt_,  ##__VA_ARGS__);
#    define DHCP_LOG_ERROR(_fmt_, ...) DHCP_LOG_INLINE(LOG_ERROR,  _fmt_,   ##__VA_ARGS__);
#    define DHCP_LOG_STATS(_fmt_, ...) DHCP_LOG_INLINE(LOG_STATS,  _fmt_,  ##__VA_ARGS__);
#    define DHCP_LOG_SHOW(_fmt_, ...) DHCP_LOG_INLINE(LOG_SHOW,  _fmt_,  ##__VA_ARGS__);

#ifdef __cplusplus
}
#endif
#endif
