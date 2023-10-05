#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include "dongco.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dinh_Huu_Nam,K65-UET");
MODULE_DESCRIPTION("PID CONTROLLER");
MODULE_VERSION("1.0");

uint32_t __iomem *base_addr = NULL;
uint32_t irq_number;
struct timer_list my_timer;

/* dong co */
struct dong_co_pid {
	dev_t dev_num;
	struct class *mclass;
	struct cdev *mcdev;
	struct device *mdevice;
	char *kernel_buffer;
	volatile unsigned int count;
	volatile uint32_t time_start;
	volatile uint32_t time_current;
}dongco;

char *forward	= "Forward";
char *reverse	= "Reverse";
char *stop	= "Stop";
/* motor controller functions */
static void motor_forward(void)
{
	/* in1 (gpio16) high & in2 (gpio17) low voltage */
	uint32_t mid_cache;
	mid_cache = *(base_addr + GPSET0_OFFSET/4) | (1<<16);
	iowrite32(mid_cache, (base_addr + GPSET0_OFFSET/4));
	mid_cache = *(base_addr + GPCLR0_OFFSET/4) | (1<<17);
	iowrite32(mid_cache, (base_addr + GPCLR0_OFFSET/4));
}

static void motor_reverse(void)
{
	/* in1 (gpio16) low & in2 (gpio17) high voltage */
	uint32_t mid_cache;
        mid_cache = *(base_addr + GPCLR0_OFFSET/4) | (1<<16);
	iowrite32(mid_cache, (base_addr + GPCLR0_OFFSET/4));
        mid_cache = *(base_addr + GPSET0_OFFSET/4) | (1<<17);
        iowrite32(mid_cache, (base_addr + GPSET0_OFFSET/4));
}

static void motor_stop(void)
{
	/* in1 (gpio16) low & in2 (gpio17) low voltage */
	uint32_t mid_cache;
        mid_cache = *(base_addr + GPCLR0_OFFSET/4) | (1<<16);
        iowrite32(mid_cache, (base_addr + GPCLR0_OFFSET/4));
        mid_cache = *(base_addr + GPCLR0_OFFSET/4) | (1<<17);
        iowrite32(mid_cache, (base_addr + GPCLR0_OFFSET/4));
}

/* interrupt handler */
irqreturn_t pulse_count(int irq, void *dev_id)
{
	dongco.count += 1;
	return IRQ_HANDLED;
}
/* timer schedule */
static void lay_mau(struct timer_list *my_t)
{
	char mid_res[20];
	uint32_t ina;
	uint32_t inb;
	local_irq_disable();
	/* get time */
	dongco.time_current = jiffies - dongco.time_start;
	memset(dongco.kernel_buffer, '\0', PAGE_SIZE);
	memset(mid_res, '\0', sizeof(mid_res));
	/* check gpio16 level of voltage */
	ina = *(base_addr + GPLEV0_OFFSET/4) & (1<<16);
	/* check gpio17 level of voltage */
	inb = *(base_addr + GPLEV0_OFFSET/4) & (1<<17);
	/* find motor direction */
	if(ina != 0 && inb == 0)// && dongco.count != 0)
	{
		strncpy(mid_res, forward, strlen(forward));
	}
	else if(ina == 0 && inb != 0)
	{
		strncpy(mid_res, reverse, strlen(reverse));
	}
	else
	{
		strncpy(mid_res, stop, strlen(stop));
	}
	sprintf(dongco.kernel_buffer, "%s %d %d", mid_res, dongco.count, dongco.time_current);
	dongco.count = 0;
	local_irq_enable();
	mod_timer(&my_timer, jiffies + HZ/50);
}

/* entrypoint register */
static int dev_open(struct inode *inodep, struct file *filep)
{
	//printk(KERN_INFO "Open called!\n");
	return 0;
}
static int dev_close(struct inode *inodep, struct file *filep)
{
	//printk(KERN_INFO "Close called!\n");
	return 0;
}
static ssize_t dev_read(struct file *filep, char __user *user_buff, size_t size, loff_t *offset)
{
	int to_read;
	to_read = (size > strlen(dongco.kernel_buffer) - *offset) ? (strlen(dongco.kernel_buffer) - *offset) : (size);
	if(copy_to_user(user_buff, dongco.kernel_buffer, strlen(dongco.kernel_buffer)) != 0)
	{
		return -EFAULT;
	}
	*offset += to_read;
	return to_read;
}
static ssize_t dev_write(struct file *filep, const char *user_buff, size_t size, loff_t *offset)
{
	char direction[10];
	int speed;
	local_irq_disable(); //disable ngat
	dongco.time_start = jiffies; //lay thoi gian luc bat dau dat setpoint
	dongco.count = 0;
	*offset = 0;
	memset(direction, '\0', 10);
	memset(dongco.kernel_buffer, '\0', PAGE_SIZE); //clear buffer
	if(copy_from_user(dongco.kernel_buffer, user_buff, size) != 0) //get data from userspace
	{
		return -EFAULT;
	}
	sscanf(dongco.kernel_buffer, "%s %d", direction, &speed); //split data
	//printk(KERN_INFO "direction: %s, speed: %d\n", direction, speed);
	if(strncmp(direction, "Forward", strlen("Foward")) == 0)
	{
		motor_forward();
	}
	else if(strncmp(direction, "Reverse", strlen("Reverse")) == 0)
	{
		motor_reverse();
	}
	else if(strncmp(direction, "Stop", strlen("Stop")) == 0)
	{
		motor_stop();
	}
	local_irq_enable(); //enable lai ngat
	return size;
}

