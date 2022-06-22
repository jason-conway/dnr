/**
 * @file xutils.h
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief Shared utility functions
 * @version 0.9.2
 * @date 2022-04-06
 * 
 * @copyright Copyright (c) 2022 Jason Conway. All rights reserved.
 * 
 */

#pragma once

#include "xplatform.h"

typedef struct xgetopt_t {
	char *arg;
	int index;
	int option;
	int position;
} xgetopt_t;

enum color {
	DEF = '\0', // No color
	BLK = '0', // 30
	RED = '1', // 31
	GRN = '2', // 32
	YEL = '3', // 33
	BLU = '4', // 34
	MAG = '5', // 35
	CYA = '6', // 36
	WHT = '7'  // 37
};

enum style {
	NONE = '0',
	BOLD = '1',
	ITALIC = '3',
	UNDERLINE = '4'
};

typedef struct slice_t {
	char *data;
	size_t len;
} slice_t;

enum HTTP {
	RESPONSE_LENGTH = 4096
};

// Print a formatted string in yellow to stdout
void xwarn(const char *format, ...);

// Print a formatted string in red to stderr
void xalert(const char *format, ...);

// Print a formatted string to stdout... in color
void xprintf(enum color color, enum style style, const char *format, ...);

/**
 * @brief Duplicate a string
 * 
 * @param str pointer to original string
 * @return Pointer to duplicate string, NULL if malloc() fails
 */
char *xstrdup(const char *str);

/**
 * @brief Concatenate `count` strings right to left
 * 
 * @param count number of strings provided
 * @param ... `count` strings
 * @return pointer to concatenated string, NULL if malloc() fails
 */
char *xstrcat(size_t count, ...);

void xbasename(const char *path, char *filename);

bool xfexists(const char *filename);

/**
 * @brief Send message of len bytes across multiple send() calls if required
 * 
 * @param sockfd File descriptor of the sending socket
 * @param data Data to be sent
 * @param len Length of 'data'
 * @return Number of bytes sent
 */
ssize_t xsendall(sock_t socket, const void *data, size_t len);

/**
 * @brief Receive len-bytes into data, blocking until full
 * 
 * @param sockfd File descriptor of the connected socket
 * @param data Message buffer to place received data into
 * @param len Number of bytes to recieve
 * @return Number of bytes remaining, i.e., 0 on success
 */
ssize_t xrecvall(sock_t socket, void *data, size_t len);

bool xport_valid(char *arg);

int xgetopt(xgetopt_t *optctx, int argc, char **argv, const char *optstr);

slice_t *slice_append(slice_t *slice, const char *data, size_t len);

char *xgetpublicip(void);

size_t xutoa(uint32_t value, char *str);

int xmkdirs(size_t depth, ...);
char *xsavepath(const char *file);

void *xmemdup(void *mem, size_t len);
void *xmemchr(const void *src, int c, size_t len);
void *xmemrchr(const void *src, int c, size_t len);
void xmemprint(const void *src, size_t len);
void xmemcpy_locked(pthread_mutex_t *lock, void *dest, void *src, size_t len);
void xhexdump(const void *src, size_t len);
