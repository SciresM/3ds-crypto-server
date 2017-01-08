#include "server.h"

int	main(int argc, char **argv)
{
	init();
	do
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}
	while (aptMainLoop() && loop());
	destroy();
	return 0;
}
