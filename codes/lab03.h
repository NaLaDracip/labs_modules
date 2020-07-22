#ifndef LAB03_H
#define LAB03_H

#include <linux/list.h> /* api for circular doubly linked-list */

#define LAB03_MAJOR 0
#define LAB03_FIRST_MINOR 0

struct lab03_entry
{
    int val;
    struct list_head list;
};

struct lab03_dev
{
    struct lab03_entry *data;
    struct cdev cdev;
};

static int __init
lab03_init(void);
static void lab03_setup_cdev(void);
static void lab03_exit(void);
