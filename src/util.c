#include <stdlib.h>
#include <errno.h>
#include "util.h"

/******************************** Log ****************************************/
void l_error(const char *format, ...)
{
    fprintf(stderr, ANSI_FG_RED);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ANSI_RESET "\n");
}

void l_warn(const char *format, ...)
{
    FILE *fp = stderr;
    fprintf(fp, ANSI_FG_YELLOW);
    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fprintf(fp, ANSI_RESET "\n");
}

void l_log(const char *format, ...)
{
    FILE *fp = stdout;
    fprintf(fp, ANSI_FG_GREEN);
    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
    fprintf(fp, ANSI_RESET "\n");
}

/******************************* Memory **************************************/
#define ALLOC_CHECK(ptr)                              \
do {                                                  \
    if (!ptr) {                                       \
        l_error("%s: %s", __func__, strerror(errno)); \
        exit(EXIT_FAILURE);                           \
    }                                                 \
} while (0)

void *l_malloc(size_t size)
{
    void *ptr = malloc(size);
    ALLOC_CHECK(ptr);
    return ptr;
}

void *l_calloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    ALLOC_CHECK(ptr);
    return ptr;
}

void *l_realloc(void *ptr, size_t size)
{
    void *new = realloc(ptr, size);
    ALLOC_CHECK(new);
    return new;
}

/******************************* String **************************************/
int l_is_num(const char *str)
{
    if (!l_has_str(str))
        return 0;
    for (; '0' <= *str && *str <= '9'; str++)
        ;
    return *str == '\0';
}

int l_has_str(const char *str)
{
    return (str && *str);
}

char *l_strdup(const char *s)
{
    char *t = l_malloc(strlen(s)+1);
    strcpy(t, s);
    return t;
}

char *l_mprintf(const char *fmt, ...)
{
    va_list args, argscpy;
    va_start(args, fmt);
    va_copy(argscpy, args);

    size_t needed = vsnprintf(NULL, 0, fmt, argscpy);
    char *buf = l_malloc(needed+1);
    va_end(argscpy);
    vsprintf(buf, fmt, args);
    va_end(args);

    return buf;
}

char *l_strnchr(const char *s, char ch, size_t n)
{
    while (n--)
    {
        if (*s == ch)
            break;
        s++;
    }
    if (n == 0 && *s != ch)
        return NULL;
    return (char *)s;
}

// replace all `old` character to `new` character in `str`
void l_repchr(char *str, char old, char new)
{
    char *ptr;

    while (1)
    {
        ptr = strchr(str, old);
        if (ptr == NULL)
            break;
        str[(int)(ptr-str)] = new;
    }
}

void l_lowercase(char *str)
{
    while (*str)
    {
        if ('A' <= *str && *str <= 'Z')
            *str = *str + 'a'-'A';
        str++;
    }
}

/********************************* HASH **************************************/
l_hitem_t *l_hput(l_hitem_t *hashtbl, const char *key, const char *value)
{
    l_hitem_t *item = NULL;

    HASH_FIND_STR(hashtbl, key, item);
    if (!item) {
        item = l_malloc(sizeof(*item));
        item->key = (char *)key;

        // NOTE: `key` is not variable `key` but l_hitem_t field `key`
        HASH_ADD_STR(hashtbl, key, item);
    }
    item->value = (char *)value;

    return hashtbl;
}

char *l_hget(l_hitem_t *hashtbl, const char *key)
{
    l_hitem_t *item = NULL;
    HASH_FIND_STR(hashtbl, key, item);
    return item ? item->value : NULL;
}

void l_hfree(l_hitem_t *hashtbl, l_hitem_free_fn free_fn)
{
    l_hitem_t *item, *tmp;

    HASH_ITER(hh, hashtbl, item, tmp) {
        HASH_DEL(hashtbl, item);
        if (free_fn)
            free_fn(item);
        L_FREE(item);
    }
}