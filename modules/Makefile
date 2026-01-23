KDIR := ../linux-6.18
DESTDIR := ../share/

HW2_MODULE := hw2

obj-m += $(HW2_MODULE).o

ccflags-y += -g -O0 -DDEBUG

all: $(HW2_MODULE)

hw2:
	make -C $(KDIR) M=$(PWD) obj-m=$(HW2_MODULE).o modules
	cp -fv $(HW2_MODULE).ko $(DESTDIR)

# Clean all
clean:
	make -C $(KDIR) M=$(PWD) clean
