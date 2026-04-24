#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8

bool serial_init(uint16_t port);
void serial_putchar(uint16_t port, char c);
void serial_puts(uint16_t port, const char *str);
char serial_getchar(uint16_t port);
bool serial_received(uint16_t port);
