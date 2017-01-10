#include "server.h"

static u32 *socket_buffer = NULL;
static crypto_server data;
crypto_server *app_data = &data;
static int ret;
static char payload[1024];

static crypto_meta_packet *metadata = NULL;

static u8  *crypto_buffer = NULL;

void socShutdown()
{
	printf("waiting for socExit...\n");
	socExit();
}

void init()
{
	hidInit(); // input
    psInit(); // ps, for AES
	gfxInitDefault(); // graphics
	consoleInit(GFX_TOP, NULL); // default console
	consoleDebugInit(debugDevice_CONSOLE);

	socket_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	if (socket_buffer == NULL)
		failExit("Socket buffer allocation failed!\n");
    
    crypto_buffer = (u8*)memalign(SOC_ALIGN, CRYPTO_BUFFERSIZE);
    if (crypto_buffer == NULL)
        failExit("Crypto buffer allocation failed!\n");
    
    metadata = (crypto_meta_packet *)(&payload[0]);


	// Init soc:u service
	if ((ret = socInit(socket_buffer, SOC_BUFFERSIZE)) != 0)
    	failExit("Service initialization failed! (code: 0x%08X)\n", (unsigned int)ret);

	// Make sure the struct is clear
	memset(&data, 0, sizeof(data));
	data.client_id = -1;
	data.server_id = -1;
	data.server_id = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	// Is socket accessible?
	if (data.server_id < 0)
		failExit("socket: %s (code: %d)\n", strerror(errno), errno);


	// Init server_addr on default address and port 8081
	data.server_addr.sin_family = AF_INET;
	data.server_addr.sin_port = htons(8081);
	data.server_addr.sin_addr.s_addr = gethostid();
	data.client_length = sizeof(data.client_addr);

	// Print network info
	printf("Server is running at %s.\n", inet_ntoa(data.server_addr.sin_addr));

	if ((ret = bind(data.server_id, (struct sockaddr *) &data.server_addr, sizeof(data.server_addr))))
	{
		close(data.server_id);
		failExit("bind: %s (code: %d)\n", strerror(errno), errno);
	}

	// Set socket non blocking so we can still read input to exit
	fcntl(data.server_id, F_SETFL, fcntl(data.server_id, F_GETFL, 0) | O_NONBLOCK);

	if ((ret = listen(data.server_id, 5)))
		failExit("listen: %s (code: %d)\n", strerror(errno), errno);
	data.running = 1;
	printf("Ready...Press START to exit.\n");
}

int loop()
{
	data.client_id = accept(data.server_id, (struct sockaddr *) &data.client_addr, &data.client_length);
	if (data.client_id < 0 && errno != EAGAIN)
		failExit("accept: %d %s\n", errno, strerror(errno));
	else
	{
		// set client socket to blocking to simplify sending data back
		fcntl(data.client_id, F_SETFL, fcntl(data.client_id, F_GETFL, 0) & ~O_NONBLOCK);
		// reset old payload
		memset(payload, 0, 1024);

		// Read metadata bytes
		ret	= recv(data.client_id, payload, 1024, 0);
        
        if (metadata->magic == 0xCAFEBABE)
        {
            if (metadata->crypto_type < 5 && (metadata->keyslot < 10 || (metadata->keyslot >= 0x80 && metadata->keyslot < 0x100)))
            {
                printf("Handling crypto request from %s:\n", inet_ntoa(data.client_addr.sin_addr));
                u32 bufsize = CRYPTO_BUFFERSIZE;
                send(data.client_id, (char *)(&bufsize), sizeof(bufsize), 0);
                printf("%s\n", manage_connection() ? "Failure." : "Success!");
            }
            else
            {
                printf("Crypto packet has invalid keymetadata");
            }
        }       		
		else if (strstr(payload, "HTTP/1.1")) { // HTTP 1.1?
            printf("Received non-crypto HTTP packet!\n");
        }

		// End connection
		close(data.client_id);
		data.client_id = -1;
	}
	return data.running;
}

void destroy()
{
	close(data.server_id);
	socShutdown();
	gfxExit();
	hidExit();
    psExit();
}

int manage_connection()
{
    uint8_t *FCRAM = (uint8_t *)0x15000000;
    
    u8 iv[0x10];
    
    PS_AESAlgorithm algo = (PS_AESAlgorithm)metadata->crypto_type;
    
    PS_AESKeyType ktype = (PS_AESKeyType)metadata->keyslot;
    
    memcpy(FCRAM, metadata->keyY, 0x10);
    memcpy(iv, metadata->iv, 0x10);
    

    const char * algos[6] = {"CBC Enc", "CBC Dec", "CTR Enc", "CTR Dec", "CCM Enc", "CCM Dec"};
    const int ktypes[10] = {0xD, 0x2D, 0x31, 0x38, 0x32, 0x39, 0x2E, 0x7, 0x36, 0x39};
    
    int usedkeyslot = (ktype & 0x80) ? (ktype & 0x3F) : (ktypes[ktype]);
    
    printf("Crypto: %s, slot 0x%X, 0x%X bytes.\n", algos[algo], (int)usedkeyslot, metadata->len);
    
    
    printf("Sending/Receiving packets...\r");
    u32 num_packets = metadata->len / CRYPTO_BUFFERSIZE;
    if (metadata->len % CRYPTO_BUFFERSIZE)
    {
        num_packets++;
    }
    
    u32 percent = 0;
    
    ssize_t r;
    ssize_t expected;
    
    u32 recv_cnt;
    
    for (u32 i = 0; i < num_packets; i++)
    {
        r = 0;
        recv_cnt = 0;
        expected = ((i == num_packets - 1) && (metadata->len % CRYPTO_BUFFERSIZE != 0)) ? (metadata->len % CRYPTO_BUFFERSIZE) : CRYPTO_BUFFERSIZE;
        do
        {
            r += recv(data.client_id, crypto_buffer + r, expected - r, 0);
            recv_cnt++;
            if (r > expected || recv_cnt > MAX_RECV_TRIES)
            {
                printf("Sending/Receiving packets... failed.\n");
                return 1;
            }
        } while (r != expected);
        
        PS_EncryptDecryptAes(CRYPTO_BUFFERSIZE, (unsigned char *)crypto_buffer, (unsigned char *)crypto_buffer, algo, ktype, iv);  
        
        ret = send(data.client_id, crypto_buffer, expected, 0);
        
        if (((100 * i) / num_packets) > percent)
        {
            percent = ((100 * i) / num_packets);
            printf("Sending/Receiving packets... %lu%%\r", percent);
        }
    }
    printf("Sending/Receiving packets... 100%%\n");
    
    expected = 4;
    r = 0;
    recv_cnt = 0;
    do
    {
        r += recv(data.client_id, crypto_buffer + r, expected - r, 0);
        recv_cnt++;
        if (r > expected || recv_cnt > MAX_RECV_TRIES)
        {
            printf("Client failed to acknowledge success.\n");
            return 1;
        }
    } while (r != expected);

	return (*((u32 *)crypto_buffer) == 0xDEADCAFE) ? 0 : 1;
}
