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
	{ 0x5c49ea9e, "alloc_pages_current" },
	{ 0xc2aee7a5, "kmem_cache_destroy" },
	{ 0x316bb9bc, "cdev_del" },
	{ 0xc2a0c03c, "cdev_init" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x6980fe91, "param_get_int" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x417c865a, "slab_buffer_size" },
	{ 0xf15faef6, "malloc_sizes" },
	{ 0x4aabc7c4, "__tracepoint_kmalloc" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0xff964b25, "param_set_int" },
	{ 0x92527145, "kmem_cache_alloc_notrace" },
	{ 0xea147363, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0x67955563, "kmem_cache_free" },
	{ 0xf7c2c68, "cdev_add" },
	{ 0xf3afa71f, "kmem_cache_alloc" },
	{ 0xba309a89, "__free_pages" },
	{ 0x93fca811, "__get_free_pages" },
	{ 0xe52947e7, "__phys_addr" },
	{ 0xbe30e722, "kmem_cache_create" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x37a0cba, "kfree" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "DA968070FD8471591D8E8FE");
