#ifndef L_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"

#if DEBUG
# define TRACE(...)   l_log("--- %s ---", __func__)
#else
# define TRACE(...)
#endif /* DEBUG */

#define ANSI_RESET          "\x1b[0m"
#define ANSI_BOLD_ON        "\x1b[1m"
#define ANSI_INVERSE_ON     "\x1b[7m"
#define ANSI_BOLD_OFF       "\x1b[22m"
#define ANSI_FG_BLACK       "\x1b[30m"
#define ANSI_FG_RED         "\x1b[31m"
#define ANSI_FG_GREEN       "\x1b[32m"
#define ANSI_FG_YELLOW      "\x1b[33m"
#define ANSI_FG_BLUE        "\x1b[34m"
#define ANSI_FG_MAGENTA     "\x1b[35m"
#define ANSI_FG_CYAN        "\x1b[36m"
#define ANSI_FG_WHITE       "\x1b[37m"
#define ANSI_BG_RED         "\x1b[41m"
#define ANSI_BG_GREEN       "\x1b[42m"
#define ANSI_BG_YELLOW      "\x1b[43m"
#define ANSI_BG_BLUE        "\x1b[44m"
#define ANSI_BG_MAGENTA     "\x1b[45m"
#define ANSI_BG_CYAN        "\x1b[46m"
#define ANSI_BG_WHITE       "\x1b[47m"

void l_error(const char *format, ...);
void l_warn(const char *format, ...);
void l_log(const char *format, ...);

#define L_FREE(memory) free((void *) (memory))
void *l_malloc(size_t size);
void *l_calloc(size_t count, size_t size);
void *l_realloc(void *ptr, size_t size);

char *l_mprintf(const char *fmt, ...);
char *l_strdup(const char *s);
char *l_strnchr(const char *s, char ch, size_t n);
void l_repchr(char *str, char old, char new);
void l_lowercase(char *str);

const char *l_get_now_time();

typedef void (*hitem_free_fn) (hitem_t *item);

hitem_t *l_hput(hitem_t *hashtbl, const char *key, const char *value);
char *l_hget(hitem_t *hashtbl, const char *key);
// leave `free_fn` NULL to tell `l_hfree` not free fields
void l_hfree(hitem_t *hashtbl, hitem_free_fn free_fn);

#define L_HITER(hashtbl, item) for(item=hashtbl; item; item=item->hh.next)

#define L_UTIL_H
#endif