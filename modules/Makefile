KDIR := ../linux-6.18
DESTDIR := ../share/

HW2_MODULE := hw2

obj-m += $(HW2_MODULE).o

ccflags-y += -g -O0 -DDEBUG

all: $(HW2_MODULE)

hw2:
	@echo "Current kernel version: $$(uname -r)"
	@VERSION=$$(grep '^VERSION =' $(KDIR)/Makefile | awk '{print $$3}'); \
	PATCHLEVEL=$$(grep '^PATCHLEVEL =' $(KDIR)/Makefile | awk '{print $$3}'); \
	SUBLEVEL=$$(grep '^SUBLEVEL =' $(KDIR)/Makefile | awk '{print $$3}'); \
	EXTRAVERSION=$$(grep '^EXTRAVERSION =' $(KDIR)/Makefile | awk '{print $$3}'); \
	KERNEL_VERSION=$${VERSION}.$${PATCHLEVEL}.$${SUBLEVEL}$${EXTRAVERSION}; \
	echo "Linux kernel version to link against: $${KERNEL_VERSION}"
	@echo "Compiler: $$(gcc --version | head -n 1)"
	make -C $(KDIR) M=$(PWD) obj-m=$(HW2_MODULE).o modules
	cp -fv $(HW2_MODULE).ko $(DESTDIR)

format-hw2:
	clang-format --verbose -i --style=file hw2.c

# Clean all
clean:
	make -C $(KDIR) M=$(PWD) clean
