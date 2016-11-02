#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x6c4309fb, "module_layout" },
	{ 0x4c4fef19, "kernel_stack" },
	{ 0x417c865a, "slab_buffer_size" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xf15faef6, "malloc_sizes" },
	{ 0x4aabc7c4, "__tracepoint_kmalloc" },
	{ 0x157002bc, "remove_proc_entry" },
	{ 0x3168f5d, "init_timer_key" },
	{ 0x47c7b0d2, "cpu_number" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xe174aa7, "__init_waitqueue_head" },
	{ 0x92527145, "kmem_cache_alloc_notrace" },
	{ 0x3d8a8ada, "current_task" },
	{ 0xea147363, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0x31082b28, "destroy_workqueue" },
	{ 0x916cd0ee, "__create_workqueue_key" },
	{ 0x1000e51, "schedule" },
	{ 0x6f1743ac, "create_proc_entry" },
	{ 0xf09c7f68, "__wake_up" },
	{ 0xe75663a, "prepare_to_wait" },
	{ 0xb00ccc33, "finish_wait" },
	{ 0xbead7b23, "queue_delayed_work" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DAD10E5537249FC4E73C2A0");
