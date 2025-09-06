#include "wordlist_filter.h"
#include <unistd.h>

void print_usage(const char *program_name)
{
    printf("Usage: %s [OPTIONS] <wordlist_file>\n", program_name);
    printf("\nOptions:\n");
    printf("  -l <length>     Minimum word length (default: 8)\n");
    printf("  -u              Require uppercase letters\n");
    printf("  -s              Require special characters\n");
    printf("  -d              Require digits\n");
    printf("  -o <file>       Output filtered wordlist to file\n");
    printf("  -v              Verbose output\n");
    printf("  -h              Show this help\n");
    printf("\nExamples:\n");
    printf("  %s -l 10 -u -s rockyou.txt\n", program_name);
    printf("  %s -l 8 -d -o filtered.txt passwords.txt\n", program_name);
}

int main(int argc, char *argv[])
{
    int opt;
    int min_length = 8;
    bool require_uppercase = false;
    bool require_special = false;
    bool require_digit = false;
    bool verbose = false;
    char *output_file = NULL;
    char *input_file = NULL;

    while ((opt = getopt(argc, argv, "l:usdo:vh")) != -1)
    {
        switch (opt)
        {
            case 'l':
                min_length = atoi(optarg);
                if (min_length < 0)
                {
                    fprintf(stderr, "Error: Minimum length must be non-negative\n");
                    return 1;
                }
                break;
            case 'u':
                require_uppercase = true;
                break;
            case 's':
                require_special = true;
                break;
            case 'd':
                require_digit = true;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    input_file = argv[optind];

    if (verbose)
    {
        printf("Wordlist Filter v%d\n", WORDLIST_FILTER_VERSION);
        printf("Input file: %s\n", input_file);
        printf("Minimum length: %d\n", min_length);
        printf("Require uppercase: %s\n", require_uppercase ? "yes" : "no");
        printf("Require special chars: %s\n", require_special ? "yes" : "no");
        printf("Require digits: %s\n", require_digit ? "yes" : "no");
        if (output_file)
            printf("Output file: %s\n", output_file);
        printf("\n");
    }

    wordlist_t *wordlist = wordlist_load(input_file);
    if (wordlist == NULL)
    {
        fprintf(stderr, "Error: Could not load wordlist from '%s'\n", input_file);
        return 1;
    }

    if (verbose)
    {
        wordlist_print(wordlist);
        printf("\n");
    }

    module_list_t *modules = modules_load();
    if (modules == NULL)
    {
        fprintf(stderr, "Error: Could not load modules\n");
        wordlist_close(wordlist);
        return 1;
    }

    if (verbose)
    {
        modules_print(modules);
        printf("\n");
    }

    printf("Applying filters...\n");
    printf("========================================\n");
    
    int length_arg = min_length;
    
    if (min_length > 0)
    {
        filter_module_t *current = modules->head;
        while (current != NULL)
        {
            if (strstr(current->name, "length") != NULL)
            {
                printf("Applying length filter (min: %d)...\n", min_length);
                break;
            }
            current = current->next;
        }
    }

    filter_module_t *current = modules->head;
    while (current != NULL)
    {
        if (require_uppercase && strstr(current->name, "uppercase") != NULL)
        {
            printf("Applying uppercase filter...\n");
        }
        else if (require_special && strstr(current->name, "special") != NULL)
        {
            printf("Applying special character filter...\n");
        }
        else if (require_digit && strstr(current->name, "digit") != NULL)
        {
            printf("Applying digit filter...\n");
        }
        current = current->next;
    }

    printf("========================================\n");

    if (!wordlist_apply_filters(wordlist, modules, &length_arg))
    {
        fprintf(stderr, "Error: Failed to apply filters\n");
        wordlist_close(wordlist);
        modules_close(modules);
        return 1;
    }

    if (output_file != NULL)
    {
        if (!wordlist_save_filtered(wordlist, output_file, modules, &length_arg))
        {
            fprintf(stderr, "Error: Failed to save filtered wordlist\n");
            wordlist_close(wordlist);
            modules_close(modules);
            return 1;
        }
    }

    wordlist_close(wordlist);
    modules_close(modules);

    return 0;
}
