BUILDROOT_DIR ?= ..
TARGET_DIR ?= $(BUILDROOT_DIR)/output/target
HOST_DIR ?= $(BUILDROOT_DIR)/output/host

CC_UCLIBC := $(HOST_DIR)/bin/i686-buildroot-linux-uclibc-gcc
CC_GNU    := $(HOST_DIR)/bin/i686-buildroot-linux-gnu-gcc
CC_FALLBACK := $(HOST_DIR)/bin/i686-linux-gcc

CC ?= $(shell if [ -x $(CC_UCLIBC) ]; then echo $(CC_UCLIBC); elif [ -x $(CC_GNU) ]; then echo $(CC_GNU); elif [ -x $(CC_FALLBACK) ]; then echo $(CC_FALLBACK); else echo i686-buildroot-linux-uclibc-gcc; fi)

CFLAGS ?= -Wall -O2 -pthread

all: setpriority deadline thread_runner

setpriority: setpriority.c
	$(CC) $(CFLAGS) -o setpriority setpriority.c -lpthread

deadline: deadline.c
	$(CC) $(CFLAGS) -o deadline deadline.c -lpthread

thread_runner: thread_runner.c
	$(CC) $(CFLAGS) -o thread_runner thread_runner.c -lpthread

install: all
	mkdir -p $(TARGET_DIR)/usr/bin
	mkdir -p $(TARGET_DIR)/bin
	cp setpriority $(TARGET_DIR)/usr/bin/setpriority
	cp deadline $(TARGET_DIR)/usr/bin/deadline
	cp thread_runner $(TARGET_DIR)/usr/bin/thread_runner
	ln -sf /usr/bin/setpriority $(TARGET_DIR)/bin/setpriority 2>/dev/null || cp setpriority $(TARGET_DIR)/bin/setpriority
	ln -sf /usr/bin/deadline $(TARGET_DIR)/bin/deadline 2>/dev/null || cp deadline $(TARGET_DIR)/bin/deadline
	ln -sf /usr/bin/thread_runner $(TARGET_DIR)/bin/thread_runner 2>/dev/null || cp thread_runner $(TARGET_DIR)/bin/thread_runner

clean:
	rm -f setpriority deadline thread_runner *.o

.PHONY: all install clean
