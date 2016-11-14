ifeq ($(KERNELRELEASE),)  

KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
PWD := $(shell pwd)  

.PHONY: build clean  

build:
	         $(MAKE) -C $(KERNELDIR) M=$(PWD) modules  

clean:
	         rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c 

else  

$(info Building with KERNELRELEASE = ${KERNELRELEASE}) 
obj-m := batpkt.o
batpkt-objs := ./src/main.o ./src/rx.o

endif
