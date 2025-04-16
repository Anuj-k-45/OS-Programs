#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/input.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/limits.h>
#include <linux/slab.h>

#define PROC_ARITH "arith_interrupt"
#define PROC_TIMER "timer_control"
#define PROC_KEY   "key_control"

#define KEY_A 30 

static struct timer_list my_timer;
static unsigned int counter = 0;
static bool timer_paused = false;
static bool key_detection_enabled = false;

static struct tasklet_struct overflow_tasklet;
static struct tasklet_struct underflow_tasklet;
static struct tasklet_struct divzero_tasklet;


static ssize_t arith_interrupt_write(struct file *file, const char __user *buf,
                                     size_t len, loff_t *off)
{
    int a, b;
    char input[64];

    if (len > sizeof(input))
        return -EFAULT;
    if (copy_from_user(input, buf, len))
        return -EFAULT;

    sscanf(input, "%d %d", &a, &b);

   
    if (b == 0)
        tasklet_schedule(&divzero_tasklet);
    else if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        tasklet_schedule(&overflow_tasklet);
    else if ((b < 0 && a > INT_MAX + b) || (b > 0 && a < INT_MIN + b))
        tasklet_schedule(&underflow_tasklet);

    return len;
}


static void timer_handler(struct timer_list *t)
{
    if (!timer_paused) {
        counter++;
        printk(KERN_INFO "Timer running, counter = %u\n", counter);
    }
    mod_timer(&my_timer, jiffies + HZ);
}


static ssize_t timer_control_write(struct file *file, const char __user *buf,
                                   size_t len, loff_t *off)
{
    char input[16];

    if (len > sizeof(input))
        return -EFAULT;
    if (copy_from_user(input, buf, len))
        return -EFAULT;

    if (strncmp(input, "pause", 5) == 0) {
        timer_paused = true;
        printk(KERN_INFO "[Timer Control] Timer paused at counter = %u\n", counter);
    }
    else if (strncmp(input, "resume", 6) == 0) {
        timer_paused = false;
        mod_timer(&my_timer, jiffies + HZ);
        printk(KERN_INFO "[Timer Control] Timer resumed at counter = %u\n", counter);
    }

    return len;
}


static ssize_t key_control_write(struct file *file, const char __user *buf,
                                 size_t len, loff_t *off)
{
    char input[16];

    if (len > sizeof(input))
        return -EFAULT;
    if (copy_from_user(input, buf, len))
        return -EFAULT;

    if (strncmp(input, "enable", 6) == 0) {
        key_detection_enabled = true;
        printk(KERN_INFO "[Key Control] Key detection enabled\n");
    }
    else if (strncmp(input, "disable", 7) == 0) {
        key_detection_enabled = false;
        printk(KERN_INFO "[Key Control] Key detection disabled\n");
    }

    return len;
}


static const struct proc_ops arith_ops = {
    .proc_write = arith_interrupt_write,
};

static const struct proc_ops timer_ops = {
    .proc_write = timer_control_write,
};

static const struct proc_ops key_ops = {
    .proc_write = key_control_write,
};


static void overflow_handler(unsigned long data)
{
    printk(KERN_INFO "[Overflow ISR] Overflow occurred!\n");
}

static void underflow_handler(unsigned long data)
{
    printk(KERN_INFO "[Underflow ISR] Underflow occurred!\n");
}

static void divzero_handler(unsigned long data)
{
    printk(KERN_INFO "[DivZero ISR] Division by Zero occurred!\n");
}


static void key_event(struct input_handle *handle, unsigned int type,
                      unsigned int code, int value)
{
 
    if (!key_detection_enabled)
        return;

    if (type == EV_KEY && code == KEY_A) {
        if (value == 1)
            printk(KERN_INFO "[Key Event] Key 'A' Pressed!\n");
        else if (value == 0)
            printk(KERN_INFO "[Key Event] Key 'A' Released!\n");
    }
}

static int key_connect(struct input_handler *handler, struct input_dev *dev,
                       const struct input_device_id *id)
{
    struct input_handle *handle;
    int err;

    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "key_event_handler";

    err = input_register_handle(handle);
    if (err) {
        kfree(handle);
        return err;
    }

    err = input_open_device(handle);
    if (err) {
        input_unregister_handle(handle);
        kfree(handle);
        return err;
    }

    return 0;
}

static void key_disconnect(struct input_handle *handle)
{
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
}

static const struct input_device_id key_ids[] = {
    { .flags = INPUT_DEVICE_ID_MATCH_EVBIT, .evbit = { BIT_MASK(EV_KEY) } },
    { },
};

static struct input_handler key_handler = {
    .event      = key_event,
    .connect    = key_connect,
    .disconnect = key_disconnect,
    .name       = "key_event_handler",
    .id_table   = key_ids,
};

static int __init my_module_init(void)
{
    int ret;

    printk(KERN_INFO "Kernel Module Initialized\n");

    proc_create(PROC_ARITH, 0666, NULL, &arith_ops);
    proc_create(PROC_TIMER, 0666, NULL, &timer_ops);
    proc_create(PROC_KEY, 0666, NULL, &key_ops);


    tasklet_init(&overflow_tasklet, overflow_handler, 0);
    tasklet_init(&underflow_tasklet, underflow_handler, 0);
    tasklet_init(&divzero_tasklet, divzero_handler, 0);

    
    timer_setup(&my_timer, timer_handler, 0);
    mod_timer(&my_timer, jiffies + HZ);
    printk(KERN_INFO "Timer started, counter = 0\n");

 
    ret = input_register_handler(&key_handler);
    if (ret) {
        printk(KERN_ERR "Failed to register key input handler\n");
        return ret;
    }

    return 0;
}


static void __exit my_module_exit(void)
{
    remove_proc_entry(PROC_ARITH, NULL);
    remove_proc_entry(PROC_TIMER, NULL);
    remove_proc_entry(PROC_KEY, NULL);

    del_timer_sync(&my_timer);
    input_unregister_handler(&key_handler);

    printk(KERN_INFO "Kernel Module Unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel Module with Timer, Arithmetic Interrupts, and Improved Key Press Handling");