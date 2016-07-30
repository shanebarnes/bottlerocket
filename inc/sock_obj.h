/**
 * @file      sock_obj.h
 * @brief     Socket object interface.
 * @author    Shane Barnes
 * @date      03 Mar 2016
 * @copyright Copyright 2016 Shane Barnes. All rights reserved.
 *            This project is released under the MIT license.
 */

#ifndef _SOCK_OBJ_H_
#define _SOCK_OBJ_H_

#include "fion_obj.h"
#include "system_types.h"
#include "token_bucket.h"
#include "util_stats.h"
#include "vector.h"

#include <netdb.h>
#include <netinet/in.h>

struct sockobj;

struct sockobj_ops
{
    /**
     * @brief Create a socket object.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True if a socket object was created.
     */
    bool (*sock_create)(struct sockobj * const obj);

    /**
     * brief Destroy a socket object.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True if a socket object was destroyed.
     */
    bool (*sock_destroy)(struct sockobj * const obj);

    /**
     * @brief Open a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_open)(struct sockobj * const obj);

    /**
     * @brief Close a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_close)(struct sockobj * const obj);

    /**
     * @brief Assign an address to a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_bind)(struct sockobj * const obj);

    /**
     * @brief Listen for connections on an open socket.
     *
     * @param[in,out] obj     A pointer to a socket object.
     * @param[in]     backlog The maximum length of the pending connection
     *                        queue for the socket.
     *
     * @return True on success.
     */
    bool (*sock_listen)(struct sockobj * const obj,
                        const int32_t backlog);

    /**
     * @brief Accept a connection on a listener socket.
     *
     * @param[in,out] listener  A pointer to a listener socket object.
     * @param[in,out] obj       A pointer to a socket object to initialize
     *                          with a new socket if a connection was accepted.
     *
     * @return True if a connection was accepted on a listener socket.
     */
    bool (*sock_accept)(struct sockobj * const listener,
                        struct sockobj * const obj);

    /**
     * @brief Initiate a connection on a socket.
     *
     * @param[in,out] obj  A pointer to a socket object.
     *
     * @return True on success.
     */
    bool (*sock_connect)(struct sockobj * const obj);

    /**
     * @brief Get socket options.
     *
     * @param[in,out] obj  A pointer to a socket object.
     * @param[in]     opts A pointer to a vector of socket options.
     *
     * @return True on success.
     */
    bool (*sock_getopts)(struct sockobj * const obj,
                         struct vector * const opts);

    /**
     * @brief Set socket options.
     *
     * @param[in,out] obj  A pointer to a socket object.
     * @param[in]     opts A pointer to a vector of socket options.
     *
     * @return True on success.
     */
    bool (*sock_setopts)(struct sockobj * const obj,
                         struct vector * const opts);

    /**
     * @brief Receive data from a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     * @param[in,out] buf A pointer to a buffer to store data received from the
     *                    socket.
     * @param[in]     len The maximum size of the receive buffer in bytes.
     *
     * @return The number of bytes received from the socket (-1 on error).
     */
    int32_t (*sock_recv)(struct sockobj * const obj,
                         void * const buf,
                         const uint32_t len);

    /**
     * @brief Send data to a socket.
     *
     * @param[in,out] obj A pointer to a socket object.
     * @param[in]     buf A pointer to a buffer containing data to be sent to
     *                    the socket.
     * @param[in]     len The maximum number of bytes in the send buffer.
     *
     * @return The number of bytes sent to the socket (-1 on error).
     */
    int32_t (*sock_send)(struct sockobj * const obj,
                         void * const buf,
                         const uint32_t len);

    /**
     * @brief Shutdown all or part of a full-duplex socket connection
     *
     * @param[in,out] obj A pointer to a socket object.
     * @param[in]     how The shutdown method to apply (SHUT_RD, SHUT_WR, or
     *                SHUT_RDWR).
     *
     * @return True on success.
     */
    bool (*sock_shutdown)(struct sockobj * const obj, const int32_t how);
};

enum sockobj_model
{
    SOCKOBJ_MODEL_NULL   = 0x00,
    SOCKOBJ_MODEL_CLIENT = 0x01,
    SOCKOBJ_MODEL_SERVER = 0x02,
    SOCKOBJ_MODEL_PEER2P = 0x03
};

