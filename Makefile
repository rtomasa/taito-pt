# Allow override via environment variables (e.g., `make KVERSION=5.15.0-86-generic`)
KVERSION    ?= $(shell uname -r)
BUILD_DIR   ?= /lib/modules/$(KVERSION)/build
PWD         := $(shell pwd)
MODULE_NAME := taito_pt
INSTALL_DIR := /lib/modules/$(KVERSION)/kernel/drivers/usb

# Kernel module build logic (handles two-pass build system)
ifeq ($(KERNELRELEASE),)

# User-facing targets
all:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(BUILD_DIR) M=$(PWD) clean

.PHONY: all clean

else

# Kernel-facing configuration (second pass)
obj-m := $(MODULE_NAME).o

endif