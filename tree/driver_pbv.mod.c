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
	{ 0xae141548, "module_layout" },
	{ 0xcd65bcbb, "alloc_disk" },
	{ 0xabbb3d63, "blk_cleanup_queue" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x999e8297, "vfree" },
	{ 0xfae02e07, "blk_alloc_queue" },
	{ 0xb72397d5, "printk" },
	{ 0x5152e605, "memcmp" },
	{ 0x7aae7332, "del_gendisk" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0xfb500d28, "generic_make_request" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0x81e75a36, "open_by_devnum" },
	{ 0xaf8b901f, "blkdev_put" },
	{ 0x1c8236a5, "bio_clone" },
	{ 0xef534e6c, "blk_queue_make_request" },
	{ 0xfa9b9788, "put_disk" },
	{ 0xd258f8be, "add_disk" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

