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
	{ 0xd7477886, "add_disk" },
	{ 0x9edbecae, "snprintf" },
	{ 0xafe0f3a7, "open_by_devnum" },
	{ 0xf02de8bc, "alloc_disk" },
	{ 0xdd01e96c, "blk_queue_make_request" },
	{ 0x488823c9, "blk_alloc_queue" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0x7a795a78, "kmem_cache_alloc" },
	{ 0x6a52c3b3, "kmalloc_caches" },
	{ 0x37a0cba, "kfree" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0x78af5838, "blkdev_put" },
	{ 0x6f00dd70, "blk_cleanup_queue" },
	{ 0xefe19136, "put_disk" },
	{ 0x6f391da3, "del_gendisk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

