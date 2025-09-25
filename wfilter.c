#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_LINE_LEN 1024
#define SPECIAL_CHARS "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

typedef struct {
    int min_length;
    int require_uppercase;
    int require_number;
    int require_special;
    char *output_file;
} filter_options_t;

void print_banner() {
    printf(COLOR_CYAN COLOR_BOLD);
    printf("================================================================\n");
    printf("                                                                \n");
    printf("    " COLOR_YELLOW "██╗    ██╗███████╗██╗██╗  ████████╗███████╗██████╗ " COLOR_CYAN "\n");
    printf("    " COLOR_YELLOW "██║    ██║██╔════╝██║██║  ╚══██╔══╝██╔════╝██╔══██╗" COLOR_CYAN "\n");
    printf("    " COLOR_YELLOW "██║ █╗ ██║█████╗  ██║██║     ██║   █████╗  ██████╔╝" COLOR_CYAN "\n");
    printf("    " COLOR_YELLOW "██║███╗██║██╔══╝  ██║██║     ██║   ██╔══╝  ██╔══██╗" COLOR_CYAN "\n");
    printf("    " COLOR_YELLOW "╚███╔███╔╝██║     ██║███████╗██║   ███████╗██║  ██║" COLOR_CYAN "\n");
    printf("    " COLOR_YELLOW " ╚══╝╚══╝ ╚═╝     ╚═╝╚══════╝╚═╝   ╚══════╝╚═╝  ╚═╝" COLOR_CYAN "\n");
    printf("                                                                \n");
    printf("  " COLOR_GREEN "Wordlist Filtering Tool" COLOR_CYAN "  \n");
    printf("                                                                \n");
    printf("================================================================\n");
    printf(COLOR_RESET);
}

void print_usage(const char *prog_name) {
    printf(COLOR_BOLD COLOR_CYAN "Usage: " COLOR_RESET);
    printf(COLOR_WHITE "wfilter " COLOR_RESET);
    printf(COLOR_YELLOW "<INPUT_WORDLIST>" COLOR_RESET);
    printf(" [" COLOR_GREEN "--min" COLOR_RESET " " COLOR_BLUE "<N>" COLOR_RESET "] [" COLOR_GREEN "--uppercase" COLOR_RESET "|" COLOR_GREEN "-u" COLOR_RESET "] [" COLOR_GREEN "--number" COLOR_RESET "|" COLOR_GREEN "-n" COLOR_RESET "] [" COLOR_GREEN "--special" COLOR_RESET "|" COLOR_GREEN "-s" COLOR_RESET "] [" COLOR_GREEN "-o" COLOR_RESET " " COLOR_BLUE "<OUTPUT_WORDLIST>" COLOR_RESET "]\n");
    
    printf(COLOR_BOLD COLOR_CYAN "\nOptions:\n" COLOR_RESET);
    printf(COLOR_YELLOW "  <INPUT_WORDLIST>" COLOR_RESET " (required): path to the source wordlist\n");
    printf(COLOR_GREEN "  --min <N>" COLOR_RESET ": minimum length (default: 1)\n");
    printf(COLOR_GREEN "  --uppercase" COLOR_RESET " or " COLOR_GREEN "-u" COLOR_RESET ": require at least one uppercase A-Z\n");
    printf(COLOR_GREEN "  --number" COLOR_RESET " or " COLOR_GREEN "-n" COLOR_RESET ": require at least one digit 0-9\n");
    printf(COLOR_GREEN "  --special" COLOR_RESET " or " COLOR_GREEN "-s" COLOR_RESET ": require at least one special character\n");
    printf(COLOR_GREEN "  -o <OUTPUT_WORDLIST>" COLOR_RESET ": path to write results (default: stdout)\n");
    printf(COLOR_GREEN "  --help" COLOR_RESET " or " COLOR_GREEN "-h" COLOR_RESET ": show this help message\n");
    
    printf(COLOR_BOLD COLOR_CYAN "\nExamples:\n" COLOR_RESET);
    printf(COLOR_WHITE "  wfilter " COLOR_RESET);
    printf(COLOR_YELLOW "words.txt " COLOR_RESET);
    printf(COLOR_GREEN "--min 8" COLOR_RESET "\n");
    printf(COLOR_WHITE "  wfilter " COLOR_RESET);
    printf(COLOR_YELLOW "passwords.txt " COLOR_RESET);
    printf(COLOR_GREEN "-u -n -s -o strong.txt" COLOR_RESET "\n");
    printf(COLOR_WHITE "  wfilter " COLOR_RESET);
    printf(COLOR_YELLOW "candidates.txt " COLOR_RESET);
    printf(COLOR_GREEN "--min 12 --uppercase --number --special" COLOR_RESET "\n");
}

