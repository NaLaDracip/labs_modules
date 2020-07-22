#include <linux/types.h>

#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>    /* to register device in vfs */
#include <linux/slab.h>  /* for dynamic allocation */
#include <linux/errno.h> /*error code number for returning correct codes */

#include "lab03.h"

#define DECLARE_DEV_PTR(name) struct lab03_dev *name = NULL
#define INIT_PTR(type, name) name = (type *)kmalloc(sizeof(type), GFP_KERNEL)

#define HANDLE_ERRMEM(ptr) \
    if (!ptr)              \
    {                      \
        err = -ENOMEM;     \
        goto fail;         \
    }

#define LAB03_NR_DEVICES 1

int lab03_major = LAB03_MAJOR;
DECLARE_DEV_PTR(lab03_device);
static struct file_operations fops = {NULL};

static int __init lab03_init(void)
{
    int err;
    dev_t devno = 0;

    if (likely(!lab03_major))
    {
        err = alloc_chrdev_region(&devno, LAB03_FIRST_MINOR, LAB03_NR_DEVICES, "lab03");
        lab03_major = MAJOR(devno);
    }

    if (err < 0)
    {
        printk(KERN_WARNING "lab03: could not get a major number for lab03 device");
        return err;
    }

    INIT_PTR(lab03_dev, lab03_device);
    HANDLE_ERRMEM(lab03_device);

    INIT_PTR(lab03_entry, lab03_device->data);
    HANDLE_ERRMEM(lab03_device->data);

    INIT_LIST_HEAD(&lab03_device->data->list);

    lab03_setup_cdev();

    return 0;

fail:
    lab03_exit();
    return err;
}

static void lab03_setup_cdev(void)
{
    dev_t devno;
    int err;

    devno = MKDEV(lab03_major + LAB03_FIRST_MINOR);

    lab03_device->cdev.owner = THIS_MODULE;
    cdev_init(&lab03_device->cdev, fops);

    err = cdev_add(&lab03_device->cdev, devno, LAB03_NR_DEVICES);

    if (err < 0)
    {
        printk(KERN_NOTICE "Error %d adding lab03", err)
    }
}

static void lab03_exit(void)
{
    if (lab03_device)
    {
        if (lab03_device->data)
        {
            struct lab03_entry *pos, *tmp;
            list_for_each_entry_safe(pos, tmp, &lab03_device->data->list, lab03_entry);
            {
                kfree(pos);
            }
            kfree(lab03_device->data);
        }
        cdev_del(lab03_device);
        kfree(lab03_device);
    }
    return;
}

module_init(lab03_init);
module_exit(lab03_exit);
