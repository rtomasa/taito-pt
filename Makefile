KVERSION    ?= $(shell uname -r)
BUILD_DIR   ?= /lib/modules/$(KVERSION)/build
PWD         := $(shell pwd)
MODULE_NAME := taito_pt
SRC_FILE    := $(MODULE_NAME).c  # Critical: Must match your actual source filename

ifeq ($(KERNELRELEASE),)

all: modules

modules:
	@echo "Building module from source: $(SRC_FILE)"
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

install: modules
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules_install
	depmod -a

uninstall:
	rm -f /lib/modules/$(KVERSION)/updates/$(MODULE_NAME).ko*
	depmod -a

clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean

.PHONY: all modules install uninstall clean

else

# Kernel build variables
obj-m := $(MODULE_NAME).o

endif