#pragma once
#include <stdarg.h>

int kprintf(const char *fmt, ...);
int ksprintf(char *buf, const char *fmt, ...);
int kvprintf(const char *fmt, va_list args);
int kvsprintf(char *buf, const char *fmt, va_list args);
