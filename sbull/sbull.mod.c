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
	{ 0x6980fe91, "param_get_int" },
	{ 0xff964b25, "param_set_int" },
	{ 0x41344088, "param_get_charp" },
	{ 0x6ad065f4, "param_set_charp" },
	{ 0xd258f8be, "add_disk" },
	{ 0x701d0ebd, "snprintf" },
	{ 0xcd65bcbb, "alloc_disk" },
	{ 0xd7218d39, "blk_queue_logical_block_size" },
	{ 0xef534e6c, "blk_queue_make_request" },
	{ 0xfae02e07, "blk_alloc_queue" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0xf4c9dacc, "kmem_cache_alloc" },
	{ 0xfc2358d1, "kmalloc_caches" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0xa7fd7f47, "filp_open" },
	{ 0xe53379d7, "fput" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xa63847fc, "vfs_read" },
	{ 0x4f2d1115, "bio_endio" },
	{ 0x31d4f75, "kunmap" },
	{ 0x763c8498, "vfs_write" },
	{ 0xb72397d5, "printk" },
	{ 0x35ecceed, "kmap" },
	{ 0x15d368f9, "check_disk_change" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x37a0cba, "kfree" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0xabbb3d63, "blk_cleanup_queue" },
	{ 0xfa9b9788, "put_disk" },
	{ 0x7aae7332, "del_gendisk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

