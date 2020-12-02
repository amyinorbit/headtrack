//===--------------------------------------------------------------------------------------------===
// server.c - UDP server implementation.
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "server.h"
#include "htrack.h"
#include "math.h"
#include <ccore/log.h>

#include <sys/types.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

static bool server_is_running;
static pthread_t server_thread;
static int server_socket;


static void *udp_track_server(void * data) {
    CCUNUSED(data);
    CCINFO("Head tracking server now listening on 0.0.0.0:4242");
    server_is_running = true;

    double *head_in = data;
    double udp_data[6];
    while(server_is_running) {
        ssize_t bytes = recvfrom(server_socket, (void*)udp_data, sizeof(udp_data), 0, NULL, NULL);
        if(bytes < 0) {
            if(errno != EAGAIN) {
                CCWARN("server: %s", strerror(errno));
            }
            continue;
        }
        for(int i = 0; i < 6; ++i) {
            head_in[i] = lerp(
                head_in[i],
                udp_data[i],
                1.0 - 0.99 * htk_settings.input_smooth
            );
        }
    }

    CCINFO("shutting down head tracking server");
    close(server_socket);

    return NULL;
}

bool server_start(double *input) {
    CCASSERT(input);
    CCINFO("starting head tracking server");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_socket = socket(PF_INET, SOCK_DGRAM, 0);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 250000;

    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    // Bind the socket
    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        htk_settings.last_error = strerror(errno);
        CCERROR("unable to start server: %s", htk_settings.last_error);
        return false;
    } else {
        pthread_create(&server_thread, NULL, udp_track_server, input);
        return true;
    }
}

void server_stop() {
    if(!server_is_running) return;
    server_is_running = false;
    pthread_join(server_thread, NULL);

}

bool server_restart(double *input) {
    server_stop();
    return server_start(input);
}
