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
	printf("\nPress B to exit\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) break;
	}
    destroy();
    exit(0);
}
// --------------------------------------------------------------------------