int parse_arguments(int argc, char *argv[], filter_options_t *options, char **input_file) {
    int i;
    
    options->min_length = 1;
    options->require_uppercase = 0;
    options->require_number = 0;
    options->require_special = 0;
    options->output_file = NULL;
    *input_file = NULL;
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            return 2;
        }
    }
    
    if (argc < 2) {
        fprintf(stderr, COLOR_RED "Error: Input wordlist is required\n" COLOR_RESET);
        return 1;
    }
    
    *input_file = argv[1];
    
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--min") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, COLOR_RED "Error: --min requires a value\n" COLOR_RESET);
                return 1;
            }
            options->min_length = atoi(argv[++i]);
            if (options->min_length < 0) {
                fprintf(stderr, COLOR_RED "Error: minimum length must be non-negative\n" COLOR_RESET);
                return 1;
            }
        } else if (strcmp(argv[i], "--uppercase") == 0 || strcmp(argv[i], "-u") == 0) {
            options->require_uppercase = 1;
        } else if (strcmp(argv[i], "--number") == 0 || strcmp(argv[i], "-n") == 0) {
            options->require_number = 1;
        } else if (strcmp(argv[i], "--special") == 0 || strcmp(argv[i], "-s") == 0) {
            options->require_special = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, COLOR_RED "Error: -o requires a value\n" COLOR_RESET);
                return 1;
            }
            options->output_file = argv[++i];
        } else {
            fprintf(stderr, COLOR_RED "Error: unknown option '%s'\n" COLOR_RESET, argv[i]);
            return 1;
        }
    }
    
    return 0;
}

int has_uppercase(const char *str) {
    while (*str) {
        if (isupper(*str)) {
            return 1;
        }
        str++;
    }
    return 0;
}

int has_number(const char *str) {
    while (*str) {
        if (isdigit(*str)) {
            return 1;
        }
        str++;
    }
    return 0;
}

int has_special(const char *str) {
    while (*str) {
        if (strchr(SPECIAL_CHARS, *str)) {
            return 1;
        }
        str++;
    }
    return 0;
}

int meets_criteria(const char *line, const filter_options_t *options) {
    size_t len = strlen(line);
    
    // Check minimum length
    if (len < (size_t)options->min_length) {
        return 0;
    }
    
    // Check uppercase requirement
    if (options->require_uppercase && !has_uppercase(line)) {
        return 0;
    }
    
    // Check number requirement
    if (options->require_number && !has_number(line)) {
        return 0;
    }
    
    // Check special character requirement
    if (options->require_special && !has_special(line)) {
        return 0;
    }
    
    return 1;
}

int process_wordlist(const char *input_file, const filter_options_t *options) {
    FILE *input_fp = NULL;
    FILE *output_fp = stdout;
    char line[MAX_LINE_LEN];
    int result = 0;
    
    input_fp = fopen(input_file, "r");
    if (!input_fp) {
        fprintf(stderr, COLOR_RED "Error: cannot open input file '%s'\n" COLOR_RESET, input_file);
        return 1;
    }
    
    if (options->output_file) {
        output_fp = fopen(options->output_file, "w");
        if (!output_fp) {
            fprintf(stderr, COLOR_RED "Error: cannot open output file '%s'\n" COLOR_RESET, options->output_file);
            fclose(input_fp);
            return 2;
        }
    }
    
    while (fgets(line, sizeof(line), input_fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        if (len == 0) {
            continue;
        }
        
        if (meets_criteria(line, options)) {
            fprintf(output_fp, "%s\n", line);
        }
    }
    
    fclose(input_fp);
    if (options->output_file) {
        fclose(output_fp);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    filter_options_t options;
    char *input_file;
    int parse_result;
    int process_result;
    
    parse_result = parse_arguments(argc, argv, &options, &input_file);
    if (parse_result == 2) {
        print_usage(argv[0]);
        return 0;
    } else if (parse_result != 0) {
        print_usage(argv[0]);
        return parse_result;
    }
    
    print_banner();
    
    process_result = process_wordlist(input_file, &options);
    
    if (process_result == 0 && options.output_file) {
        printf(COLOR_GREEN COLOR_BOLD "✓ " COLOR_RESET);
        printf(COLOR_GREEN "Wordlist filtered successfully! Output written to: " COLOR_YELLOW "%s\n" COLOR_RESET, options.output_file);
    }
    
    return process_result;
}
