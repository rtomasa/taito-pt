ifeq ($(KERNELRELEASE),)

KVERSION ?= $(shell uname -r)
BUILD_DIR ?= /lib/modules/${KVERSION}/build

PWD := $(shell pwd)

modules:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

install:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules_install

uninstall:
	rm -f /lib/modules/${KVERSION}/kernel/drivers/usb/taito_driver.ko.xz
	depmod -a

clean:
	rm -rf *~ *.o .*.cmd *.mod *.mod.c *.ko *.ko.unsigned .depend \
    	.tmp_versions modules.order Module.symvers Module.markers

.PHONY: modules install uninstall clean

else

obj-m := taito_pt.o

endif