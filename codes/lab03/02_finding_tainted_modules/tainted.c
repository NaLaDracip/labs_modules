#include <linux/init.h>   /* header that defines module init and cleanup functions */
#include <linux/module.h> /*header that defines struct module */
#include <linux/cdev.h>   /*for management of cdev structure*/
#include <linux/fs.h>     /* required for writing device (allocation of devno, struct file_operations, struct file, struct inode) */

#include <linux/types.h>  /* dev_t */
#include <linux/kernel.h> /* printk */
#include <linux/slab.h>   /* kmalloc, kfree */

#define FIRST_MINOR 0
#define DEV_COUNT 0
#define DEV_NAME "taint_module"

static int __init module_init(void)
{
    dev_t deno = 0;
    int err;

    err = alloc_chrdev_region
}