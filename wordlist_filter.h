#ifndef WORDLIST_FILTER_H
#define WORDLIST_FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <dlfcn.h>
#include <dirent.h>
#include <limits.h>

#ifndef PATH_MAX
#ifdef _POSIX_PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#else
#define PATH_MAX 4096
#endif
#endif

#define WORDLIST_FILTER_VERSION 1
#define WORDLIST_FILTER_MAX_FILE_SIZE 1024 * 1024 * 1024
#define WORDLIST_FILTER_MAX_LINE_LENGTH 1024
#define MODULES_DIR "modules"

#define RETURN_NULL_IF(con) \
    if ((con))              \
    {                       \
        return NULL;        \
    }

#define RETURN_FALSE_IF(con) \
    if ((con))               \
    {                        \
        return false;        \
    }

typedef struct wordlist_t wordlist_t;
typedef struct filter_module_t filter_module_t;
typedef struct module_list_t module_list_t;
typedef bool (*filter_function_t)(const char *word, void *arg);

struct wordlist_t
{
    unsigned int version;
    unsigned int line_count;
    unsigned int size;
    char **lines;
    char *raw_data;
};

struct filter_module_t
{
    char *name;
    void *handle;
    filter_function_t filter_func;
    struct filter_module_t *next;
};

struct module_list_t
{
    filter_module_t *head;
    unsigned int count;
};

wordlist_t *wordlist_load(const char *file_path);
void wordlist_close(wordlist_t *wordlist);
void wordlist_print(wordlist_t *wordlist);
bool wordlist_apply_filters(wordlist_t *wordlist, module_list_t *modules, void *arg);

module_list_t *modules_load(void);
void modules_close(module_list_t *modules);
void modules_print(module_list_t *modules);
bool modules_add(module_list_t *modules, const char *name, void *handle, filter_function_t func);

bool wordlist_save_filtered(wordlist_t *wordlist, const char *output_path, module_list_t *modules, void *arg);

#endif
