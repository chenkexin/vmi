#makefile for hello world 
ifneq ($(KERNELRELEASE),) 
	obj-m := test_module.o
else 
	KDIR:=/lib/modules/$(shell uname -r)/build 
all: 
	make -C $(KDIR) M=$(PWD) modules 
clean: 
	rm -rf *.ko *.o *.mod.o *.mod.c *.symvers 
endif 
