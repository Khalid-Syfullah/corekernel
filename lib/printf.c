#include "printf.h"
#include "string.h"
#include <vga.h>

static void print_pad(char *buf, int *pos, char c, int width, bool left_align) {
    (void)buf; (void)pos; (void)c; (void)width; (void)left_align;
}

/* Write char to vga or buffer */
typedef struct {
    char *buf;
    int   idx;
    int   max;
} print_ctx_t;

static void ctx_putc(print_ctx_t *ctx, char c) {
    if (ctx->buf) {
        if (ctx->idx < ctx->max - 1) ctx->buf[ctx->idx++] = c;
    } else {
        vga_put_char(c);
        ctx->idx++;
    }
}

static void ctx_puts(print_ctx_t *ctx, const char *s) {
    while (*s) ctx_putc(ctx, *s++);
}

static void ctx_pad(print_ctx_t *ctx, int width, bool left, const char *s, char pad) {
    int len = (int)strlen(s);
    if (!left) { for (int i = len; i < width; i++) ctx_putc(ctx, pad); }
    ctx_puts(ctx, s);
    if (left)  { for (int i = len; i < width; i++) ctx_putc(ctx, ' '); }
}

int kvsprintf(char *buf, const char *fmt, va_list args) {
    print_ctx_t ctx = { .buf = buf, .idx = 0, .max = buf ? 0x7FFFFFFF : 0 };
    char tmp[32];

    for (; *fmt; fmt++) {
        if (*fmt != '%') { ctx_putc(&ctx, *fmt); continue; }
        fmt++;

        bool left_align = false, zero_pad = false, has_width = false;
        int  width = 0;

        if (*fmt == '-') { left_align = true; fmt++; }
        if (*fmt == '0') { zero_pad  = true;  fmt++; }
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            has_width = true;
            fmt++;
        }

        char pad = (zero_pad && !left_align) ? '0' : ' ';

        switch (*fmt) {
        case 'd': case 'i': {
            int v = va_arg(args, int);
            itoa(v, tmp, 10);
            ctx_pad(&ctx, has_width ? width : 0, left_align, tmp, pad);
            break;
        }
        case 'u': {
            unsigned v = va_arg(args, unsigned);
            utoa(v, tmp, 10);
            ctx_pad(&ctx, has_width ? width : 0, left_align, tmp, pad);
            break;
        }
        case 'x': {
            unsigned v = va_arg(args, unsigned);
            utoa(v, tmp, 16);
            ctx_pad(&ctx, has_width ? width : 0, left_align, tmp, pad);
            break;
        }
        case 'X': {
            unsigned v = va_arg(args, unsigned);
            utoa(v, tmp, 16);
            for (int i = 0; tmp[i]; i++)
                if (tmp[i] >= 'a' && tmp[i] <= 'f') tmp[i] -= 32;
            ctx_pad(&ctx, has_width ? width : 0, left_align, tmp, pad);
            break;
        }
        case 'o': {
            unsigned v = va_arg(args, unsigned);
            utoa(v, tmp, 8);
            ctx_pad(&ctx, has_width ? width : 0, left_align, tmp, pad);
            break;
        }
        case 'p': {
            unsigned v = (unsigned)(uintptr_t)va_arg(args, void *);
            tmp[0] = '0'; tmp[1] = 'x';
            utoa(v, tmp + 2, 16);
            ctx_puts(&ctx, tmp);
            break;
        }
        case 'c': {
            char c = (char)va_arg(args, int);
            ctx_putc(&ctx, c);
            break;
        }
        case 's': {
            const char *s = va_arg(args, const char *);
            if (!s) s = "(null)";
            ctx_pad(&ctx, has_width ? width : 0, left_align, s, ' ');
            break;
        }
        case '%':
            ctx_putc(&ctx, '%');
            break;
        case 'n': /* deliberately not supported */ break;
        default:
            ctx_putc(&ctx, '%');
            ctx_putc(&ctx, *fmt);
        }
    }
    if (buf) buf[ctx.idx] = '\0';
    return ctx.idx;
}

int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = kvsprintf(NULL, fmt, args);
    va_end(args);
    return n;
}

int ksprintf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = kvsprintf(buf, fmt, args);
    va_end(args);
    return n;
}

int kvprintf(const char *fmt, va_list args) {
    return kvsprintf(NULL, fmt, args);
}
