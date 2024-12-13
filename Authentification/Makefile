# Compiler and flags
CC = gcc
CFLAGS = -Wall -O0 -g -I/usr/include/ffmpeg -I/usr/include -I/usr/include/libmongoc-1.0 -I/usr/include/libbson-1.0
LDFLAGS = -L/usr/lib64 -lmongoc-1.0 -lbson-1.0 -lcurl -lasound

# Directories
SRC_DIR = src
OBJ_DIR = build
INC_DIR = include

# Source and object files
INC = $(INC_DIR)/code.h $(INC_DIR)/wavaudio.h
SRC = $(SRC_DIR)/authdaemon.c $(SRC_DIR)/code.c $(SRC_DIR)/wavaudio.c
OBJ = $(OBJ_DIR)/authdaemon.o $(OBJ_DIR)/code.o $(OBJ_DIR)/wavaudio.o
EXE = authdaemon

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Default target: Build the executable
$(EXE): $(OBJ)
	$(CC) -O0 $(OBJ) -o $(EXE) $(LDFLAGS)

# Rule to compile .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean the build (remove object files and executables)
clean:
	rm -f $(OBJ) $(EXE)
