use kernel::prelude::*;
use kernel::alloc::flags::{GFP_KERNEL, __GFP_ZERO};
use kernel::alloc::KVec;
use kernel::c_str;
use kernel::fs::file::File;
use kernel::miscdevice::{MiscDevice, MiscDeviceOptions, MiscDeviceRegistration};
use kernel::mm::virt::VmaNew;
use kernel::page::{Page, PAGE_SIZE};
use kernel::sync::Arc;
use kernel::types::ForeignOwnable;
use core::sync::atomic::{AtomicBool, Ordering};

module! {
    type: RustMmapModule,
    name:        "rust_mmap",
    authors:     ["AlexVD"],
    description: "Simple Rust mmap driver",
    license:     "GPL",
}

const BUF_PAGES: usize = 4;

// один писатель на весь модуль !
static WRITER_ACTIVE: AtomicBool = AtomicBool::new(false);

// pin и храним вектор указателей
#[pin_data]
struct MmapDev {
    pages:     KVec<Page>,
    is_writer: bool,
}

impl MmapDev {
    fn new(is_writer: bool) -> Result<Arc<Self>> {
        let mut pages = KVec::new();

        for i in 0..BUF_PAGES {
            // alloc memory in kernel space
            let page = Page::alloc_page(GFP_KERNEL | __GFP_ZERO)?;

            // temp buffer on stack
            let mut buf = [0u8; PAGE_SIZE];

            // 4 байта — код номерa страницы
            let magic: u32 = (i as u32).wrapping_mul(0x1000_0000);
            buf[0..4].copy_from_slice(&magic.to_le_bytes());

            // просто инкремент
            for j in 4..PAGE_SIZE {
                buf[j] = ((i * PAGE_SIZE + j) & 0xFF) as u8;
            }

            // пишем в страницу
            unsafe {
                page.write_raw(buf.as_ptr(), 0, PAGE_SIZE)?;
            }

            // пушим в массив
            pages.push(page, GFP_KERNEL)?;
        }

        pr_info!("allocated {} pages (writer={})\n", BUF_PAGES, is_writer);

        // сделать shared
        Arc::pin_init(pin_init!(MmapDev { pages, is_writer }), GFP_KERNEL)
    }
}

#[vtable]
impl MiscDevice for MmapDev {
    type Ptr = Arc<Self>;

    fn open(file: &File, _misc: &MiscDeviceRegistration<Self>) -> Result<Arc<Self>> {
        // проверяем хочет ли процесс писать
        let wants_write = (file.flags() & kernel::bindings::O_ACCMODE)
                          != kernel::bindings::O_RDONLY as u32;

        if wants_write {
            // если уже есть писатьель то ошибка
            WRITER_ACTIVE
                .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
                .map_err(|_| {
                    pr_info!("open: writer already active, rejecting\n");
                    EBUSY
                })?;
        }

        MmapDev::new(wants_write)
    }

    fn release(device: Arc<Self>, _file: &File) {
        if device.is_writer {
            WRITER_ACTIVE.store(false, Ordering::Release);
            pr_info!("writer released\n");
        }
        pr_info!("release — {} pages freed\n", device.pages.len());
    }

    fn mmap(
        device: <Arc<Self> as ForeignOwnable>::Borrowed<'_>,
        _file: &File,
        vma: &VmaNew,
    ) -> Result {
        let dev: &MmapDev = &*device;
        let requested = vma.end() - vma.start();

        if requested > BUF_PAGES * PAGE_SIZE || requested == 0 || requested % PAGE_SIZE != 0 {
            return Err(EINVAL);
        }

        // запрет повышения прав читателям
        if !dev.is_writer {
            vma.try_clear_maywrite()?;
        }

        vma.set_dontexpand();
        vma.set_dontcopy();
        vma.set_dontdump();

        // кешируемый тип
        let mixedmap = vma.set_mixedmap();

        let num_pages = requested / PAGE_SIZE;
        for (i, page) in dev.pages.iter().take(num_pages).enumerate() {
            // вставляем в VM
            mixedmap.vm_insert_page(vma.start() + i * PAGE_SIZE, page)?;
        }

        Ok(())
    }
}

struct RustMmapModule {
    _reg: Pin<KBox<MiscDeviceRegistration<MmapDev>>>,
}

impl kernel::Module for RustMmapModule {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("/dev/rust_mmap ready\n");

        let reg = KBox::pin_init(
            MiscDeviceRegistration::<MmapDev>::register(MiscDeviceOptions {
                name: c_str!("rust_mmap"),
            }),
            GFP_KERNEL,
        )?;

        Ok(RustMmapModule { _reg: reg })
    }
}

impl Drop for RustMmapModule {
    fn drop(&mut self) {
        pr_info!("unloaded\n");
    }
}
