/**
 * @file parceld.c
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief Entry point for parcel daemon
 * @version 0.9.2
 * @date 2022-01-28
 *
 * @copyright Copyright (c) 2022 Jason Conway. All rights reserved.
 *
 */

#include "daemon.h"

static void usage(FILE *f)
{
	static const char usage[] =
		"usage: parceld [-h] [-p PORT] [-M CONNS]\n"
		"  -p PORT  start daemon on port PORT\n"
		"  -q LMAX  limit length of pending connections queue to LMAX\n"
		"  -m CMAX  limit number of active server connections to CMAX\n"
		"  -h        print this usage information\n";
	fprintf(f, "%s", usage);
}

int main(int argc, char *argv[])
{
	server_t server = {
		.server_port = "2315"
	};

	int option;
	xgetopt_t optctx = { 0 };

	while ((option = xgetopt(&optctx, argc, argv, "hp:q:m:")) != -1) {
		switch (option) {
			case 'p':
				if (xport_valid(optctx.arg)) {
					memcpy(server.server_port, optctx.arg, strlen(optctx.arg));
				}
				break;
			case 'h':
				usage(stdout);
				return 0;
			case ':':
				printf("Option is missing an argument\n");
				return 1;
		}
	}

	if (init_daemon(&server)) {
		return 1;
	}

	if (display_daemon_info(&server)) {
		return 1;
	}

	return main_thread((void *)&server);
}
