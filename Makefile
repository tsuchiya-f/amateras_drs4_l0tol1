# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -O3
INCLUDES := -Iinc 

# find architecture
ARCH := $(shell uname -m)

ifeq ($(ARCH), arm64)
	INCLUDES += -I/opt/homebrew/include
    CFLAGS  += -arch arm64
    LDFLAGS += -L/opt/homebrew/lib -L/usr/local/opt/libomp/lib -lcfitsio -lm -lomp
else
	INCLUDES += -I/usr/local/include
    CFLAGS  += 
    LDFLAGS += -L/usr/local/lib -lcfitsio -lm -lomp
endif

# Directories
SRC_DIR := src
BIN_DIR := bin

amt_drs4_l0_l1: amt_drs4_l0_l1.o vdif_util.o
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $(BIN_DIR)/amt_drs4_l0_l1 amt_drs4_l0_l1.o vdif_util.o
amt_drs4_l0_l1.o:    $(SRC_DIR)/amt_drs4_l0_l1.c
	$(CC) $(INCLUDES) -c $(SRC_DIR)/amt_drs4_l0_l1.c
vdif_util.o:    $(SRC_DIR)/vdif_util.c
	$(CC) $(INCLUDES) -c $(SRC_DIR)/vdif_util.c

# Clean up the bin directory
clean:
	rm -rf $(BIN_DIR)/*