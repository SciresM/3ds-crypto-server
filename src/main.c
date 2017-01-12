#include "server.h"

int self_test()
{
    uint8_t *FCRAM = (uint8_t *)0x15000000;
    u8 iv[0x10];
    u8 keyY[0x10];
    u8 zeroes[0x10];
    u8 testvec[0x10] = {0xBC, 0xC4, 0x16, 0x2C, 0x2A, 0x06, 0x91, 0xEE, 0x47, 0x18, 0x86, 0xB8, 0xEB, 0x2F, 0xB5, 0x48};
    for (int i = 0; i < 0x10; i++)
    {
        iv[i] = 0;
        keyY[i] = 0;
        zeroes[i] = 0;
    }
    
    memcpy(FCRAM, keyY, 0x10);
    
    PS_EncryptDecryptAes(0x10, (unsigned char *)testvec, (unsigned char *)testvec, PS_ALGORITHM_CBC_DEC, (0x2C | 0x80 | 0x40), iv);
    
    return memcmp(testvec, zeroes, 0x10);
}

int	main(int argc, char **argv)
{
	init();
    
    printf("Performing self test...\n");
    if (self_test() != 0)
    {
        failExit("Self test failed!\nEnsure svc/service access is patched,\nAnd that P9 crypto patches are installed.\n");
    }
    
    printf("Self test succeeded!\n\n");
    printf("Ready... Press START to exit.\n");

    auto_recovering = false;
    
	do
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}
	while (aptMainLoop() && loop());
	destroy();
	exit(0);
}
