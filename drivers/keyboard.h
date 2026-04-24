#pragma once
#include <stdint.h>
#include <stdbool.h>

#define KB_BUFFER_SIZE 256

void keyboard_init(void);

/* Returns next character (blocking) */
char keyboard_getchar(void);

/* Returns next character or 0 if none available (non-blocking) */
char keyboard_poll(void);

bool keyboard_buffer_empty(void);
