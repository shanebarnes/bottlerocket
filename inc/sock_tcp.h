/**
 * @file      sock_tcp.h
 * @brief     TCP socket interface.
 * @author    Shane Barnes
 * @date      01 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_TCP_H_
#define _SOCK_TCP_H_

#include "sock_obj.h"
#include "system_types.h"

struct socktcp_info
{
    uint16_t state;     // Connection state
    uint8_t  sndwscale; // Window scale for send window
    uint8_t  rcvwscale; // Window scale for receive window
    uint32_t options;   // Options supported
    uint32_t flags;     // Flags
    uint32_t rto;       // Retransmit timeout in ms
    uint32_t mss;       // Maximum segment size supported in bytes
    uint32_t ssthresh;  // Slow-start threshold in bytes
    uint32_t cwnd;      // Send congestion window in bytes
    uint32_t sndwin;    // Send window in bytes
    uint32_t sndbuf;    // Bytes in socket send buffer including in-flight bytes
    uint32_t rcvwin;    // Receive window in bytes
    uint32_t rttcur;    // Latest RTT in ms
    uint32_t rttavg;    // Average RTT in ms
    uint32_t rttvar;    // RTT variance in ms
    uint64_t txpackets; // Total segments transmitted
    uint64_t txbytes;   // Total bytes transmitted
    uint64_t retxbytes; // Total bytes retransmitted
    uint64_t rxpackets; // Total segments received
    uint64_t rxbytes;   // Total bytes received
    uint64_t rxoobytes; // Total out-of-order bytes received
};

/**
 * @brief Get TCP socket-specific information.
 *
 * @param[in]     fd   A TCP socket file descriptor.
 * @param[in,out] info A pointer to a TCP information structure.
 *
 * @return True on success.
 */
bool socktcp_getinfo(const int32_t fd, struct socktcp_info * const info);

/**
 * @see sock_create() for interface comments.
 */
bool socktcp_create(struct sockobj * const obj);

/**
 * @see sock_destroy() for interface comments.
 */
bool socktcp_destroy(struct sockobj * const obj);

/**
 * @see sock_listen() for interface comments.
 */
bool socktcp_listen(struct sockobj * const obj, const int32_t backlog);

/**
 * @see sock_accept() for interface comments.
 */
bool socktcp_accept(struct sockobj * const listener,
                    struct sockobj * const obj);

/**
 * @see sock_connect() for interface comments.
 */
bool socktcp_connect(struct sockobj * const obj);

/**
 * @see sock_recv() for interface comments.
 */
int32_t socktcp_recv(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len);

/**
 * @see sock_send() for interface comments.
 */
int32_t socktcp_send(struct sockobj * const obj,
                     void * const buf,
                     const uint32_t len);

#endif // _SOCK_TCP_H_
