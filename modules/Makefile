KDIR := ../linux-6.18
DESTDIR := ../share/

HELLO_MODULE := hello
PRINTL_MODULE := printl

obj-m += $(HELLO_MODULE).o $(PRINTL_MODULE).o

ccflags-y += -g -O0 -DDEBUG

all: $(HELLO_MODULE) $(PRINTL_MODULE)

hello:
	make -C $(KDIR) M=$(PWD) obj-m=$(HELLO_MODULE).o modules
	cp -fv $(HELLO_MODULE).ko $(DESTDIR)

printl:
	make -C $(KDIR) M=$(PWD) obj-m=$(PRINTL_MODULE).o modules
	cp -fv $(PRINTL_MODULE).ko $(DESTDIR)


# Clean all
clean:
	make -C $(KDIR) M=$(PWD) clean
