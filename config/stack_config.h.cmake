/*
 * config.h (template for cmake)
 *
 * Contains global defines to configure the stack. You need to recompile the stack to make
 * changes effective (make clean; make)
 *
 */

#ifndef STACK_CONFIG_H_
#define STACK_CONFIG_H_

/* set to 0 for a little-endian target, 1 for a big-endian target */
#cmakedefine01 PLATFORM_IS_BIGENDIAN

/* define if the system supports clock_gettime */
#cmakedefine CONFIG_SYSTEM_HAS_CLOCK_GETTIME

/* include asserts if set to 1 */
#cmakedefine01 DEBUG

#cmakedefine01 DEBUG_SOCKET


/* activate TCP keep alive mechanism. 1 -> activate */
#cmakedefine01 CONFIG_ACTIVATE_TCP_KEEPALIVE

/* time (in s) between last message and first keepalive message */
#define CONFIG_TCP_KEEPALIVE_IDLE 5

/* time between subsequent keepalive messages if no ack received */
#define CONFIG_TCP_KEEPALIVE_INTERVAL 2

/* number of not missing keepalive responses until socket is considered dead */
#define CONFIG_TCP_KEEPALIVE_CNT 2

/* timeout while reading from TCP stream in ms */
#define CONFIG_TCP_READ_TIMEOUT_MS 1000

#endif /* STACK_CONFIG_H_ */
