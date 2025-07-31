# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# Linker flags for FFmpeg libraries
LDFLAGS = -lavformat -lavutil

# The target executable name
TARGET = mirava

# Default rule
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(LDFLAGS)

# Rule to clean up the build artifacts
clean:
	rm -f $(TARGET)

.PHONY: all clean
