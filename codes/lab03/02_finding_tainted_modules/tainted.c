#include <linux/init.h>   /* header that defines module init and cleanup functions */
#include <linux/module.h> /*header that defines struct module */
#include <linux/cdev.h>   /*for management of cdev structure*/
#include <linux/fs.h>     /* required for writing device (allocation of devno, struct file_operations, struct file, struct inode) */

#include <linux/types.h>  /* dev_t */
#include <linux/kernel.h> /* printk */
#include <linux/slab.h>   /* kmalloc, kfree */
#include <linux/errno.h>  /* error number */
#include <linux/list.h>   /* list */
#include <linux/string.h> /* for memset */

#define DECLARE_DEV(name) struct tainted_dev *name;
#define FIRST_MINOR 0
#define DEV_COUNT 1
#define DEV_NAME "taint_module"

struct tainted_dev
{
    struct cdev cdev;
};

static int __init my_module_init(void);
static void my_module_cleanup(void);
static int setup_cdev(int);

static int module_major = 0;
static DECLARE_DEV(tdev);

static int setup_cdev(int minor)
{
    int err, devno;

    devno = MKDEV(module_major, minor);
    tdev->cdev.owner = THIS_MODULE;

    cdev_init(&tdev->cdev, NULL);

    err = cdev_add(&tdev->cdev, devno, DEV_COUNT);
    if (err < 0)
        return err;

    return 0;
}

static int __init my_module_init(void)
{
    dev_t devno = 0;
    int err;

    struct module *pos;

    err = alloc_chrdev_region(&devno, FIRST_MINOR, DEV_COUNT, DEV_NAME);
    module_major = MAJOR(devno);

    if (err < 0)
        return err;

    printk(KERN_INFO "Module created with major number %d\n", module_major);

    tdev = kmalloc(sizeof(struct tainted_dev), GFP_KERNEL);
    memset(tdev, 0, sizeof(struct tainted_dev));

    if (!tdev)
    {
        err = -ENOMEM;
        goto fail;
    }

    err = setup_cdev(FIRST_MINOR);

    if (err < 0)
    {
        goto fail;
    }

    list_for_each_entry(pos, &tdev->cdev.owner->list, list)
    {
        if (pos->taints)
            printk(KERN_INFO "Module %s is tainted\n", pos->name);
    }
    return 0;

fail:
    my_module_cleanup();
    return err;
}

static void my_module_cleanup(void)
{

    if (tdev)
    {
        if (&tdev->cdev)
            cdev_del(&tdev->cdev);

        kfree(tdev);
    }
}

module_init(my_module_init);
module_exit(my_module_cleanup);
