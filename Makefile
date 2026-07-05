TARGET = amt_drs4_l0_l1

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

all: $(TARGET)

amt_drs4_l0_l1: amt_drs4_l0_l1.o vdif_util.o
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -o $(BIN_DIR)/amt_drs4_l0_l1 amt_drs4_l0_l1.o vdif_util.o

.c.o: $(SRC_DIR)/*.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $(SRC_DIR)/*.c

# Clean up the bin directory
clean:
	rm -rf $(BIN_DIR)/*