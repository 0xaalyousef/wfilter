#include "wordlist_filter.h"

wordlist_t *wordlist_load(const char *file_path)
{
    wordlist_t *wordlist = NULL;
    struct stat info;
    FILE *handler = NULL;
    unsigned int line_count = 0;
    char **lines = NULL;
    char *raw_data = NULL;
    unsigned int raw_size = 0;

    RETURN_NULL_IF(file_path == NULL);

    RETURN_NULL_IF(stat(file_path, &info) != 0);
    RETURN_NULL_IF((info.st_size == 0) || (info.st_size > WORDLIST_FILTER_MAX_FILE_SIZE));

    handler = fopen(file_path, "r");
    RETURN_NULL_IF(handler == NULL);

    wordlist = (wordlist_t *)malloc(sizeof(wordlist_t));
    if (wordlist == NULL)
    {
        fclose(handler);
        return NULL;
    }

    raw_data = (char *)malloc(info.st_size + 1);
    if (raw_data == NULL)
    {
        free(wordlist);
        fclose(handler);
        return NULL;
    }

    raw_size = fread(raw_data, 1, info.st_size, handler);
    raw_data[raw_size] = '\0';
    fclose(handler);

    char *ptr = raw_data;
    while (*ptr)
    {
        if (*ptr == '\n')
            line_count++;
        ptr++;
    }
    if (raw_data[raw_size - 1] != '\n')
        line_count++;

    lines = (char **)malloc(sizeof(char *) * line_count);
    if (lines == NULL)
    {
        free(raw_data);
        free(wordlist);
        return NULL;
    }

    ptr = raw_data;
    unsigned int line_idx = 0;
    char *line_start = raw_data;

    while (*ptr)
    {
        if (*ptr == '\n')
        {
            *ptr = '\0';
            lines[line_idx] = line_start;
            line_idx++;
            line_start = ptr + 1;
        }
        ptr++;
    }
    if (line_start < ptr)
    {
        lines[line_idx] = line_start;
    }

    wordlist->version = WORDLIST_FILTER_VERSION;
    wordlist->line_count = line_count;
    wordlist->size = raw_size;
    wordlist->lines = lines;
    wordlist->raw_data = raw_data;

    return wordlist;
}

void wordlist_close(wordlist_t *wordlist)
{
    if (wordlist)
    {
        if (wordlist->raw_data)
            free(wordlist->raw_data);
        if (wordlist->lines)
            free(wordlist->lines);
        
        wordlist->line_count = 0;
        wordlist->size = 0;
        wordlist->version = 0;
        
        free(wordlist);
    }
}

void wordlist_print(wordlist_t *wordlist)
{
    if (wordlist == NULL)
        return;

    printf("wordlist-version: %u\n", wordlist->version);
    printf("wordlist-size: %u bytes\n", wordlist->size);
    printf("wordlist-lines: %u\n", wordlist->line_count);
    printf("wordlist-address: %p\n", wordlist);
    printf("\n");

    unsigned int sample_count = (wordlist->line_count > 10) ? 10 : wordlist->line_count;
    for (unsigned int i = 0; i < sample_count; i++)
    {
        printf("[%u] %s\n", i, wordlist->lines[i]);
    }
    if (wordlist->line_count > 10)
    {
        printf("... (%u more lines)\n", wordlist->line_count - 10);
    }
}

bool wordlist_apply_filters(wordlist_t *wordlist, module_list_t *modules, void *arg)
{
    if (wordlist == NULL || modules == NULL)
        return false;

    printf("Applying filters to %u lines...\n", wordlist->line_count);
    
    unsigned int filtered_count = 0;
    for (unsigned int i = 0; i < wordlist->line_count; i++)
    {
        bool passes_all_filters = true;
        filter_module_t *current = modules->head;
        
        while (current != NULL && passes_all_filters)
        {
            if (current->filter_func != NULL)
            {
                passes_all_filters = current->filter_func(wordlist->lines[i], arg);
            }
            current = current->next;
        }
        
        if (passes_all_filters)
        {
            printf("%s\n", wordlist->lines[i]);
            filtered_count++;
        }
    }
    
    printf("\nFiltered %u lines out of %u total lines\n", filtered_count, wordlist->line_count);
    return true;
}

