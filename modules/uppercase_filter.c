#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

bool filter(const char *word, void *arg)
{
    if (word == NULL)
        return false;
    
    (void)arg;
    
    for (const char *c = word; *c; c++)
    {
        if (isupper(*c))
            return true;
    }
    
    return false;
}
