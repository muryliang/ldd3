ifneq ($(KERNELRELEASE),)
	obj-m := ram.o
else
	KERNDIR := /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

all:
	make -C $(KERNDIR) M=$(PWD) modules
clean:
	make -C $(KERNDIR) M=$(PWD) clean
endif
