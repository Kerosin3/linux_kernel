KDIR    := ../linux-kernel
LLVM    := /usr/lib/llvm18/bin/
BINDGEN := /home/$(USER)/.cargo/bin/bindgen
DESTDIR := ../share/

obj-m += rust_module.o

all: build test_mmap

# ── Kernel module (Rust) ──────────────────────────────────────────────────────

build:
	LIBCLANG_PATH=/usr/lib/llvm18/lib \
	$(MAKE) LLVM=$(LLVM) BINDGEN=$(BINDGEN) -C $(KDIR) M=$(PWD) modules

test_mmap: test_mmap.c
	$(CC) -O2 -Wall -static -march=x86-64 -o test_mmap test_mmap.c

install: build
	mkdir -p $(DESTDIR)
	cp *.ko $(DESTDIR)
	cp test_mmap $(DESTDIR)
	@echo "binary copied to $(DESTDIR)"

clean:
	$(MAKE) LLVM=$(LLVM) -C $(KDIR) M=$(PWD) clean
	rm -f test_mmap

.PHONY: all build clean install
