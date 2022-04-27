/**
 * @file xplatform.h
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief xplatform (cross-platform) methods for Parcel
 * @version 0.9.1
 * @date 2022-01-26
 *
 * @copyright Copyright (c) 2022 Jason Conway. All rights reserved.
 *
 */

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/types.h>

#if __unix__ || __APPLE__
	#include <unistd.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <termios.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <ifaddrs.h>
	#include <dirent.h>
	typedef int sock_t;
#endif

#if _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2ipdef.h>
	#include <ws2tcpip.h>
	#include <mstcpip.h>
	#include <iphlpapi.h>
	#include <ntsecapi.h>
	#include <winbase.h>
	#include <io.h>
	#ifdef _MSC_VER
		#pragma comment(lib, "ws2_32.lib")
		#pragma comment(lib, "advapi32")
		#pragma comment(lib, "IPHLPAPI.lib")
	#endif

	#if __MINGW32__
		// char *inet_ntop(int af, const void *src, char *dst, size_t size);
	#endif

	typedef unsigned short in_port_t;
	typedef SOCKET sock_t;
#endif

void *xmalloc(size_t len);
void *xcalloc(size_t len);
void *xrealloc(void *mem, size_t len);
void xfree(void *mem);

int xsocket(sock_t *xsocket, int domain, int type, int protocol);
int xaccept(sock_t *connection_socket, sock_t listening_socket, struct sockaddr *address, socklen_t *len);

ssize_t xsend(sock_t socket, const void *data, size_t len, int flags);
ssize_t xrecv(sock_t socket, void *data, size_t len, int flags);

size_t xfd_count(sock_t fd, size_t count);
size_t xfd_init_count(sock_t fd);
sock_t xfd_isset(fd_set *set, fd_set *read_fds, size_t index);
sock_t xfd_inset(fd_set *set, size_t index);

int xgetaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
int xgetpeername(sock_t socket, struct sockaddr *address, socklen_t *len);
int xgetpeeraddr(sock_t socket, char *address, in_port_t *port);

int xsetsockopt(sock_t socket, int level, int optname, const void *optval, socklen_t optlen);
int xclose(sock_t socket);
void xexit(int status);

int xgetifaddrs(void);
int xstartup(void);

int xgetlogin(char *username, size_t len);
void xgetrandom(void *dest, size_t len);
size_t xfilesize(const char *filename);
char *xget_dir(char *file);