struct file_operations fops = {
	.owner		= THIS_MODULE,
	.open		= dev_open,
	.release	= dev_close,
	.read		= dev_read,
	.write		= dev_write
};

static int __init dongco_init(void)
{
	uint32_t middle_config;
	uint64_t tick_per_sec;
	//uint32_t irq_number;
	/* allocate device number */
	if(alloc_chrdev_region(&dongco.dev_num, 0, 1, DEV_NAME) < 0)
	{
		printk(KERN_ERR "Alloc Failure\n");
		return -1;
	}
	printk(KERN_INFO "Major: %d, Minor: %d\n", MAJOR(dongco.dev_num), MINOR(dongco.dev_num));
	/* create /sys/class file */
	if((dongco.mclass = class_create(THIS_MODULE, DEV_NAME)) == NULL)
	{
		printk(KERN_ERR "Cannot init class\n");
		goto rm_dev_num;
	}
	/* config character device struct */
	dongco.mcdev = cdev_alloc();
	dongco.mcdev->owner = THIS_MODULE;
	dongco.mcdev->dev = dongco.dev_num;
	cdev_init(dongco.mcdev, &fops);
	if(cdev_add(dongco.mcdev, dongco.dev_num, 1) < 0)
	{
		printk(KERN_ERR "Fail to add cdev\n");
		goto rm_class;
	}
	/* create device file */
	if((dongco.mdevice = device_create(dongco.mclass, NULL, dongco.dev_num, NULL, DEV_NAME)) == NULL)
	{
		printk(KERN_ERR "fail to create device file\n");
		goto rm_cdev;
	}
	/* allocate kernel buffer */
	dongco.kernel_buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if(dongco.kernel_buffer == NULL)
	{
		printk(KERN_ERR "Allocate failure\n");
		goto rm_device;
	}
	/* mapping register with virtual address (virtual memory) */
	base_addr = (uint32_t *)ioremap(GPIO_BASE_ADDR, GPIO_LENGTH);
	if(base_addr == NULL)
	{
		printk(KERN_ERR "mapping failure\n");
		goto rm_buff;
	}
	/* config in1 gpio 16 as an output */
	middle_config = *(base_addr + GPFSEL1_OFFSET/4) | (1<<18);
	middle_config &= ~(1<<19);
	middle_config &= ~(1<<20);
	iowrite32(middle_config, (base_addr + GPFSEL1_OFFSET/4));
	/* config in2 gpio 17 as an output */
	middle_config = *(base_addr + GPFSEL1_OFFSET/4) | (1<<21);
	middle_config &= ~(1<<22);
	middle_config &= ~(1<<23);
	iowrite32(middle_config, (base_addr + GPFSEL1_OFFSET/4));
	/* config read encoder gpio 19 as an input & pull up */
	middle_config = *(base_addr + GPFSEL1_OFFSET/4) & (~(1<<27));
	middle_config &= ~(1<<28);
	middle_config &= ~(1<<29);
	iowrite32(middle_config, (base_addr + GPFSEL1_OFFSET/4));
	middle_config = *(base_addr + PULLUP1_OFFSET/4) | (1<<6);
	middle_config &= ~(1<<7);
	iowrite32(middle_config, (base_addr + PULLUP1_OFFSET/4));
	/* config interrupt */
	irq_number = gpio_to_irq(19);
	if(request_irq(irq_number, pulse_count, IRQF_TRIGGER_FALLING, "dongco", NULL) < 0)
	{
		printk("interrupt Fault\n");
		goto rm_buff;
	}
	/* config kernel timer */
	timer_setup(&my_timer, lay_mau, 0);
	my_timer.expires	= jiffies + HZ/50; //20ms
	//my_timer.data		= 0;
	my_timer.function	= lay_mau;
	add_timer(&my_timer);

	tick_per_sec		= HZ;
	printk(KERN_INFO "Init success!, HZ = %lld\n", tick_per_sec);
	return 0;
rm_buff:
	kfree(dongco.kernel_buffer);
rm_device:
	device_destroy(dongco.mclass, dongco.dev_num);
rm_cdev:
	cdev_del(dongco.mcdev);
rm_class:
	class_destroy(dongco.mclass);
rm_dev_num:
	unregister_chrdev_region(dongco.dev_num, 1);
	return -1;
}

static void __exit dongco_exit(void)
{
	del_timer(&my_timer);
	free_irq(irq_number, NULL);
	kfree(dongco.kernel_buffer);
	device_destroy(dongco.mclass, dongco.dev_num);
	class_destroy(dongco.mclass);
	cdev_del(dongco.mcdev);
	unregister_chrdev_region(dongco.dev_num, 1);
	iounmap(base_addr);
	printk(KERN_INFO "Exit success\n");
}

module_init(dongco_init);
module_exit(dongco_exit);
