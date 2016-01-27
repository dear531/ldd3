ifneq ($(KERNELRELEASE),)
	obj-m	:=hello_world.o scull.o scullm.o scull_block.o
else
	PWD	:=$(shell pwd)
	KVER	:=$(shell uname -r)
	KDIR	:=/lib/modules/$(KVER)/build/
default:
	$(MAKE) -C $(KDIR) M=$(PWD)
clean:
	-rm -rf *.o *.ko *.mod.c modules.order\
		*a.out Module.symvers .*.cmd .tmp_versions
endif
