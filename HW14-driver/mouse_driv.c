#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>

#define DEVICE_NAME "mouselog"
#define CLASS_NAME  "mouse"
#define DEFAULT_BUFFER_SIZE 1024
#define TIMER_INTERVAL (HZ)

MODULE_LICENSE("GPL");

static int major_number;
static struct class *mouse_class = NULL;
static struct device *mouse_device = NULL;
static struct cdev mouse_cdev;

static char *mouse_buffer;
static size_t buffer_size = DEFAULT_BUFFER_SIZE;
static size_t head = 0;
static size_t tail = 0;

static struct mutex mouse_mutex;

static struct timer_list mouse_timer;

static struct kobject *mouse_kobj;

static int display_mode = 0; // 0 - X only, 1 - Y only, 2 - both
module_param(display_mode, int, 0660);
MODULE_PARM_DESC(display_mode, "Display mode: 0=X, 1=Y, 2=Both");

#define BUF_EMPTY (head == tail)
#define BUF_FULL (((head + 1) % buffer_size) == tail)

static void simulate_mouse_data(struct timer_list *t)
{
    char event_str[100];
    int event_len = 0;
    int x_value = get_random_long() % 10 - 5;
    int y_value = get_random_long() % 10 - 5;

    switch (display_mode) {
    case 0:
        event_len = snprintf(event_str, sizeof(event_str), "MOUSE_X:%d\n", x_value);
        break;
    case 1:
        event_len = snprintf(event_str, sizeof(event_str), "MOUSE_Y:%d\n", y_value);
        break;
    case 2:
    default:
        event_len = snprintf(event_str, sizeof(event_str), "MOUSE_X:%d\nMOUSE_Y:%d\n", x_value, y_value);
        break;
    }

    mutex_lock(&mouse_mutex);
    for (int i = 0; i < event_len; i++) {
        if (BUF_FULL)
            break;
        mouse_buffer[head] = event_str[i];
        head = (head + 1) % buffer_size;
    }
    mutex_unlock(&mouse_mutex);

    mod_timer(&mouse_timer, jiffies + TIMER_INTERVAL);
}

static ssize_t mouse_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    size_t copied = 0;

    mutex_lock(&mouse_mutex);
    while (copied < len && !BUF_EMPTY) {
        if (put_user(mouse_buffer[tail], buf + copied)) {
            mutex_unlock(&mouse_mutex);
            return -EFAULT;
        }
        tail = (tail + 1) % buffer_size;
        copied++;
    }
    mutex_unlock(&mouse_mutex);
    return copied;
}

static ssize_t mouse_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    size_t i;
    mutex_lock(&mouse_mutex);
    for (i = 0; i < len; i++) {
        char ch;
        if (copy_from_user(&ch, buf + i, 1)) {
            mutex_unlock(&mouse_mutex);
            return -EFAULT;
        }
        if (BUF_FULL)
            break;
        mouse_buffer[head] = ch;
        head = (head + 1) % buffer_size;
    }
    mutex_unlock(&mouse_mutex);
    return i;
}

static int mouse_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int mouse_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = mouse_open,
    .read = mouse_read,
    .write = mouse_write,
    .release = mouse_release,
};

static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", display_mode);
}

static ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int new_mode;
    if (kstrtoint(buf, 10, &new_mode) == 0 && new_mode >= 0 && new_mode <= 2) {
        display_mode = new_mode;
    }
    return count;
}

static struct kobj_attribute mode_attribute = __ATTR(display_mode, 0660, mode_show, mode_store);

static int __init mouse_init(void)
{
    int result;

    mouse_buffer = kmalloc(DEFAULT_BUFFER_SIZE, GFP_KERNEL);
    if (!mouse_buffer)
        return -ENOMEM;

    mutex_init(&mouse_mutex);
    head = tail = 0;

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0)
        goto fail_chrdev;

    mouse_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mouse_class)) {
        result = PTR_ERR(mouse_class);
        goto fail_class;
    }

    mouse_device = device_create(mouse_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mouse_device)) {
        result = PTR_ERR(mouse_device);
        goto fail_device;
    }

    cdev_init(&mouse_cdev, &fops);
    result = cdev_add(&mouse_cdev, MKDEV(major_number, 0), 1);
    if (result)
        goto fail_cdev;

    mouse_kobj = kobject_create_and_add("mouselog", kernel_kobj);
    if (!mouse_kobj) {
        result = -ENOMEM;
        goto fail_kobj;
    }
    result = sysfs_create_file(mouse_kobj, &mode_attribute.attr);
    if (result)
        goto fail_sysfs;

    timer_setup(&mouse_timer, simulate_mouse_data, 0);
    mod_timer(&mouse_timer, jiffies + TIMER_INTERVAL);

    printk(KERN_INFO "Драйвер мыши инициализирован с номером %d\n", major_number);

    return 0;

fail_sysfs:
    kobject_put(mouse_kobj);
fail_kobj:
    cdev_del(&mouse_cdev);
fail_cdev:
    device_destroy(mouse_class, MKDEV(major_number, 0));
fail_device:
    class_destroy(mouse_class);
fail_class:
    unregister_chrdev(major_number, DEVICE_NAME);
fail_chrdev:
    kfree(mouse_buffer);
    return result;
}

static void __exit mouse_exit(void)
{
    del_timer_sync(&mouse_timer);
    sysfs_remove_file(mouse_kobj, &mode_attribute.attr);
    kobject_put(mouse_kobj);
    cdev_del(&mouse_cdev);
    device_destroy(mouse_class, MKDEV(major_number, 0));
    class_destroy(mouse_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    kfree(mouse_buffer);
}

module_init(mouse_init);
module_exit(mouse_exit);
