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
	{ 0x3b0ce1e, "__blk_end_request_cur" },
	{ 0x1467a36b, "__blk_end_request_all" },
	{ 0x69f8d3fd, "blk_fetch_request" },
	{ 0xb72397d5, "printk" },
	{ 0xd258f8be, "add_disk" },
	{ 0xe914e41e, "strcpy" },
	{ 0xcd65bcbb, "alloc_disk" },
	{ 0x71a50dbc, "register_blkdev" },
	{ 0xd7218d39, "blk_queue_logical_block_size" },
	{ 0x1aa2f796, "blk_init_queue" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x999e8297, "vfree" },
	{ 0xabbb3d63, "blk_cleanup_queue" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0xfa9b9788, "put_disk" },
	{ 0x7aae7332, "del_gendisk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

