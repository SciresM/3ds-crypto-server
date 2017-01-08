#ifndef SERVER_TYPES_H
#define SERVER_TYPES_H

#include <3ds.h>
#include <netinet/in.h>

typedef struct
{
	u8					running;
	struct sockaddr_in	client_addr;
	struct sockaddr_in	server_addr;
	u32					client_length;
	s32					server_id;
	s32					client_id;
} crypto_server;

typedef struct
{
    u32 magic;
    u32 len;
    u32 keyslot;
    u32 crypto_type;
    u8  keyY[0x10];
    u8  iv[0x10];
    u8  garbage[0x3D0];
} crypto_meta_packet;

#endif
