#ifndef __MOS_ESP_H__
#define __MOS_ESP_H__

#ifdef MOS_ESP
#include "esp_common.h"

#include "lwip/sockets.h"
#if LWIP_IPV6
#define inet_ntop(af,src,dst,size)                                      \
    (((af) == AF_INET6) ? ip6addr_ntoa_r((const ip6_addr_t *)(src),(dst),(size)) \
     : (((af) == AF_INET) ? ipaddr_ntoa_r((const ip_addr_t *)(src),(dst),(size)) : NULL))
#define inet_pton(af,src,dst)                                           \
    (((af) == AF_INET6) ? inet6_aton((src),(const ip6_addr_t *)(dst))   \
     : (((af) == AF_INET) ? inet_aton((src),(const ip_addr_t *)(dst)) : 0))
#else /* LWIP_IPV6 */
#define inet_ntop(af,src,dst,size)                                  \
    (((af) == AF_INET) ? ipaddr_ntoa_r((src),(dst),(size)) : NULL)
#define inet_pton(af,src,dst)                           \
    (((af) == AF_INET) ? inet_aton((src),(dst)) : 0)
#endif /* LWIP_IPV6 */

#define FMT_INT64  "%jd"
#define FMT_UINT64 "%ju"
#endif

#endif
