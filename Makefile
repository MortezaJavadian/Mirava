# Compiler and compiler flags
CC = gcc
# Add -I. to include the current directory for header files
CFLAGS = -Wall -Wextra -std=c99 -g -I.

# Linker flags for external libraries
LDFLAGS = -lavformat -lavutil -ljansson -lm

# The target executable name
TARGET = mirava

# List of object files
OBJS = main.o actions.o cli.o data_manager.o file_utils.o video_list.o

# Default rule: build the target
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile a .c file into a .o file
%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up the build artifacts
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
