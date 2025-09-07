# wfilter

A modular C tool for filtering wordlists based on various criteria. Built with a plugin architecture similar to the mojibake project, allowing for easy extension with custom filter modules.

## Features

- **Modular Design**: Uses shared library (.so) modules for filtering functions
- **Memory Efficient**: Loads entire wordlist into memory for fast processing
- **Extensible**: Easy to add new filter modules
- **Command Line Interface**: Simple and intuitive command line options
- **Cross-Platform**: Works on Linux, macOS, and other POSIX systems

## Filter Modules

The tool comes with several built-in filter modules:

- **Length Filter** (`length_filter.so`): Filters words by minimum length
- **Uppercase Filter** (`uppercase_filter.so`): Requires words to contain uppercase letters
- **Special Character Filter** (`special_char_filter.so`): Requires words to contain special characters
- **Digit Filter** (`digit_filter.so`): Requires words to contain digits

## Installation

### Prerequisites

- GCC compiler
- Make utility
- POSIX-compliant system (Linux, macOS, etc.)

### Building

```bash
git clone https://github.com/0xaalyousef/wfilter.git
cd wfilter

make all

make modules

make clean
```

## Usage

```bash
wfilter [OPTIONS] <wordlist_file>
```

### Options

- `-l <length>`: Minimum word length (default: 8)
- `-u`: Require uppercase letters
- `-s`: Require special characters
- `-d`: Require digits
- `-o <file>`: Output filtered wordlist to file
- `-v`: Verbose output
- `-h`: Show help

### Examples

```bash
# Show help
wfilter -h

# Filter words with minimum length 8 (default)
wfilter rockyou.txt

# Filter words with minimum length 10, uppercase, and special characters
wfilter -l 10 -u -s rockyou.txt

# Filter words with minimum length 8 and digits, save to file
wfilter -l 8 -d -o filtered.txt passwords.txt

# Verbose output showing all details
wfilter -l 6 -u -s -d -v wordlist.txt
```

## Creating Custom Modules

To create a new filter module:

1. Create a new `.c` file in the `modules/` directory
2. Implement a `filter` function with the required signature
3. Build with: `make modules`

Example custom module (`modules/custom_filter.c`):
```c
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

bool filter(const char *word, void *arg)
{
    if (word == NULL)
        return false;
    
    // Your filtering logic here
    return strlen(word) > 5; // Example: words longer than 5 chars
}
```

## Architecture

The tool follows a modular architecture:

1. **Core Library** (`wordlist_filter.c/h`): Handles wordlist loading and module management
2. **Module System**: Dynamic loading of .so files from the `modules/` directory
3. **Linked List**: All loaded modules are stored in a linked list for easy iteration
4. **Filter Functions**: Each module exports a `filter` function with signature:
   ```c
   bool filter(const char *word, void *arg);
   ```

## File Structure

```
.
├── wordlist_filter.h          # Main header file
├── wordlist_filter.c          # Core implementation
├── wfilter_main.c             # Main program
├── Makefile                   # Build configuration
├── modules/                   # Filter modules directory
│   ├── length_filter.c        # Length filter module
│   ├── uppercase_filter.c     # Uppercase filter module
│   ├── special_char_filter.c  # Special character filter module
│   └── digit_filter.c         # Digit filter module
└── README.md                  # This file
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the mojibake project architecture
- Built with standard C libraries for maximum compatibility
