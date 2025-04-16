#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define PROC_NAME "linked_list"
#define BUFFER_SIZE 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anuj Kaushal");
MODULE_DESCRIPTION("Kernel Module for Linked List Management");

struct linked_list {
    int data;
    struct linked_list *next;
};

static struct linked_list *head = NULL;
static struct proc_dir_entry *proc_entry;
static int node_count = 0;
static DEFINE_MUTEX(list_mutex);

static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    char buffer[BUFFER_SIZE];
    int len = 0;
    struct linked_list *temp;

    if (*ppos > 0) return 0;

    mutex_lock(&list_mutex);
    len += snprintf(buffer, BUFFER_SIZE, "Node count: %d\n", node_count);
    temp = head;
    while (temp) {
        len += snprintf(buffer + len, BUFFER_SIZE - len, "Data: %d\n", temp->data);
        temp = temp->next;
    }
    mutex_unlock(&list_mutex);

    if (copy_to_user(ubuf, buffer, len))
        return -EFAULT;
    
    *ppos = len;
    return len;
}

static ssize_t proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    char buffer[BUFFER_SIZE];
    int num;
    struct linked_list *new_node, *temp;

    if (count >= BUFFER_SIZE) return -EFAULT;
    if (copy_from_user(buffer, ubuf, count)) return -EFAULT;
    buffer[count] = '\0';

    if (kstrtoint(buffer, 10, &num)) return -EINVAL;
    
    new_node = kmalloc(sizeof(struct linked_list), GFP_KERNEL);
    if (!new_node) return -ENOMEM;
    new_node->data = num;
    new_node->next = NULL;
    
    mutex_lock(&list_mutex);
    if (!head) {
        head = new_node;
    } else {
        temp = head;
        while (temp->next)
            temp = temp->next;
        temp->next = new_node;
    }
    node_count++;
    mutex_unlock(&list_mutex);
    
    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init linked_list_init(void) {
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry) return -ENOMEM;
    pr_info("Linked List Module Loaded\n");
    return 0;
}

static void __exit linked_list_exit(void) {
    struct linked_list *temp;
    while (head) {
        temp = head;
        head = head->next;
        kfree(temp);
    }
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("Linked List Module Unloaded\n");
}

module_init(linked_list_init);
module_exit(linked_list_exit);
