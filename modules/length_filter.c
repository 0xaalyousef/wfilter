#include <string.h>
#include <stdbool.h>
#include <stddef.h>

bool filter(const char *word, void *arg)
{
    if (word == NULL)
        return false;
    
    int min_length = *(int *)arg;
    return (int)strlen(word) >= min_length;
}
