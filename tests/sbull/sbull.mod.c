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
	{ 0xef0eea15, "module_layout" },
	{ 0x6980fe91, "param_get_int" },
	{ 0xff964b25, "param_set_int" },
	{ 0xf18c4f9d, "__blk_end_request_cur" },
	{ 0x236c8c64, "memcpy" },
	{ 0xc5fd7c77, "__blk_end_request_all" },
	{ 0xda2133fb, "blk_fetch_request" },
	{ 0xea147363, "printk" },
	{ 0xd7477886, "add_disk" },
	{ 0xf02de8bc, "alloc_disk" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0x439545c0, "blk_queue_logical_block_size" },
	{ 0x7be5b2d1, "blk_init_queue" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x999e8297, "vfree" },
	{ 0x6f00dd70, "blk_cleanup_queue" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0xefe19136, "put_disk" },
	{ 0x6f391da3, "del_gendisk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

