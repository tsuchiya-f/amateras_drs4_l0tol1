# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -O1
INCLUDES := -Iinc

# Directories
SRC_DIR := src
BIN_DIR := bin

main: main.o vdif_util.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BIN_DIR)/main main.o vdif_util.o
main.o:    $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/main.c
vdif_util.o:    $(SRC_DIR)/vdif_util.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_DIR)/vdif_util.c

# Clean up the bin directory
clean:
	rm -rf $(BIN_DIR)