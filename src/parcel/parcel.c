/**
 * @file parcel.c
 * @author Jason Conway (jpc@jasonconway.dev)
 * @brief Entry point for parcel client
 * @version 0.9.2
 * @date 2022-01-15
 *
 * @copyright Copyright (c) 2022 - 2024 Jason Conway. All rights reserved.
 *
 */

// TODO: console - return a "back" code when backspacing on nothing

#include "client.h"

static void catch_sigint(int sig)
{
    (void)sig;
    xprintf(RED, BOLD, "\nAborting application\n");
    exit(EXIT_FAILURE);
}

static void usage(FILE *f)
{
    static const char usage[] =
        "usage: parcel [-hd] [-a ADDR] [-p PORT] [-u NAME]\n"
        "  -a ADDR  server address (www.example.com, 111.222.333.444)\n"
        "  -p PORT  server port (3724, 9216)\n"
        "  -u NAME  username displayed alongside sent messages\n"
        "  -l       use computer login as username\n"
        "  -h       print this usage information\n";
    fprintf(f, "%s", usage);
}

int main(int argc, char **argv)
{
    signal(SIGINT, catch_sigint);

    char address[ADDRESS_MAX_LENGTH];
    memset(address, 0, ADDRESS_MAX_LENGTH);

    char port[PORT_MAX_LENGTH] = "2315";

    client_t client = { 0 };
    atomic_store(&client.keep_alive, true);
    pthread_mutex_init(&client.lock, NULL);

    xgetopt_t xgo = { 0 };
    for (ptrdiff_t opt; (opt = xgetopt(&xgo, argc, argv, "lha:p:u:")) != -1;) {
        switch (opt) {
            case 'a':
                if (strlen(xgo.arg) < ADDRESS_MAX_LENGTH) {
                    memcpy(address, xgo.arg, strlen(xgo.arg));
                    break;
                }
                xwarn("Address argument too long\n");
                break;
            case 'p':
                if (xstrrange(xgo.arg, NULL, 0, 65535)) {
                    memcpy(port, xgo.arg, strlen(xgo.arg));
                    break;
                }
                xwarn("Using default port: 2315\n");
                break;
            case 'u':
                if (strlen(xgo.arg) < USERNAME_MAX_LENGTH) {
                    size_t len = strlen(xgo.arg);
                    memcpy(client.username, xgo.arg, len);
                    break;
                }
                xwarn("Username argument too long\n");
                break;
            case 'l':
                if (!xgetlogin(client.username, USERNAME_MAX_LENGTH)) {
                    break;
                }
                xwarn("Could not determine login name\n");
                break;
            case 'h':
                usage(stdout);
                return 0;
            case ':':
                printf("Option is missing an argument\n");
                return -1;
            default:
                usage(stderr);
                return -1;
        }
    }

    if (argc < 5) {
        prompt_args(address, client.username);
    }

    if (!connect_server(&client, address, port)) {
        return -1;
    }

    pthread_t recv_ctx;
    if (pthread_create(&recv_ctx, NULL, recv_thread, (void *)&client)) {
        xalert("Unable to create receiver thread\n");
        return -1;
    }

    int thread_status[2] = { 0, 0 };

    thread_status[0] = send_thread((void *)&client);

    if (pthread_join(recv_ctx, (void **)&thread_status[1])) {
        xalert("Unable to join receiver thread\n");
        return -1;
    }

    return thread_status[0] | thread_status[1];
}
