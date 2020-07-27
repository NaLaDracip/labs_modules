#include <linux/types.h>

#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>    /* to register device in vfs */
#include <linux/slab.h>  /* for dynamic allocation */
#include <linux/errno.h> /*error code number for returning correct codes */

#include "lab03.h"

MODULE_LICENSE("GPL");

#define DECLARE_DEV_PTR(name) struct lab03_dev *name = NULL
#define INIT_PTR(type, name) name = (type *)kmalloc(sizeof(type), GFP_KERNEL)
#define ADD_ENTRY(value, head)                                                                      \
    list_add(&((struct lab03_entry *)kmalloc(sizeof(struct lab03_entry), GFP_KERNEL))->list, head); \
    (container_of((head)->next, struct lab03_entry, list))->val = value;

#define HANDLE_ERRMEM(ptr) \
    if (!ptr)              \
    {                      \
        err = -ENOMEM;     \
        goto fail;         \
    }

#define LAB03_NR_DEVICES 1

int lab03_major = LAB03_MAJOR;
DECLARE_DEV_PTR(lab03_device);
static struct file_operations fops;

static int __init lab03_init(void)
{
    int err;
    dev_t devno = 0;

    if (likely(!lab03_major))
    {
        err = alloc_chrdev_region(&devno, LAB03_FIRST_MINOR, LAB03_NR_DEVICES, "lab03");
        lab03_major = MAJOR(devno);
        printk(KERN_INFO "Major number allocated %d\n", lab03_major);
    }

    if (err < 0)
    {
        printk(KERN_WARNING "lab03: could not get a major number for lab03 device");
        return err;
    }

    printk(KERN_INFO "Allocating space for device\n");
    INIT_PTR(struct lab03_dev, lab03_device);
    HANDLE_ERRMEM(lab03_device);

    printk(KERN_INFO "Allocatinf space for the entries\n");
    INIT_PTR(struct lab03_entry, lab03_device->data);
    HANDLE_ERRMEM(lab03_device->data);

    printk(KERN_INFO "Init the list\n");
    INIT_LIST_HEAD(&lab03_device->data->list);

    lab03_setup_cdev();

    // struct lab03_entry *tmp = (struct lab03_entry *)kmalloc(sizeof(struct lab03_entry), GFP_KERNEL);
    //tmp->val = 10;
    //list_add(&tmp->list, &lab03_device->data->list);

    ADD_ENTRY(10, &lab03_device->data->list);
    ADD_ENTRY(42, &lab03_device->data->list);

    struct lab03_entry *pos;

    list_for_each_entry(pos, &lab03_device->data->list, list)
    {
        printk(KERN_INFO "%d\n", pos->val);
    }
    return 0;

fail:
    lab03_exit();
    return err;
}

static void lab03_setup_cdev(void)
{
    dev_t devno;
    int err;

    devno = MKDEV(lab03_major, LAB03_FIRST_MINOR);

    lab03_device->cdev.owner = THIS_MODULE;
    cdev_init(&lab03_device->cdev, NULL);

    err = cdev_add(&lab03_device->cdev, devno, LAB03_NR_DEVICES);

    if (err < 0)
    {
        printk(KERN_NOTICE "Error %d adding lab03", err);
        return;
    }

    lab03_device->cls = class_create(THIS_MODULE, "lab03");
    lab03_device->device = device_create(lab03_device->cls, NULL, devno, "%s", "lab03");
}

static void lab03_exit(void)
{
    if (lab03_device)
    {
        printk(KERN_INFO "Freeing entries");
        if (lab03_device->data)
        {
            struct lab03_entry *pos, *tmp;
            list_for_each_entry_safe(pos, tmp, &lab03_device->data->list, list)
            {
                printk(KERN_NOTICE "\tRemove element with value %d\n", pos->val);
                kfree(pos);
            }

            printk(KERN_INFO "Freeing list");
            kfree(lab03_device->data);
        }
        device_destroy(lab03_device->cls, MKDEV(lab03_major, LAB03_FIRST_MINOR));
        class_destroy(lab03_device->cls);
        cdev_del(&lab03_device->cdev);

        printk(KERN_INFO "Freeing device");
        kfree(lab03_device);
    }
    unregister_chrdev_region(MKDEV(lab03_major, LAB03_FIRST_MINOR), LAB03_NR_DEVICES);
    return;
}

module_init(lab03_init);
module_exit(lab03_exit);
