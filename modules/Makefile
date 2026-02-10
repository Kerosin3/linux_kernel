KDIR := ../linux-6.18
DESTDIR := ../share/

KMODULE := bitmap

obj-m += $(KMODULE).o

SRCS := $(wildcard src/*.c)
$(KMODULE)-objs := $(SRCS:.c=.o)


ccflags-y += -g -O0 -DDEBUG -I$(src)/inc

all: build

build:
	@echo "Current kernel version: $$(uname -r)"
	@VERSION=$$(grep '^VERSION =' $(KDIR)/Makefile | awk '{print $$3}'); \
	PATCHLEVEL=$$(grep '^PATCHLEVEL =' $(KDIR)/Makefile | awk '{print $$3}'); \
	SUBLEVEL=$$(grep '^SUBLEVEL =' $(KDIR)/Makefile | awk '{print $$3}'); \
	EXTRAVERSION=$$(grep '^EXTRAVERSION =' $(KDIR)/Makefile | awk '{print $$3}'); \
	KERNEL_VERSION=$${VERSION}.$${PATCHLEVEL}.$${SUBLEVEL}$${EXTRAVERSION}; \
	echo "Linux kernel version to link against: $${KERNEL_VERSION}"
	@echo "Compiler: $$(gcc --version | head -n 1)"
	make -C $(KDIR) M=$(PWD) modules
	cp -fv $(KMODULE).ko $(DESTDIR)

format:
	clang-format --verbose -i --style=file src/main.c lib/*.c submodule/*.c

# Clean all
clean:
	make -C $(KDIR) M=$(PWD) clean
