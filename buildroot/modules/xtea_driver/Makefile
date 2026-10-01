obj-m := xtea_driver.o
BUILDROOT_DIR := ../..
KDIR := $(BUILDROOT_DIR)/output/build/linux-custom
COMPILER := $(BUILDROOT_DIR)/output/host/bin/i686-buildroot-linux-uclibc-gcc

all:
	$(MAKE) -C $(KDIR) M=$$PWD
	$(MAKE) -C $(KDIR) M=$$PWD modules_install INSTALL_MOD_PATH=../../target
	$(COMPILER) -o $(BUILDROOT_DIR)/output/target/bin/test_xtea_driver test_xtea_driver.c

clean:
	rm -f *.o *.ko .*.cmd
	rm -f modules.order Module.symvers xtea_driver.mod.c
	rm -f test_xtea_driver

