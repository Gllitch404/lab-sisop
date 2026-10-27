BUILDROOT_DIR ?= ..
TARGET_DIR ?= $(BUILDROOT_DIR)/output/target
HOST_DIR ?= $(BUILDROOT_DIR)/output/host

CC_UCLIBC := $(HOST_DIR)/bin/i686-buildroot-linux-uclibc-gcc
CC_GNU    := $(HOST_DIR)/bin/i686-buildroot-linux-gnu-gcc
CC_FALLBACK := $(HOST_DIR)/bin/i686-linux-gcc

CC ?= $(shell if [ -x $(CC_UCLIBC) ]; then echo $(CC_UCLIBC); elif [ -x $(CC_GNU) ]; then echo $(CC_GNU); elif [ -x $(CC_FALLBACK) ]; then echo $(CC_FALLBACK); else echo i686-buildroot-linux-uclibc-gcc; fi)

CFLAGS ?= -Wall -O2

all: disk_test

disk_test: disk_test.c
	$(CC) $(CFLAGS) -o disk_test disk_test.c

install: disk_test
	mkdir -p $(TARGET_DIR)/usr/bin
	cp disk_test $(TARGET_DIR)/usr/bin/disk_test
	mkdir -p $(TARGET_DIR)/bin
	cp disk_test $(TARGET_DIR)/bin/disk_test

clean:
	rm -f disk_test *.o

.PHONY: all install clean
