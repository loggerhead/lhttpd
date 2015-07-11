#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
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

// rewrite this function because I want it never failed.
char *l_strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *copy = l_malloc(len);
    memcpy(copy, str, len);
    return copy;
}

/********************************* Hash **************************************/
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

/********************************* File **************************************/
l_bool_t l_match_file_suffix(const char *filename, const char *suffix)
{
    const char *dot = strrchr(filename, '.');
    return dot && !strcmp(dot + 1, suffix);
}

l_bool_t l_is_file_exist(const char *path)
{
    struct stat s;
    if (stat(path, &s) == 0)
        return TRUE;
    return errno != ENOENT;
}

size_t l_get_filesize(FILE *fp)
{
    struct stat s;
    fstat(fileno(fp), &s);
    if (s.st_size == -1)
        l_error("%s: failed", __func__);
    return s.st_size;
}

// NOTE: return value need free
const char *l_pathcat(const char *dir, const char *filename)
{
    size_t len = strlen(dir);
    char last = *(dir + len - 1);
    char first = *filename;

    if (last == '/') {
        if (first == '/')
            return (const char *) l_mprintf("%s%s", dir, filename+1);
        else
            return (const char *) l_mprintf("%s%s", dir, filename);
    }

    if (*dir) {
        if (first == '/')
            return (const char *) l_mprintf("%s%s", dir, filename);
        else
            return (const char *) l_mprintf("%s/%s", dir, filename);
    } else {
        return l_strdup(filename);
    }
}

// NOTE: return value need free by `L_FREE`
const char *l_url2filename(const char *url)
{
    size_t url_len = strlen(url);
    const char *begin = url;
    const char *end = strchr(url, '?');
    const char *tmp = url;

    // end before first '?'
    end = (end == NULL) ? (url + url_len - 1) : (end - 1);
    // match "/"
    if (begin == end && *begin == '/' && *end == '/')
        return strndup("index.html", 5);
    // skip all end with '/'
    while (url < end && *end == '/')
        end--;
    // begin after last '/'
    while (tmp < end)
    {
        tmp = strchr(begin, '/');
        if (tmp == NULL || tmp > end)
            break;
        begin = tmp + 1;
    }

    if (begin > end)
        return NULL;
    return strndup(begin, end - begin + 1);
}

// get the directory component of a pathname
// NOTE: return value need free by `L_FREE`
const char *l_get_dirname(const char *filepath)
{
    const char *slash = strrchr(filepath, '/');
    return slash ? strndup(filepath, slash - filepath) : l_strdup("");
}

// get the final component of a pathname
const char *l_get_basename(const char *filepath)
{
    const char *slash = strrchr(filepath, '/');
    return slash ? slash + 1 : filepath;
}

// get the suffix of filename
const char *l_get_suffix(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    return (!dot || dot == filename) ? "" : dot+1;
}

/* read file data into memory, return `buf.len` == 0 when failed
 * NOTE: `buf.data` need free by `L_FREE`
 */
l_buf_t l_read_file(const char *filepath)
{
    l_buf_t tmp = { NULL, 0 };

    FILE *fp = fopen(filepath, "rb");
    // if can't read
    if (!fp)
        goto RETURN;

    size_t len = l_get_filesize(fp);
    // if is empty file
    if (!len)
        goto RETURN;

    char *data = l_malloc(len);
    // if can't read into memory
    if (fread(data, 1, len, fp) != len)
        goto RETURN;

    tmp.data = data;
    tmp.len = len;

RETURN:
    fclose(fp);
    return tmp;
}

void l_mkdirs(const char *dir)
{
    char *tmp = l_strdup(dir);
    size_t len = strlen(tmp);

    if (tmp[len-1] == '/')
        tmp[len-1] = 0;

    for(char *p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }

    mkdir(tmp, S_IRWXU);

    L_FREE(tmp);
}