bool wordlist_save_filtered(wordlist_t *wordlist, const char *output_path, module_list_t *modules, void *arg)
{
    if (wordlist == NULL || modules == NULL || output_path == NULL)
        return false;

    FILE *output = fopen(output_path, "w");
    if (output == NULL)
        return false;

    unsigned int filtered_count = 0;
    for (unsigned int i = 0; i < wordlist->line_count; i++)
    {
        bool passes_all_filters = true;
        filter_module_t *current = modules->head;
        
        while (current != NULL && passes_all_filters)
        {
            if (current->filter_func != NULL)
            {
                passes_all_filters = current->filter_func(wordlist->lines[i], arg);
            }
            current = current->next;
        }
        
        if (passes_all_filters)
        {
            fprintf(output, "%s\n", wordlist->lines[i]);
            filtered_count++;
        }
    }
    
    fclose(output);
    printf("Saved %u filtered lines to %s\n", filtered_count, output_path);
    return true;
}

// Module management functions
module_list_t *modules_load(void)
{
    module_list_t *modules = (module_list_t *)malloc(sizeof(module_list_t));
    if (modules == NULL)
        return NULL;

    modules->head = NULL;
    modules->count = 0;

    DIR *dir = opendir(MODULES_DIR);
    if (dir == NULL)
    {
        printf("Warning: Could not open modules directory '%s'\n", MODULES_DIR);
        return modules;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        char *ext = strrchr(entry->d_name, '.');
        if (ext == NULL || strcmp(ext, ".so") != 0)
            continue;

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", MODULES_DIR, entry->d_name);

        void *handle = dlopen(full_path, RTLD_LAZY);
        if (handle == NULL)
        {
            printf("Warning: Could not load module '%s': %s\n", entry->d_name, dlerror());
            continue;
        }

        filter_function_t filter_func = (filter_function_t)dlsym(handle, "filter");
        if (filter_func == NULL)
        {
            printf("Warning: Could not find 'filter' function in module '%s': %s\n", entry->d_name, dlerror());
            dlclose(handle);
            continue;
        }

        if (modules_add(modules, entry->d_name, handle, filter_func))
        {
            printf("Loaded module: %s\n", entry->d_name);
        }
        else
        {
            dlclose(handle);
        }
    }

    closedir(dir);
    return modules;
}

void modules_close(module_list_t *modules)
{
    if (modules == NULL)
        return;

    filter_module_t *current = modules->head;
    while (current != NULL)
    {
        filter_module_t *next = current->next;
        
        if (current->handle != NULL)
            dlclose(current->handle);
        if (current->name != NULL)
            free(current->name);
        
        free(current);
        current = next;
    }

    free(modules);
}

void modules_print(module_list_t *modules)
{
    if (modules == NULL)
        return;

    printf("Loaded modules (%u):\n", modules->count);
    filter_module_t *current = modules->head;
    unsigned int index = 0;
    
    while (current != NULL)
    {
        printf("[%u] %s -> %p\n", index, current->name, current->filter_func);
        current = current->next;
        index++;
    }
}

bool modules_add(module_list_t *modules, const char *name, void *handle, filter_function_t func)
{
    if (modules == NULL || name == NULL || func == NULL)
        return false;

    filter_module_t *new_module = (filter_module_t *)malloc(sizeof(filter_module_t));
    if (new_module == NULL)
        return false;

    new_module->name = strdup(name);
    if (new_module->name == NULL)
    {
        free(new_module);
        return false;
    }

    new_module->handle = handle;
    new_module->filter_func = func;
    new_module->next = modules->head;
    modules->head = new_module;
    modules->count++;

    return true;
}
