obj-m := khello.o
BUILDROOT_DIR := ../..
KDIR := $(BUILDROOT_DIR)/output/build/linux-custom

CC_UCLIBC := $(BUILDROOT_DIR)/output/host/bin/i686-buildroot-linux-uclibc-gcc
CC_GNU    := $(BUILDROOT_DIR)/output/host/bin/i686-buildroot-linux-gnu-gcc
CC_FALLBACK := $(BUILDROOT_DIR)/output/host/bin/i686-linux-gcc
COMPILER  ?= $(shell if [ -x $(CC_UCLIBC) ]; then echo $(CC_UCLIBC); elif [ -x $(CC_GNU) ]; then echo $(CC_GNU); elif [ -x $(CC_FALLBACK) ]; then echo $(CC_FALLBACK); else echo $(CC_UCLIBC); fi)

INSTALL_MOD_PATH ?= $(shell if [ -d $(BUILDROOT_DIR)/output/target ]; then echo $(BUILDROOT_DIR)/output/target; else echo ../../target; fi)

all:
	$(MAKE) -C $(KDIR) M=$$PWD
	$(MAKE) -C $(KDIR) M=$$PWD modules_install INSTALL_MOD_PATH=$(INSTALL_MOD_PATH)

clean:
	rm -f *.o *.ko .*.cmd
	rm -f modules.order
	rm -f Module.symvers
	rm -f khello.mod.c
