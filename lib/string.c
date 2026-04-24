#include "string.h"

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = dest;
    const uint8_t *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *d = dest;
    const uint8_t *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void *a, const void *b, size_t n) {
    const uint8_t *pa = a, *pb = b;
    while (n--) {
        if (*pa != *pb) return *pa - *pb;
        pa++; pb++;
    }
    return 0;
}

size_t strlen(const char *s) {
    size_t n = 0;
    while (*s++) n++;
    return n;
}

size_t strnlen(const char *s, size_t max) {
    size_t n = 0;
    while (n < max && *s++) n++;
    return n;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) n--;
    while (n--) *d++ = '\0';
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest + strlen(dest);
    while (n-- && *src) *d++ = *src++;
    *d = '\0';
    return dest;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n-- && *a && *a == *b) { a++; b++; }
    if (!n) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char *strchr(const char *s, int c) {
    while (*s) { if (*s == (char)c) return (char *)s; s++; }
    return (c == '\0') ? (char *)s : NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) { if (*s == (char)c) last = s; s++; }
    return (char *)last;
}

char *strstr(const char *h, const char *n) {
    if (!*n) return (char *)h;
    size_t nl = strlen(n);
    for (; *h; h++) {
        if (*h == *n && memcmp(h, n, nl) == 0) return (char *)h;
    }
    return NULL;
}

static char *strtok_ptr = NULL;
char *strtok(char *str, const char *delim) {
    if (str) strtok_ptr = str;
    if (!strtok_ptr) return NULL;
    /* Skip leading delimiters */
    while (*strtok_ptr && strchr(delim, *strtok_ptr)) strtok_ptr++;
    if (!*strtok_ptr) { strtok_ptr = NULL; return NULL; }
    char *start = strtok_ptr;
    while (*strtok_ptr && !strchr(delim, *strtok_ptr)) strtok_ptr++;
    if (*strtok_ptr) *strtok_ptr++ = '\0';
    else strtok_ptr = NULL;
    return start;
}

long strtol(const char *str, char **endptr, int base) {
    while (*str == ' ') str++;
    int sign = 1;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') str++;
    if (base == 0) {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) { base = 16; str += 2; }
        else if (str[0] == '0') { base = 8; str++; }
        else base = 10;
    }
    long result = 0;
    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') digit = *str - '0';
        else if (*str >= 'a' && *str <= 'f') digit = *str - 'a' + 10;
        else if (*str >= 'A' && *str <= 'F') digit = *str - 'A' + 10;
        else break;
        if (digit >= base) break;
        result = result * base + digit;
        str++;
    }
    if (endptr) *endptr = (char *)str;
    return sign * result;
}

void itoa(int val, char *buf, int base) {
    char tmp[32];
    int i = 0, neg = 0;
    if (val < 0 && base == 10) { neg = 1; val = -val; }
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val) {
        int r = val % base;
        tmp[i++] = (char)(r < 10 ? '0' + r : 'a' + r - 10);
        val /= base;
    }
    if (neg) tmp[i++] = '-';
    for (int j = 0; j < i; j++) buf[j] = tmp[i - j - 1];
    buf[i] = '\0';
}

void utoa(unsigned int val, char *buf, int base) {
    char tmp[32];
    int i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val) {
        unsigned r = val % base;
        tmp[i++] = (char)(r < 10 ? '0' + r : 'a' + r - 10);
        val /= base;
    }
    for (int j = 0; j < i; j++) buf[j] = tmp[i - j - 1];
    buf[i] = '\0';
}
