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
	{ 0x316bb9bc, "cdev_del" },
	{ 0xc2a0c03c, "cdev_init" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x77315741, "down_interruptible" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0xe174aa7, "__init_waitqueue_head" },
	{ 0x71de9b3f, "_copy_to_user" },
	{ 0x3d8a8ada, "current_task" },
	{ 0xea147363, "printk" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xefb54b49, "fasync_helper" },
	{ 0xb4390f9a, "mcount" },
	{ 0xf7c2c68, "cdev_add" },
	{ 0x7dceceac, "capable" },
	{ 0x1000e51, "schedule" },
	{ 0x6443d74d, "_raw_spin_lock" },
	{ 0xf09c7f68, "__wake_up" },
	{ 0xe75663a, "prepare_to_wait" },
	{ 0x57b09822, "up" },
	{ 0xe6535d08, "kill_fasync" },
	{ 0xb00ccc33, "finish_wait" },
	{ 0x77e2f33, "_copy_from_user" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "65715ABB4976F2FA16D5384");
