#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "server_types.h"
#include "utils.h"

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000
#define CRYPTO_BUFFERSIZE 0x10000

#define MAX_RECV_TRIES 0x400

void init();

int self_test();

int  loop();
void destroy();
int manage_connection();
#endif