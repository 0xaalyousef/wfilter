# Wordlist Filter Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC -g
LDFLAGS = -ldl

# Directories
MODULES_DIR = modules
SRC_DIR = .
OBJ_DIR = obj

# Source files
CORE_SOURCES = wordlist_filter.c
MAIN_SOURCE = wfilter_main.c
MODULE_SOURCES = $(wildcard $(MODULES_DIR)/*.c)

# Object files
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
MAIN_OBJECT = $(MAIN_SOURCE:.c=.o)
MODULE_OBJECTS = $(MODULE_SOURCES:.c=.o)

# Shared libraries
MODULE_LIBS = $(MODULE_SOURCES:.c=.so)

# Target executable
TARGET = wfilter

# Default target
all: $(TARGET) modules

# Build main executable
$(TARGET): $(CORE_OBJECTS) $(MAIN_OBJECT)
	$(CC) $(CORE_OBJECTS) $(MAIN_OBJECT) -o $(TARGET) $(LDFLAGS)

# Build modules as shared libraries
modules: $(MODULE_LIBS)

# Pattern rule for shared libraries
$(MODULES_DIR)/%.so: $(MODULES_DIR)/%.c
	$(CC) $(CFLAGS) -shared -o $@ $<

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(CORE_OBJECTS) $(MAIN_OBJECT) $(MODULE_OBJECTS) $(MODULE_LIBS) $(TARGET)

# Install (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo mkdir -p /usr/local/lib/wfilter/modules
	sudo cp $(MODULE_LIBS) /usr/local/lib/wfilter/modules/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo rm -rf /usr/local/lib/wfilter/

# Test with a sample wordlist
test: $(TARGET) modules
	@echo "Creating test wordlist..."
	@echo -e "password\nPassword123\nP@ssw0rd\nadmin\nAdmin123\nroot\nRoot123!" > test_wordlist.txt
	@echo "Running filter with minimum length 8, uppercase, and special chars..."
	./$(TARGET) -l 8 -u -s -v test_wordlist.txt
	@echo "Running filter with minimum length 6 and digits..."
	./$(TARGET) -l 6 -d -v test_wordlist.txt
	@echo "Saving filtered results..."
	./$(TARGET) -l 8 -u -s -o filtered_test.txt test_wordlist.txt
	@echo "Filtered results saved to filtered_test.txt"
	@rm -f test_wordlist.txt

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build main executable and modules (default)"
	@echo "  modules  - Build only the filter modules"
	@echo "  clean    - Remove all build artifacts"
	@echo "  install  - Install to /usr/local/bin and /usr/local/lib/wfilter/modules"
	@echo "  uninstall- Remove installed files"
	@echo "  test     - Run test with sample wordlist"
	@echo "  help     - Show this help"

.PHONY: all modules clean install uninstall test help
