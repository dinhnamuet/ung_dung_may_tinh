#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x179b3102, "pwm_apply_state" },
	{ 0x967b464a, "pwm_free" },
	{ 0x2b68bd2f, "del_timer" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x37a0cba, "kfree" },
	{ 0x815b93c3, "device_destroy" },
	{ 0xe730cdd8, "class_destroy" },
	{ 0x76b48bd9, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xedc03953, "iounmap" },
	{ 0x92997ed8, "_printk" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0x98cf60b3, "strlen" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x4cbcc1fd, "__class_create" },
	{ 0xef26bcc5, "cdev_alloc" },
	{ 0x7b6d404c, "cdev_init" },
	{ 0x6499ae55, "cdev_add" },
	{ 0x600df7e7, "device_create" },
	{ 0x14f0a857, "kmalloc_caches" },
	{ 0xc086c927, "kmalloc_trace" },
	{ 0xaf56600a, "arm64_use_ng_mappings" },
	{ 0x40863ba1, "ioremap_prot" },
	{ 0x73f3aefa, "gpio_to_desc" },
	{ 0xc0e10d7, "gpiod_to_irq" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x24d273d1, "add_timer" },
	{ 0xf8276d2, "pwm_request" },
	{ 0x4b0a3f52, "gic_nonsecure_priorities" },
	{ 0xdcb764ad, "memset" },
	{ 0x9166fada, "strncpy" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xd697e69a, "trace_hardirqs_on" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0xec3d2e1b, "trace_hardirqs_off" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x87b40f76, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EBFBC4C32320D8EC1AEC9C6");
