CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = wfilter
SOURCE = wfilter.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: clean install
