#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H
#include <3ds.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int startWith(char *str, char *start);

__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);
#endif
