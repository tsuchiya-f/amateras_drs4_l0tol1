# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -O3
INCLUDES := -Iinc 

# find architecture
ARCH := $(shell uname -m)

ifeq ($(ARCH), arm64)
    CFLAGS  += -arch arm64 -I/opt/homebrew/include
    LDFLAGS += -arch arm64 -L/opt/homebrew/lib -lcfitsio -lm 
else
    CFLAGS  += -arch x86_64 -I/usr/local/include
    LDFLAGS += -arch x86_64 -L/usr/local/lib -lcfitsio -lm 
endif

# Directories
SRC_DIR := src
BIN_DIR := bin

amt_drs4_l0_l1: amt_drs4_l0_l1.o vdif_util.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $(BIN_DIR)/amt_drs4_l0_l1 amt_drs4_l0_l1.o vdif_util.o
amt_drs4_l0_l1.o:    $(SRC_DIR)/amt_drs4_l0_l1.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c $(SRC_DIR)/amt_drs4_l0_l1.c
vdif_util.o:    $(SRC_DIR)/vdif_util.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c $(SRC_DIR)/vdif_util.c

# Clean up the bin directory
clean:
	rm -rf $(BIN_DIR)/*