enum sockobj_state
{
    SOCKOBJ_STATE_NULL    = 0x00,
    SOCKOBJ_STATE_CLOSE   = 0x01,
    SOCKOBJ_STATE_OPEN    = 0x02,
    SOCKOBJ_STATE_BIND    = 0x04,
    SOCKOBJ_STATE_LISTEN  = 0x08,
    SOCKOBJ_STATE_CONNECT = 0x10,
};

struct sockobj_addr
{
    struct sockaddr_storage sockaddr;
    char                    ipaddr[INET6_ADDRSTRLEN];
    uint16_t                ipport;
    char                    sockaddrstr[INET6_ADDRSTRLEN + 6]; // <addr>:<port>
};

struct sockobj_opt
{
    int32_t   level;
    int32_t   name;
    uint32_t  val;
    socklen_t len;
};

struct sockobj_conf
{
    int32_t            family; // e.g., AF_INET, AF_INET6
    int32_t            type;   // e.g.: SOCK_DGRAM, SOCK_STREAM
    char               ipaddr[INET6_ADDRSTRLEN];
    uint16_t           ipport;
    int32_t            backlog;
    enum sockobj_model model;
    int32_t            timeoutms;
    uint64_t           datalimitbyte;
    uint64_t           ratelimitbps;
    uint64_t           timelimitusec;
    struct vector      opts;
};

struct sockobj_flowstats
{
    int32_t              winsize;     // socket window size
    bool                 lastcall;    // true/false for successful/failed socket function call
    uint64_t             lasttsus;    // last socket timestamp
    uint64_t             passedtsus;  // total successful socket function call time
    uint64_t             failedtsus;  // total failed socket function call time
    uint64_t             passedcalls; // successful socket function call count
    uint64_t             failedcalls; // failed socket function call count
    struct utilstats_qty buflen;      // buffer size passed to/from socket function
    uint64_t             totalbytes;  // total bytes passed to/from socket funcion
};

struct sockobj_info
{
    uint64_t                 startusec;
    uint64_t                 stopusec;
    struct sockobj_flowstats recv;
    struct sockobj_flowstats send;
};

struct sockobj
{
    struct sockobj_info  info;
    struct sockobj_ops   ops;
    struct fionobj       event;
    struct tokenbucket   tb;
    int32_t              fd;
    uint32_t             id;
    struct sockobj_addr  addrself,
                         addrpeer;
    struct addrinfo      ainfo,
                        *alist;
    struct sockobj_conf  conf;
    enum sockobj_state   state;
};

/**
 * @brief Get the peer (remote) socket address.
 *
 * @param[in,out] obj A pointer to a socket object.
 *
 * @return True if the peer socket address was obtained.
 */
bool sockobj_getaddrpeer(struct sockobj * const obj);

/**
 * @brief Get the self (local) socket address.
 *
 * @param[in,out] obj A pointer to a socket object.
 *
 * @return True if the self socket address was obtained.
 */
bool sockobj_getaddrself(struct sockobj * const obj);

/**
 * @brief Determine if an error number is fatal.
 *
 * @param[in] err An error number set by a system call.
 *
 * @return True if the error number is fatal.
 */
bool sockobj_iserrfatal(const int32_t err);

/**
 * @see sock_create() for interface comments.
 */
bool sockobj_create(struct sockobj * const obj);

/**
 * @see sock_destroy() for interface comments.
 */
bool sockobj_destroy(struct sockobj * const obj);

/**
 * @see sock_open() for interface comments.
 */
bool sockobj_open(struct sockobj * const obj);

/**
 * @see sock_close() for interface comments.
 */
bool sockobj_close(struct sockobj * const obj);

/**
 * @see sock_bind() for interface comments.
 */
bool sockobj_bind(struct sockobj * const obj);

/**
 * @see sock_getopts() for interface comments.
 */
bool sockobj_getopts(struct sockobj * const obj, struct vector * const opts);

/**
 * @see sock_setopts() for interface comments.
 */
bool sockobj_setopts(struct sockobj * const obj, struct vector * const opts);

/**
 * @brief Update a flow stats structure.
 *
 * @param[in,out] stats A pointer to a flow stats structure to update.
 * @param[in]     len   The length of buffer passed to/from a socket function.
 *
 * @return True if a flow stats structure was updated.
 */
bool sockobj_setstats(struct sockobj_flowstats * const stats, const int32_t len);

#endif // _SOCK_OBJ_H_
