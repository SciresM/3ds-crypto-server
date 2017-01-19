#include "server.h"

static void *initial_stack_ptr = NULL;

#define SELF_TEST_FAIL 0
#define SELF_TEST_RETAIL 1
#define SELF_TEST_DEV 2

int self_test()
{
    uint8_t *FCRAM = (uint8_t *)0x15000000;
    u8 iv[0x10];
    u8 keyY[0x10];
    u8 zeroes[0x10];
    u8 retailvec[0x10] = {0xBC, 0xC4, 0x16, 0x2C, 0x2A, 0x06, 0x91, 0xEE, 0x47, 0x18, 0x86, 0xB8, 0xEB, 0x2F, 0xB5, 0x48};
    u8 devvec[0x10] = {0x29, 0xB5, 0x5D, 0x9F, 0x61, 0xAC, 0xD2, 0x28, 0x22, 0x23, 0xFB, 0x57, 0xDD, 0x50, 0x8A, 0xF5};

    for (int i = 0; i < 0x10; i++)
    {
        iv[i] = 0;
        keyY[i] = 0;
        zeroes[i] = 0;
    }
    
    memcpy(FCRAM, keyY, 0x10);
    
    PS_EncryptDecryptAes(0x10, (unsigned char *)retailvec, (unsigned char *)retailvec, PS_ALGORITHM_CBC_DEC, (0x2C | 0x80 | 0x40), iv);
    if (memcmp(retailvec, zeroes, 0x10) == 0) 
    {
        return SELF_TEST_RETAIL;
    }

    for (int i = 0; i < 0x10; i++)
    {
        iv[i] = 0;
        keyY[i] = 0;
        zeroes[i] = 0;
    }
    
    memcpy(FCRAM, keyY, 0x10);
    
    PS_EncryptDecryptAes(0x10, (unsigned char *)devvec, (unsigned char *)devvec, PS_ALGORITHM_CBC_DEC, (0x2C | 0x80 | 0x40), iv);
    if (memcmp(devvec, zeroes, 0x10) == 0)
    {
        return SELF_TEST_DEV;
    }

    return SELF_TEST_FAIL;
}

int	main(int argc, char **argv)
{
    if (initial_stack_ptr == NULL)
    {
        register int sp asm ("sp");
        initial_stack_ptr = (void *)sp;
        auto_recovering = false;
    }
    else 
    {
        asm("mov sp, %[isp]" : : [isp]"r"(initial_stack_ptr));
    }
	init();
    
    printf("Performing self test...\n");
    int test = self_test();
    if (test == SELF_TEST_FAIL)
    {
        failExit("Self test failed!\nEnsure svc/service access is patched,\nAnd that P9 crypto patches are installed.\n\nIf you are sure this is the case, try again.");
    }
    else if (test == SELF_TEST_RETAIL)
    {
        printf("Self test succeeded! Retail keys detected.\n\n");
    }
    else if (test == SELF_TEST_DEV)
    {
        printf("Self test succeeded! Devkit keys detected.\n\n");
    }
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
