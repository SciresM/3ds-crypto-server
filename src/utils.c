#include "server.h"
#include <string.h>

extern crypto_server *app_data;

// --------------------------------------------------------------------------
// string utils
int startWith(char *str, char *start)
{
	if (!str || !start)
		return (0);
	int startlen = strlen(start);
	return startlen <= strlen(str) && strncmp(str, start, startlen) == 0;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// others utils
void failExit(const char *fmt, ...)
{

	if(app_data->server_id > 0) close(app_data->server_id);
	if(app_data->client_id > 0) close(app_data->client_id);

	va_list ap;

	printf(CONSOLE_RED);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(CONSOLE_RESET);
	if (auto_recovering)
	{
		printf("\nAuto-recovery failed...\n");
		printf("Press B to exit, or START to auto_recover\n");

		while (aptMainLoop()) {
			gspWaitForVBlank();
			hidScanInput();

			u32 kDown = hidKeysDown();
			if (kDown & KEY_B) break;
			if (kDown & KEY_START) goto auto_recover;
		}
		destroy();
    	exit(0);
	}

	auto_recover:
	printf("\nAttempting to auto-recover.\n");
	destroy();
	auto_recovering = true;
	main(0, NULL);
}
// --------------------------------------------------------------------------
