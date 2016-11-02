ifneq ($(KERNELRELEASE),)
obj-m := percpu.o
else
KDIR := /lib/modules/$(shell uname -r)/build
CWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(CWD) modules
clean:
	make -C $(KDIR) M=$(CWD) clean
endif
